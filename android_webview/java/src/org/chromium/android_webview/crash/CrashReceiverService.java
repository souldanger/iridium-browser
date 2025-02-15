// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.chromium.android_webview.crash;

import android.annotation.TargetApi;
import android.app.Service;
import android.app.job.JobInfo;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.os.Binder;
import android.os.Build;
import android.os.IBinder;
import android.os.ParcelFileDescriptor;

import org.chromium.base.Log;
import org.chromium.base.VisibleForTesting;
import org.chromium.components.background_task_scheduler.TaskIds;
import org.chromium.components.minidump_uploader.CrashFileManager;
import org.chromium.components.minidump_uploader.MinidumpUploadJobService;

import java.io.File;
import java.io.IOException;

/**
 * Service that is responsible for receiving crash dumps from an application, for upload.
 */
@TargetApi(Build.VERSION_CODES.LOLLIPOP)
public class CrashReceiverService extends Service {
    private static final String TAG = "CrashReceiverService";

    private static final String WEBVIEW_CRASH_DIR = "WebView_Crashes";
    private static final String WEBVIEW_TMP_CRASH_DIR = "WebView_Crashes_Tmp";

    private final Object mCopyingLock = new Object();
    private boolean mIsCopying = false;

    @Override
    public void onCreate() {
        super.onCreate();
    }

    private final ICrashReceiverService.Stub mBinder = new ICrashReceiverService.Stub() {
        @Override
        public void transmitCrashes(ParcelFileDescriptor[] fileDescriptors) {
            int uid = Binder.getCallingUid();
            performMinidumpCopyingSerially(
                    CrashReceiverService.this, uid, fileDescriptors, true /* scheduleUploads */);
        }
    };

    /**
     * Copies minidumps in a synchronized way, waiting for any already started copying operations to
     * finish before copying the current dumps.
     * @param scheduleUploads whether to ask JobScheduler to schedule an upload-job (avoid this
     * during testing).
     */
    @VisibleForTesting
    public void performMinidumpCopyingSerially(Context context, int uid,
            ParcelFileDescriptor[] fileDescriptors, boolean scheduleUploads) {
        if (!waitUntilWeCanCopy()) {
            Log.e(TAG, "something went wrong when waiting to copy minidumps, bailing!");
            return;
        }

        try {
            boolean copySucceeded = copyMinidumps(context, uid, fileDescriptors);
            if (copySucceeded && scheduleUploads) {
                // Only schedule a new job if there actually are any files to upload.
                scheduleNewJob();
            }
        } finally {
            synchronized (mCopyingLock) {
                mIsCopying = false;
                mCopyingLock.notifyAll();
            }
        }
    }

    /**
     * Wait until we are allowed to copy minidumps.
     * @return whether we are actually allowed to copy the files - if false we should just bail.
     */
    private boolean waitUntilWeCanCopy() {
        synchronized (mCopyingLock) {
            while (mIsCopying) {
                try {
                    mCopyingLock.wait();
                } catch (InterruptedException e) {
                    Log.e(TAG, "Was interrupted when waiting to copy minidumps", e);
                    return false;
                }
            }
            mIsCopying = true;
            return true;
        }
    }

    private void scheduleNewJob() {
        JobInfo.Builder builder =
                new JobInfo
                        .Builder(TaskIds.WEBVIEW_MINIDUMP_UPLOADING_JOB_ID,
                                new ComponentName(this, AwMinidumpUploadJobService.class))
                        .setRequiredNetworkType(JobInfo.NETWORK_TYPE_UNMETERED);
        MinidumpUploadJobService.scheduleUpload(this, builder);
    }

    /**
     * Copy minidumps from the {@param fileDescriptors} to the directory where WebView stores its
     * minidump files. {@param context} is used to look up the directory in which the files will be
     * saved.
     * @return whether any minidump was copied.
     */
    @VisibleForTesting
    public static boolean copyMinidumps(
            Context context, int uid, ParcelFileDescriptor[] fileDescriptors) {
        CrashFileManager crashFileManager = new CrashFileManager(createWebViewCrashDir(context));
        boolean copiedAnything = false;
        if (fileDescriptors != null) {
            for (ParcelFileDescriptor fd : fileDescriptors) {
                if (fd == null) continue;
                try {
                    File copiedFile = crashFileManager.copyMinidumpFromFD(fd.getFileDescriptor(),
                            getWebViewTmpCrashDir(context), uid);
                    if (copiedFile == null) {
                        Log.w(TAG, "failed to copy minidump from " + fd.toString());
                        // TODO(gsennton): add UMA metric to ensure we aren't losing too many
                        // minidumps here.
                    } else {
                        copiedAnything = true;
                    }
                } catch (IOException e) {
                    Log.w(TAG, "failed to copy minidump from " + fd.toString() + ": "
                            + e.getMessage());
                } finally {
                    deleteFilesInWebViewTmpDirIfExists(context);
                }
            }
        }
        return copiedAnything;
    }

    /**
     * Delete all files in the directory where temporary files from this Service are stored.
     */
    @VisibleForTesting
    public static void deleteFilesInWebViewTmpDirIfExists(Context context) {
        deleteFilesInDirIfExists(getWebViewTmpCrashDir(context));
    }

    private static void deleteFilesInDirIfExists(File directory) {
        if (directory.isDirectory()) {
            File[] files = directory.listFiles();
            if (files != null) {
                for (File file : files) {
                    if (!file.delete()) {
                        Log.w(TAG, "Couldn't delete file " + file.getAbsolutePath());
                    }
                }
            }
        }
    }

    /**
     * Create the directory in which WebView will store its minidumps.
     * WebView needs a crash directory different from Chrome's to ensure Chrome's and WebView's
     * minidump handling won't clash in cases where both Chrome and WebView are provided by the
     * same app (Monochrome).
     * @param context Android Context used to find a cache-directory where minidumps can be stored.
     * @return a reference to the created directory, or null if the creation failed.
     */
    @VisibleForTesting
    public static File createWebViewCrashDir(Context context) {
        File dir = getWebViewCrashDir(context);
        // Call mkdir before isDirectory to ensure that if another thread created the directory
        // just before the call to mkdir, the current thread fails mkdir, but passes isDirectory.
        if (dir.mkdir() || dir.isDirectory()) {
            return dir;
        }
        return null;
    }

    /**
     * Fetch the crash directory where WebView stores its minidumps.
     * @param context Android Context used to find a cache-directory where minidumps can be stored.
     * @return a File pointing to the crash directory.
     */
    @VisibleForTesting
    public static File getWebViewCrashDir(Context context) {
        return new File(context.getCacheDir(), WEBVIEW_CRASH_DIR);
    }

    /**
     * Directory where we store files temporarily when copying from an app process.
     * @param context Android Context used to find a cache-directory where minidumps can be stored.
     */
    @VisibleForTesting
    public static File getWebViewTmpCrashDir(Context context) {
        return new File(context.getCacheDir(), WEBVIEW_TMP_CRASH_DIR);
    }

    @Override
    public IBinder onBind(Intent intent) {
        return mBinder;
    }
}
