// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "components/favicon/core/favicon_service_impl.h"

#include <stddef.h>
#include <cmath>
#include <utility>

#include "base/hash.h"
#include "base/single_thread_task_runner.h"
#include "base/threading/thread_task_runner_handle.h"
#include "base/trace_event/trace_event.h"
#include "components/favicon/core/favicon_client.h"
#include "components/favicon_base/favicon_util.h"
#include "components/favicon_base/select_favicon_frames.h"
#include "components/history/core/browser/history_service.h"
#include "third_party/skia/include/core/SkBitmap.h"
#include "ui/gfx/codec/png_codec.h"
#include "ui/gfx/favicon_size.h"
#include "ui/gfx/image/image_skia.h"
#include "url/gurl.h"

namespace favicon {
namespace {

// Returns a vector of pixel edge sizes from |size_in_dip| and
// favicon_base::GetFaviconScales().
std::vector<int> GetPixelSizesForFaviconScales(int size_in_dip) {
  std::vector<float> scales = favicon_base::GetFaviconScales();
  std::vector<int> sizes_in_pixel;
  for (size_t i = 0; i < scales.size(); ++i) {
    sizes_in_pixel.push_back(std::ceil(size_in_dip * scales[i]));
  }
  return sizes_in_pixel;
}

}  // namespace

FaviconServiceImpl::FaviconServiceImpl(
    std::unique_ptr<FaviconClient> favicon_client,
    history::HistoryService* history_service)
    : favicon_client_(std::move(favicon_client)),
      history_service_(history_service) {
  DCHECK(history_service_);
}

FaviconServiceImpl::~FaviconServiceImpl() {}

base::CancelableTaskTracker::TaskId FaviconServiceImpl::GetFaviconImage(
    const GURL& icon_url,
    const favicon_base::FaviconImageCallback& callback,
    base::CancelableTaskTracker* tracker) {
  TRACE_EVENT0("browser", "FaviconServiceImpl::GetFaviconImage");
  favicon_base::FaviconResultsCallback callback_runner =
      base::Bind(&FaviconServiceImpl::RunFaviconImageCallbackWithBitmapResults,
                 base::Unretained(this), callback, gfx::kFaviconSize);
  std::vector<GURL> icon_urls;
  icon_urls.push_back(icon_url);
  return history_service_->GetFavicons(
      icon_urls, favicon_base::FAVICON,
      GetPixelSizesForFaviconScales(gfx::kFaviconSize), callback_runner,
      tracker);
}

base::CancelableTaskTracker::TaskId FaviconServiceImpl::GetRawFavicon(
    const GURL& icon_url,
    favicon_base::IconType icon_type,
    int desired_size_in_pixel,
    const favicon_base::FaviconRawBitmapCallback& callback,
    base::CancelableTaskTracker* tracker) {
  TRACE_EVENT0("browser", "FaviconServiceImpl::GetRawFavicon");
  favicon_base::FaviconResultsCallback callback_runner = base::Bind(
      &FaviconServiceImpl::RunFaviconRawBitmapCallbackWithBitmapResults,
      base::Unretained(this), callback, desired_size_in_pixel);

  std::vector<GURL> icon_urls;
  icon_urls.push_back(icon_url);
  std::vector<int> desired_sizes_in_pixel;
  desired_sizes_in_pixel.push_back(desired_size_in_pixel);

  return history_service_->GetFavicons(
      icon_urls, icon_type, desired_sizes_in_pixel, callback_runner, tracker);
}

base::CancelableTaskTracker::TaskId FaviconServiceImpl::GetFavicon(
    const GURL& icon_url,
    favicon_base::IconType icon_type,
    int desired_size_in_dip,
    const favicon_base::FaviconResultsCallback& callback,
    base::CancelableTaskTracker* tracker) {
  TRACE_EVENT0("browser", "FaviconServiceImpl::GetFavicon");
  std::vector<GURL> icon_urls;
  icon_urls.push_back(icon_url);
  return history_service_->GetFavicons(
      icon_urls, icon_type, GetPixelSizesForFaviconScales(desired_size_in_dip),
      callback, tracker);
}

base::CancelableTaskTracker::TaskId
FaviconServiceImpl::GetFaviconImageForPageURL(
    const GURL& page_url,
    const favicon_base::FaviconImageCallback& callback,
    base::CancelableTaskTracker* tracker) {
  TRACE_EVENT0("browser", "FaviconServiceImpl::GetFaviconImageForPageURL");
  return GetFaviconForPageURLImpl(
      page_url, favicon_base::FAVICON,
      GetPixelSizesForFaviconScales(gfx::kFaviconSize),
      base::Bind(&FaviconServiceImpl::RunFaviconImageCallbackWithBitmapResults,
                 base::Unretained(this), callback, gfx::kFaviconSize),
      tracker);
}

base::CancelableTaskTracker::TaskId FaviconServiceImpl::GetRawFaviconForPageURL(
    const GURL& page_url,
    int icon_types,
    int desired_size_in_pixel,
    const favicon_base::FaviconRawBitmapCallback& callback,
    base::CancelableTaskTracker* tracker) {
  TRACE_EVENT0("browser", "FaviconServiceImpl::GetRawFaviconForPageURL");
  std::vector<int> desired_sizes_in_pixel;
  desired_sizes_in_pixel.push_back(desired_size_in_pixel);
  return GetFaviconForPageURLImpl(
      page_url, icon_types, desired_sizes_in_pixel,
      base::Bind(
          &FaviconServiceImpl::RunFaviconRawBitmapCallbackWithBitmapResults,
          base::Unretained(this), callback, desired_size_in_pixel),
      tracker);
}

base::CancelableTaskTracker::TaskId
FaviconServiceImpl::GetLargestRawFaviconForPageURL(
    const GURL& page_url,
    const std::vector<int>& icon_types,
    int minimum_size_in_pixels,
    const favicon_base::FaviconRawBitmapCallback& callback,
    base::CancelableTaskTracker* tracker) {
  TRACE_EVENT0("browser", "FaviconServiceImpl::GetLargestRawFaviconForPageURL");
  favicon_base::FaviconResultsCallback favicon_results_callback = base::Bind(
      &FaviconServiceImpl::RunFaviconRawBitmapCallbackWithBitmapResults,
      base::Unretained(this), callback, 0);
  if (favicon_client_ && favicon_client_->IsNativeApplicationURL(page_url)) {
    std::vector<int> desired_sizes_in_pixel;
    desired_sizes_in_pixel.push_back(0);
    return favicon_client_->GetFaviconForNativeApplicationURL(
        page_url, desired_sizes_in_pixel, favicon_results_callback, tracker);
  }
  return history_service_->GetLargestFaviconForURL(
      page_url, icon_types, minimum_size_in_pixels, callback, tracker);
}

base::CancelableTaskTracker::TaskId FaviconServiceImpl::GetFaviconForPageURL(
    const GURL& page_url,
    int icon_types,
    int desired_size_in_dip,
    const favicon_base::FaviconResultsCallback& callback,
    base::CancelableTaskTracker* tracker) {
  TRACE_EVENT0("browser", "FaviconServiceImpl::GetFaviconForPageURL");
  return GetFaviconForPageURLImpl(
      page_url, icon_types, GetPixelSizesForFaviconScales(desired_size_in_dip),
      callback, tracker);
}

base::CancelableTaskTracker::TaskId
FaviconServiceImpl::UpdateFaviconMappingsAndFetch(
    const GURL& page_url,
    const std::vector<GURL>& icon_urls,
    int icon_types,
    int desired_size_in_dip,
    const favicon_base::FaviconResultsCallback& callback,
    base::CancelableTaskTracker* tracker) {
  return history_service_->UpdateFaviconMappingsAndFetch(
      page_url, icon_urls, icon_types,
      GetPixelSizesForFaviconScales(desired_size_in_dip), callback, tracker);
}

base::CancelableTaskTracker::TaskId
FaviconServiceImpl::GetLargestRawFaviconForID(
    favicon_base::FaviconID favicon_id,
    const favicon_base::FaviconRawBitmapCallback& callback,
    base::CancelableTaskTracker* tracker) {
  TRACE_EVENT0("browser", "FaviconServiceImpl::GetLargestRawFaviconForID");
  // Use 0 as |desired_size| to get the largest bitmap for |favicon_id| without
  // any resizing.
  int desired_size = 0;
  favicon_base::FaviconResultsCallback callback_runner = base::Bind(
      &FaviconServiceImpl::RunFaviconRawBitmapCallbackWithBitmapResults,
      base::Unretained(this), callback, desired_size);

  return history_service_->GetFaviconForID(favicon_id, desired_size,
                                           callback_runner, tracker);
}

void FaviconServiceImpl::SetFaviconOutOfDateForPage(const GURL& page_url) {
  history_service_->SetFaviconsOutOfDateForPage(page_url);
}

void FaviconServiceImpl::SetImportedFavicons(
    const favicon_base::FaviconUsageDataList& favicon_usage) {
  history_service_->SetImportedFavicons(favicon_usage);
}

void FaviconServiceImpl::MergeFavicon(
    const GURL& page_url,
    const GURL& icon_url,
    favicon_base::IconType icon_type,
    scoped_refptr<base::RefCountedMemory> bitmap_data,
    const gfx::Size& pixel_size) {
  history_service_->MergeFavicon(page_url, icon_url, icon_type, bitmap_data,
                                 pixel_size);
}

void FaviconServiceImpl::SetFavicons(const GURL& page_url,
                                     const GURL& icon_url,
                                     favicon_base::IconType icon_type,
                                     const gfx::Image& image) {
  gfx::ImageSkia image_skia = image.AsImageSkia();
  image_skia.EnsureRepsForSupportedScales();
  const std::vector<gfx::ImageSkiaRep>& image_reps = image_skia.image_reps();
  std::vector<SkBitmap> bitmaps;
  const std::vector<float> favicon_scales = favicon_base::GetFaviconScales();
  for (size_t i = 0; i < image_reps.size(); ++i) {
    // Don't save if the scale isn't one of supported favicon scales.
    if (std::find(favicon_scales.begin(), favicon_scales.end(),
                  image_reps[i].scale()) == favicon_scales.end()) {
      continue;
    }
    bitmaps.push_back(image_reps[i].sk_bitmap());
  }
  history_service_->SetFavicons(page_url, icon_type, icon_url, bitmaps);
}

void FaviconServiceImpl::UnableToDownloadFavicon(const GURL& icon_url) {
  MissingFaviconURLHash url_hash = base::Hash(icon_url.spec());
  missing_favicon_urls_.insert(url_hash);
}

bool FaviconServiceImpl::WasUnableToDownloadFavicon(
    const GURL& icon_url) const {
  MissingFaviconURLHash url_hash = base::Hash(icon_url.spec());
  return missing_favicon_urls_.find(url_hash) != missing_favicon_urls_.end();
}

void FaviconServiceImpl::ClearUnableToDownloadFavicons() {
  missing_favicon_urls_.clear();
}

base::CancelableTaskTracker::TaskId
FaviconServiceImpl::GetFaviconForPageURLImpl(
    const GURL& page_url,
    int icon_types,
    const std::vector<int>& desired_sizes_in_pixel,
    const favicon_base::FaviconResultsCallback& callback,
    base::CancelableTaskTracker* tracker) {
  if (favicon_client_ && favicon_client_->IsNativeApplicationURL(page_url)) {
    return favicon_client_->GetFaviconForNativeApplicationURL(
        page_url, desired_sizes_in_pixel, callback, tracker);
  }
  return history_service_->GetFaviconsForURL(
      page_url, icon_types, desired_sizes_in_pixel, callback, tracker);
}

void FaviconServiceImpl::RunFaviconImageCallbackWithBitmapResults(
    const favicon_base::FaviconImageCallback& callback,
    int desired_size_in_dip,
    const std::vector<favicon_base::FaviconRawBitmapResult>&
        favicon_bitmap_results) {
  TRACE_EVENT0("browser",
               "FaviconServiceImpl::RunFaviconImageCallbackWithBitmapResults");
  favicon_base::FaviconImageResult image_result;
  image_result.image = favicon_base::SelectFaviconFramesFromPNGs(
      favicon_bitmap_results, favicon_base::GetFaviconScales(),
      desired_size_in_dip);
  favicon_base::SetFaviconColorSpace(&image_result.image);

  image_result.icon_url = image_result.image.IsEmpty()
                              ? GURL()
                              : favicon_bitmap_results[0].icon_url;
  callback.Run(image_result);
}

void FaviconServiceImpl::RunFaviconRawBitmapCallbackWithBitmapResults(
    const favicon_base::FaviconRawBitmapCallback& callback,
    int desired_size_in_pixel,
    const std::vector<favicon_base::FaviconRawBitmapResult>&
        favicon_bitmap_results) {
  TRACE_EVENT0(
      "browser",
      "FaviconServiceImpl::RunFaviconRawBitmapCallbackWithBitmapResults");
  callback.Run(
      ResizeFaviconBitmapResult(favicon_bitmap_results, desired_size_in_pixel));
}

}  // namespace favicon
