// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package org.chromium.chrome.browser.media.remote;

import android.app.Dialog;
import android.graphics.Rect;
import android.os.StrictMode;
import android.support.v4.app.DialogFragment;
import android.support.v4.app.FragmentManager;
import android.view.View;

import junit.framework.Assert;

import org.chromium.base.Log;
import org.chromium.base.ThreadUtils;
import org.chromium.chrome.browser.ChromeActivity;
import org.chromium.chrome.browser.media.RouterTestUtils;
import org.chromium.chrome.browser.media.remote.RemoteVideoInfo.PlayerState;
import org.chromium.chrome.browser.tab.Tab;
import org.chromium.chrome.test.ChromeActivityTestCaseBase;
import org.chromium.content.browser.ContentViewCore;
import org.chromium.content.browser.test.util.ClickUtils;
import org.chromium.content.browser.test.util.DOMUtils;
import org.chromium.content.browser.test.util.JavaScriptUtils;
import org.chromium.content.browser.test.util.UiUtils;
import org.chromium.content_public.browser.WebContents;
import org.chromium.net.test.EmbeddedTestServer;

import java.util.HashSet;
import java.util.Set;
import java.util.concurrent.Callable;
import java.util.concurrent.CountDownLatch;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.TimeoutException;

/**
 * Base class for tests of Clank Cast. Contains functions for setting up a cast connection and other
 * utility functions.
 */
public abstract class CastTestBase extends ChromeActivityTestCaseBase<ChromeActivity> {
    private class TestListener implements MediaRouteController.UiListener {
        @Override
        public void onRouteSelected(String name, MediaRouteController mediaRouteController) {
        }

        @Override
        public void onRouteUnselected(MediaRouteController mediaRouteController) {
        }

        @Override
        public void onPrepared(MediaRouteController mediaRouteController) {
        }

        @Override
        public void onError(int errorType, String message) {
        }

        @Override
        public void onPlaybackStateChanged(final PlayerState newState) {
            // Use postOnUiThread to handling the latch until the current UI task has completed,
            // this makes sure that Cast has finished handling the event.
            ThreadUtils.postOnUiThread(new Runnable() {
                @Override
                public void run() {
                    if ((mAwaitedStates.contains(newState))) {
                        mLatch.countDown();
                    }
                }
            });
        }

        @Override
        public void onDurationUpdated(long durationMillis) {}

        @Override
        public void onPositionChanged(long positionMillis) {}

        @Override
        public void onTitleChanged(String title) {
        }

    }

    private Set<PlayerState> mAwaitedStates;
    private CountDownLatch mLatch;
    private EmbeddedTestServer mTestServer;

    // The name of the route provided by the dummy cast device.
    protected static final String CAST_TEST_ROUTE = "Cast Test Route";

    // URLs of the default test page and video.
    protected static final String DEFAULT_VIDEO_PAGE =
            "/chrome/test/data/android/media/simple_video.html";
    protected static final String DEFAULT_VIDEO = "/chrome/test/data/android/media/test.webm";

    // Constants used to find the default video and maximise button on the page
    protected static final String VIDEO_ELEMENT = "video";

    // Max time to open a view.
    protected static final int MAX_VIEW_TIME_MS = 10000;

    // Time to let a video run to ensure that it has started.
    protected static final int RUN_TIME_MS = 1000;
    // Time to allow for the UI to react to video controls,
    protected static final int STABILIZE_TIME_MS = 3000;

    // Retry interval when looking for a view.
    protected static final int VIEW_RETRY_MS = 100;

    protected static final String TEST_VIDEO_PAGE_2 =
            "/chrome/test/data/android/media/simple_video2.html";

    protected static final String TEST_VIDEO_2 = "/chrome/test/data/android/media/test2.webm";

    protected static final String TWO_VIDEO_PAGE =
            "/chrome/test/data/android/media/two_videos.html";

    private static final String TAG = "CastTestBase";

    private MediaRouteController mMediaRouteController;

    private StrictMode.ThreadPolicy mOldPolicy;

    public CastTestBase() {
        super(ChromeActivity.class);
    }

    @Override
    protected void setUp() throws Exception {
        super.setUp();
        // Temporary until support library is updated, see http://crbug.com/576393.
        ThreadUtils.runOnUiThreadBlocking(new Runnable() {
            @Override
            public void run() {
                mOldPolicy = StrictMode.allowThreadDiskReads();
                StrictMode.allowThreadDiskWrites();
            }
        });
        mTestServer = EmbeddedTestServer.createAndStartServer(getInstrumentation().getContext());
    }

    @Override
    protected void tearDown() throws Exception {
        mTestServer.stopAndDestroyServer();
        // Temporary until support library is updated, see http://crbug.com/576393.
        ThreadUtils.runOnUiThreadBlocking(new Runnable() {
            @Override
            public void run() {
                StrictMode.setThreadPolicy(mOldPolicy);
            }
        });
        super.tearDown();
    }

    @Override
    public void startMainActivity() throws InterruptedException {
        startMainActivityOnBlankPage();
    }

    /**
     * Wait for cast to reach a state we are interested in.
     * Will deadlock if called on the target's UI thread.
     * @param states
     */
    protected boolean waitForStates(final Set<PlayerState> states, int waitTimeMs) {
        mAwaitedStates = states;
        mLatch = new CountDownLatch(1);
        // Deal with the case where Chrome is already in the desired state
        ThreadUtils.runOnUiThread(new Runnable() {
            @Override
            public void run() {
                if (mMediaRouteController != null
                        && states.contains(mMediaRouteController.getDisplayedPlayerState())) {
                    mLatch.countDown();
                }
            }
        });
        try {
            return mLatch.await(waitTimeMs, TimeUnit.MILLISECONDS);
        } catch (InterruptedException e) {
            return false;
        }
    }

    /**
     * Wait for cast to reach a state we are interested in.
     * Will deadlock if called on the target's UI thread.
     * @param states
     */
    protected boolean waitForState(final PlayerState state, int waitTimeMs) {
        Set<PlayerState> states = new HashSet<PlayerState>();
        states.add(state);
        return waitForStates(states, waitTimeMs);
    }

    protected EmbeddedTestServer getTestServer() {
        return mTestServer;
    }

    protected void castAndPauseDefaultVideoFromPage(String pagePath) throws InterruptedException,
            TimeoutException {
        Rect videoRect = castDefaultVideoFromPage(pagePath);

        final Tab tab = getActivity().getActivityTab();

        Rect pauseButton = playPauseButton(videoRect);

        // Make sure the video has made some progress
        Thread.sleep(RUN_TIME_MS);

        tapButton(tab, pauseButton);
        assertTrue("Not paused", waitForState(PlayerState.PAUSED, MAX_VIEW_TIME_MS));
    }

    private boolean videoReady(String videoElement, WebContents webContents) {
        // Create a javascript function to check if the video meta-data has been loaded.
        StringBuilder sb = new StringBuilder();
        sb.append("(function() {");
        sb.append("  var node = document.getElementById('" + videoElement + "');");
        sb.append("  if (!node) return null;");
        // Any video readyState value greater than 0 means that at least the meta-data has been
        // loaded but we also need the a document readyState of complete to ensure that page has
        // been laid out with the correct video size, and everything is drawn.
        sb.append("  return node.readyState > 0 && document.readyState == 'complete';");
        sb.append("})();");
        String javascriptResult;
        try {
            javascriptResult = JavaScriptUtils.executeJavaScriptAndWaitForResult(
                    webContents, sb.toString());
            Assert.assertFalse("Failed to retrieve contents for " + videoElement,
                    javascriptResult.trim().equalsIgnoreCase("null"));

            Boolean ready = javascriptResult.trim().equalsIgnoreCase("true");
            return ready;
        } catch (InterruptedException e) {
            Assert.fail("Interrupted");
        } catch (TimeoutException e) {
            Assert.fail("Javascript execution timed out");
        }
        return false;
    }

    protected void waitUntilVideoReady(String videoElement, WebContents webContents) {
        for (int time = 0; time < MAX_VIEW_TIME_MS; time += VIEW_RETRY_MS) {
            try {
                if (videoReady(videoElement, webContents)) return;
            } catch (Exception e) {
                fail(e.toString());
            }
            sleepNoThrow(VIEW_RETRY_MS);
        }
        Assert.fail("Video not ready");
    }

    protected Rect prepareDefaultVideofromPage(String pagePath, Tab currentTab)
            throws InterruptedException, TimeoutException {

        loadUrl(mTestServer.getURL(pagePath));

        WebContents webContents = currentTab.getWebContents();

        waitUntilVideoReady(VIDEO_ELEMENT, webContents);

        return DOMUtils.getNodeBounds(webContents, VIDEO_ELEMENT);
    }

    protected Rect castDefaultVideoFromPage(String pagePath)
            throws InterruptedException, TimeoutException {
        final Tab tab = getActivity().getActivityTab();
        final Rect videoRect = prepareDefaultVideofromPage(pagePath, tab);

        castVideoAndWaitUntilPlaying(CAST_TEST_ROUTE, tab, videoRect);

        return videoRect;
    }

    protected void castVideoAndWaitUntilPlaying(final String chromecastName, final Tab tab,
            final Rect videoRect) {
        castVideo(chromecastName, tab, videoRect);

        assertTrue("Video didn't start playing", waitUntilPlaying());

    }

    protected void castVideo(final String chromecastName, final Tab tab, final Rect videoRect) {
        Log.i(TAG, "castVideo, videoRect = " + videoRect);

        ThreadUtils.runOnUiThreadBlocking(new Runnable() {
            @Override
            public void run() {
                RemoteMediaPlayerController playerController =
                        RemoteMediaPlayerController.instance();
                mMediaRouteController = playerController.getMediaRouteController(
                        mTestServer.getURL(DEFAULT_VIDEO),
                        mTestServer.getURL(DEFAULT_VIDEO_PAGE));
                assertNotNull("Could not get MediaRouteController", mMediaRouteController);
                mMediaRouteController.addUiListener(new TestListener());
            }
        });
        tapCastButton(tab, videoRect);

        // Wait for the test device to appear in the device list.
        try {
            UiUtils.settleDownUI(getInstrumentation());
        } catch (InterruptedException e) {
            fail();
        }

        View testRouteButton = RouterTestUtils.waitForRouteButton(
                getActivity(), chromecastName, MAX_VIEW_TIME_MS, VIEW_RETRY_MS);
        assertNotNull("Test route not found", testRouteButton);

        ClickUtils.mouseSingleClickView(getInstrumentation(), testRouteButton);
    }

    protected void checkDisconnected() {
        HashSet<PlayerState> disconnectedStates = new HashSet<PlayerState>();
        disconnectedStates.add(PlayerState.FINISHED);
        disconnectedStates.add(PlayerState.INVALIDATED);
        waitForStates(disconnectedStates, MAX_VIEW_TIME_MS);
        // Could use assertTrue(isDisconnected()) here, but retesting the individual aspects of
        // disconnection gives more specific error messages.
        CastNotificationControl notificationControl =
                CastNotificationControl.getForTests();
        if (notificationControl != null && notificationControl.isShowingForTests()) {
            fail("Failed to close notification");
        }
        assertEquals("Video still playing?", null, getUriPlaying());
        assertTrue("RemoteMediaPlayerController not stopped", !isPlayingRemotely());
    }

    protected void clickDisconnectFromRoute(Tab tab, Rect videoRect) {
        // Click on the cast control button to stop casting
        tapCastButton(tab, videoRect);

        // Wait for the disconnect button
        final View disconnectButton = RouterTestUtils.waitForView(new Callable<View>() {
            @Override
            public View call() {
                FragmentManager fm = getActivity().getSupportFragmentManager();
                if (fm == null) return null;
                DialogFragment mediaRouteControllerFragment = (DialogFragment) fm.findFragmentByTag(
                        "android.support.v7.mediarouter:MediaRouteControllerDialogFragment");
                if (mediaRouteControllerFragment == null) return null;
                Dialog dialog = mediaRouteControllerFragment.getDialog();
                if (dialog == null) return null;
                // The stop button (previously called disconnect) simply uses 'button1' in the
                // latest version of the support library. See:
                // https://cs.corp.google.com/#android/frameworks/support/v7/mediarouter/src/android/support/v7/app/MediaRouteControllerDialog.java&l=90.
                // TODO(aberent) remove dependency on internals of support library
                //               https://crbug/548599
                return dialog.findViewById(android.R.id.button1);
            }
        }, MAX_VIEW_TIME_MS, VIEW_RETRY_MS);

        assertNotNull("No disconnect button", disconnectButton);

        ClickUtils.clickButton(disconnectButton);
    }

    /*
     * Check that a (non-YouTube) video has started playing, and that all the controls have been
     * correctly set up.
     */
    protected void checkVideoStarted(String testVideo) {
        // Check we have a notification
        CastNotificationControl notificationControl = waitForCastNotification();
        assertNotNull("No notification controller", notificationControl);
        assertTrue("No notification", notificationControl.isShowingForTests());
        // Check that we are playing the right video
        waitUntilVideoCurrent(testVideo);
        assertEquals(
                "Wrong video playing", mTestServer.getURL(testVideo), getUriPlaying());

        // Check that the RemoteMediaPlayerController and the (YouTube)MediaRouteController have
        // been set up correctly
        waitUntilPlaying();
        RemoteMediaPlayerController playerController = RemoteMediaPlayerController.getIfExists();
        assertNotNull("No RemoteMediaPlayerController", playerController);
        assertTrue("Video not playing", isPlayingRemotely());
        assertTrue("Wrong sort of MediaRouteController", (playerController
                .getCurrentlyPlayingMediaRouteController() instanceof DefaultMediaRouteController));
    }

    protected void sleepNoThrow(long timeout) {
        try {
            Thread.sleep(timeout);
        } catch (InterruptedException e) {
            fail(e.toString());
        }
    }

    protected void tapVideoFullscreenButton(final Tab tab, final Rect videoRect) {
        tapButton(tab, fullscreenButton(videoRect));
    }

    protected void tapCastButton(final Tab tab, final Rect videoRect) {
        tapButton(tab, castButton(videoRect));
    }

    protected void tapPlayPauseButton(final Tab tab, final Rect videoRect) {
        tapButton(tab, playPauseButton(videoRect));
    }

    protected CastNotificationControl waitForCastNotification() {
        for (int time = 0; time < MAX_VIEW_TIME_MS; time += VIEW_RETRY_MS) {
            CastNotificationControl result = ThreadUtils.runOnUiThreadBlockingNoException(
                    new Callable<CastNotificationControl>() {
                        @Override
                        public CastNotificationControl call() {
                            return CastNotificationControl.getForTests();
                        }
                    });
            if (result != null) {
                return result;
            }
            sleepNoThrow(VIEW_RETRY_MS);
        }
        return null;
    }

    protected boolean waitUntilPlaying() {
        return waitForState(PlayerState.PLAYING, MAX_VIEW_TIME_MS);
    }

    private boolean isDisconnected() {
        return ThreadUtils.runOnUiThreadBlockingNoException(new Callable<Boolean>() {
            @Override
            public Boolean call() {
                CastNotificationControl notificationControl = CastNotificationControl.getForTests();
                if (notificationControl != null && notificationControl.isShowingForTests()) {
                    return false;
                }
                if (getUriPlaying() != null) return false;
                return !isPlayingRemotely();
            }
        });
    }

    private boolean waitUntilVideoCurrent(String testVideo) {
        for (int time = 0; time < MAX_VIEW_TIME_MS; time += VIEW_RETRY_MS) {
            if (mTestServer.getURL(testVideo).equals(getUriPlaying())) {
                return true;
            }
            sleepNoThrow(VIEW_RETRY_MS);
        }
        return false;
    }

    protected String getUriPlaying() {
        return ThreadUtils.runOnUiThreadBlockingNoException(new Callable<String>() {
            @Override
            public String call() {
                if (mMediaRouteController == null) return "";
                return mMediaRouteController.getUriPlaying();
            }
        });
    }

    protected long getRemotePositionMs() {
        return ThreadUtils.runOnUiThreadBlockingNoException(new Callable<Long>() {
            @Override
            public Long call() {
                return getMediaRouteController().getPosition();
            }
        });
    }

    protected long getRemoteDurationMs() {
        return ThreadUtils.runOnUiThreadBlockingNoException(new Callable<Long>() {
            @Override
            public Long call() {
                return getMediaRouteController().getDuration();
            }
        });
    }

    protected boolean isPlayingRemotely() {
        return ThreadUtils.runOnUiThreadBlockingNoException(new Callable<Boolean>() {
            @Override
            public Boolean call() {
                RemoteMediaPlayerController playerController =
                        RemoteMediaPlayerController.getIfExists();
                if (playerController == null) return false;
                MediaRouteController routeController =
                        playerController.getCurrentlyPlayingMediaRouteController();
                if (routeController == null) return false;
                return routeController.isPlaying();
            }
        });
    }

    protected MediaRouteController getMediaRouteController() {
        return ThreadUtils.runOnUiThreadBlockingNoException(new Callable<MediaRouteController>() {
            @Override
            public MediaRouteController call() {
                RemoteMediaPlayerController playerController =
                        RemoteMediaPlayerController.getIfExists();
                assertNotNull("No RemoteMediaPlayerController", playerController);
                MediaRouteController routeController =
                        playerController.getCurrentlyPlayingMediaRouteController();
                assertNotNull("No MediaRouteController", routeController);
                return routeController;
            }
        });
    }

    /*
     * Functions to find the controls Unfortunately the controls are invisible to the code outside
     * Blink, so this is highly dependent on the geometry defined in Blink css (see
     * MediaControls.css & MediaControlsAndroid.css).
     */
    private static final int CONTROLS_HEIGHT = 35;
    private static final int BUTTON_WIDTH = 35;
    private static final int CONTROL_BAR_MARGIN = 5;
    private static final int BUTTON_RIGHT_MARGIN = 9;
    private static final int PLAY_BUTTON_LEFT_MARGIN = 9;
    private static final int FULLSCREEN_BUTTON_LEFT_MARGIN = -5;

    private Rect controlBar(Rect videoRect) {
        int left = videoRect.left + CONTROL_BAR_MARGIN;
        int right = videoRect.right - CONTROL_BAR_MARGIN;
        int bottom = videoRect.bottom - CONTROL_BAR_MARGIN;
        int top = videoRect.bottom - CONTROLS_HEIGHT;
        return new Rect(left, top, right, bottom);
    }

    private Rect playPauseButton(Rect videoRect) {
        Rect bar = controlBar(videoRect);
        int left = bar.left + PLAY_BUTTON_LEFT_MARGIN;
        int right = left + BUTTON_WIDTH;
        return new Rect(left, bar.top, right, bar.bottom);
    }

    private Rect castButton(Rect videoRect) {
        Rect bar = controlBar(videoRect);
        int right = bar.right - BUTTON_RIGHT_MARGIN;
        int left = right - BUTTON_WIDTH;
        return new Rect(left, bar.top, right, bar.bottom);
    }

    private Rect fullscreenButton(Rect videoRect) {
        Rect downloadButton = downloadButton(videoRect);
        int right = downloadButton.left;
        int left = right - BUTTON_WIDTH;
        return new Rect(left, downloadButton.top, right, downloadButton.bottom);
    }

    private Rect downloadButton(Rect videoRect) {
        Rect castButton = castButton(videoRect);
        int right = castButton.right - BUTTON_RIGHT_MARGIN;
        int left = right - BUTTON_WIDTH;
        return new Rect(left, castButton.top, right, castButton.bottom);
    }

    private void tapButton(Tab tab, Rect rect) {
        ContentViewCore core = tab.getContentViewCore();
        int clickX =
                (int) core.getRenderCoordinates().fromLocalCssToPix(
                        ((float) (rect.left + rect.right)) / 2);
        int clickY = (int) core.getRenderCoordinates().fromLocalCssToPix(
                             ((float) (rect.top + rect.bottom)) / 2)
                + core.getTopControlsHeightPix();
        // Click using a virtual mouse, since a touch may result in a disambiguation pop-up.
        ClickUtils.mouseSingleClickView(getInstrumentation(), tab.getView(), clickX, clickY);
    }


}
