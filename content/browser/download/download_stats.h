// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// Holds helpers for gathering UMA stats about downloads.

#ifndef CONTENT_BROWSER_DOWNLOAD_DOWNLOAD_STATS_H_
#define CONTENT_BROWSER_DOWNLOAD_DOWNLOAD_STATS_H_

#include <stddef.h>
#include <stdint.h>

#include <string>
#include <vector>

#include "base/optional.h"
#include "content/common/content_export.h"
#include "content/public/browser/download_danger_type.h"
#include "content/public/browser/download_interrupt_reasons.h"
#include "ui/base/page_transition_types.h"
#include "url/gurl.h"

namespace base {
class FilePath;
class Time;
class TimeDelta;
class TimeTicks;
}

namespace content {

// We keep a count of how often various events occur in the
// histogram "Download.Counts".
enum DownloadCountTypes {
  // Stale enum values left around so that values passed to UMA don't
  // change.
  DOWNLOAD_COUNT_UNUSED_0 = 0,
  DOWNLOAD_COUNT_UNUSED_1,
  DOWNLOAD_COUNT_UNUSED_2,
  DOWNLOAD_COUNT_UNUSED_3,
  DOWNLOAD_COUNT_UNUSED_4,

  // Downloads that made it to DownloadResourceHandler
  UNTHROTTLED_COUNT,

  // Downloads that actually complete.
  COMPLETED_COUNT,

  // Downloads that are cancelled before completion (user action or error).
  CANCELLED_COUNT,

  // Downloads that are started.
  START_COUNT,

  // Downloads that were interrupted by the OS.
  INTERRUPTED_COUNT,

  // (Deprecated) Write sizes for downloads.
  // This is equal to the number of samples in Download.WriteSize histogram.
  DOWNLOAD_COUNT_UNUSED_10,

  // (Deprecated) Counts iterations of the BaseFile::AppendDataToFile() loop.
  // This is equal to the number of samples in Download.WriteLoopCount
  // histogram.
  DOWNLOAD_COUNT_UNUSED_11,

  // Counts interruptions that happened at the end of the download.
  INTERRUPTED_AT_END_COUNT,

  // Counts errors due to writes to BaseFiles that have been detached already.
  // This can happen when saving web pages as complete packages. It happens
  // when we get messages to append data to files that have already finished and
  // been detached, but haven't yet been removed from the list of files in
  // progress.
  APPEND_TO_DETACHED_FILE_COUNT,

  // (Deprecated) Counts the number of instances where the downloaded file is
  // missing after a successful invocation of ScanAndSaveDownloadedFile().
  DOWNLOAD_COUNT_UNUSED_14,

  // (Deprecated) Count of downloads with a strong ETag and specified
  // 'Accept-Ranges: bytes'.
  DOWNLOAD_COUNT_UNUSED_15,

  // Count of downloads that didn't have a valid WebContents at the time it was
  // interrupted.
  INTERRUPTED_WITHOUT_WEBCONTENTS,

  // Count of downloads that supplies a strong validator (implying byte-wise
  // equivalence) and has a 'Accept-Ranges: bytes' header. These downloads are
  // candidates for partial resumption.
  STRONG_VALIDATOR_AND_ACCEPTS_RANGES,

  // To be used for parallel requests.
  DOWNLOAD_COUNT_UNUSED_18,

  // Count of new downloads.
  NEW_DOWNLOAD_COUNT,

  // Count of new downloads that are started in normal profile.
  NEW_DOWNLOAD_COUNT_NORMAL_PROFILE,

  // Downloads that are actually completed in normal profile.
  COMPLETED_COUNT_NORMAL_PROFILE,

  DOWNLOAD_COUNT_TYPES_LAST_ENTRY
};

enum DownloadSource {
  // The download was initiated when the SavePackage system rejected
  // a Save Page As ... by returning false from
  // SavePackage::IsSaveableContents().
  INITIATED_BY_SAVE_PACKAGE_ON_NON_HTML = 0,

  // The download was initiated by a drag and drop from a drag-and-drop
  // enabled web application.
  INITIATED_BY_DRAG_N_DROP,

  // The download was initiated by explicit RPC from the renderer process
  // (e.g. by Alt-click) through the IPC ViewHostMsg_DownloadUrl.
  INITIATED_BY_RENDERER,

  // Fomerly INITIATED_BY_PEPPER_SAVE.
  DOWNLOAD_SOURCE_UNUSED_3,

  // Formerly INITIATED_BY_RESUMPTION.
  DOWNLOAD_SOURCE_UNUSED_4,

  // A request that was initiated as a result of manually resuming an
  // interrupted download.
  INITIATED_BY_MANUAL_RESUMPTION,

  // A request that was initiated as a result of automatically resuming an
  // interrupted download.
  INITIATED_BY_AUTOMATIC_RESUMPTION,

  DOWNLOAD_SOURCE_LAST_ENTRY
};

enum DownloadDiscardReason {
  // The download is being discarded due to a user action.
  DOWNLOAD_DISCARD_DUE_TO_USER_ACTION,

  // The download is being discarded due to the browser being shut down.
  DOWNLOAD_DISCARD_DUE_TO_SHUTDOWN
};

// Increment one of the above counts.
void RecordDownloadCount(DownloadCountTypes type);

// Record initiation of a download from a specific source.
void RecordDownloadSource(DownloadSource source);

// Record COMPLETED_COUNT and how long the download took.
void RecordDownloadCompleted(const base::TimeTicks& start,
                             int64_t download_len);

// Record INTERRUPTED_COUNT, |reason|, |received| and |total| bytes.
void RecordDownloadInterrupted(DownloadInterruptReason reason,
                               int64_t received,
                               int64_t total);

// Record that a download has been classified as malicious.
void RecordMaliciousDownloadClassified(DownloadDangerType danger_type);

// Record a dangerous download accept event.
void RecordDangerousDownloadAccept(
    DownloadDangerType danger_type,
    const base::FilePath& file_path);

// Record a dangerous download discard event.
void RecordDangerousDownloadDiscard(
    DownloadDiscardReason reason,
    DownloadDangerType danger_type,
    const base::FilePath& file_path);

// Records the mime type of the download.
void RecordDownloadMimeType(const std::string& mime_type);

// Records the mime type of the download for normal profile.
void RecordDownloadMimeTypeForNormalProfile(const std::string& mime_type);

// Records usage of Content-Disposition header.
void RecordDownloadContentDisposition(const std::string& content_disposition);

// Record the number of buffers piled up by the IO thread
// before the file thread gets to draining them.
void RecordFileThreadReceiveBuffers(size_t num_buffers);

// Record the time of both the first open and all subsequent opens since the
// download completed.
void RecordOpen(const base::Time& end, bool first);

// Record whether or not the server accepts ranges, and the download size. Also
// counts if a strong validator is supplied. The combination of range request
// support and ETag indicates downloads that are candidates for partial
// resumption.
void RecordAcceptsRanges(const std::string& accepts_ranges,
                         int64_t download_len,
                         bool has_strong_validator);

// Record the number of completed unopened downloads when a download is opened.
void RecordOpensOutstanding(int size);

// Record how long we block the file thread at a time.
void RecordContiguousWriteTime(base::TimeDelta time_blocked);

// Record the percentage of time we had to block the network (i.e.
// how often, for each download, something other than the network
// was the bottleneck).
void RecordNetworkBlockage(base::TimeDelta resource_handler_lifetime,
                           base::TimeDelta resource_handler_blocked_time);

// Record overall bandwidth stats at the file end.
void RecordFileBandwidth(size_t length,
                         base::TimeDelta disk_write_time,
                         base::TimeDelta elapsed_time);

// Record the result of a download file rename.
void RecordDownloadFileRenameResultAfterRetry(
    base::TimeDelta time_since_first_failure,
    DownloadInterruptReason interrupt_reason);

enum SavePackageEvent {
  // The user has started to save a page as a package.
  SAVE_PACKAGE_STARTED,

  // The save package operation was cancelled.
  SAVE_PACKAGE_CANCELLED,

  // The save package operation finished without being cancelled.
  SAVE_PACKAGE_FINISHED,

  // The save package tried to write to an already completed file.
  SAVE_PACKAGE_WRITE_TO_COMPLETED,

  // The save package tried to write to an already failed file.
  SAVE_PACKAGE_WRITE_TO_FAILED,

  SAVE_PACKAGE_LAST_ENTRY
};

void RecordSavePackageEvent(SavePackageEvent event);

enum OriginStateOnResumption {
  ORIGIN_STATE_ON_RESUMPTION_ADDITIONAL_REDIRECTS = 1<<0,
  ORIGIN_STATE_ON_RESUMPTION_VALIDATORS_CHANGED = 1<<1,
  ORIGIN_STATE_ON_RESUMPTION_CONTENT_DISPOSITION_CHANGED = 1<<2,
  ORIGIN_STATE_ON_RESUMPTION_MAX = 1<<3
};

// Record the state of the origin information across a download resumption
// request. |state| is a combination of values from OriginStateOnResumption
// enum.
void RecordOriginStateOnResumption(bool is_partial,
                                   int state);

void RecordDownloadConnectionSecurity(const GURL& download_url,
                                      const std::vector<GURL>& url_chain);

void RecordDownloadSourcePageTransitionType(
    const base::Optional<ui::PageTransition>& transition);

}  // namespace content

#endif  // CONTENT_BROWSER_DOWNLOAD_DOWNLOAD_STATS_H_
