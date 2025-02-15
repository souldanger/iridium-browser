// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CC_TILES_DECODED_IMAGE_TRACKER_H_
#define CC_TILES_DECODED_IMAGE_TRACKER_H_

#include <utility>
#include <vector>

#include "base/bind.h"
#include "cc/base/cc_export.h"
#include "cc/tiles/image_controller.h"

class SkImage;

namespace cc {

// This class is the main interface for the rest of the system to request
// decodes. It is responsible for keeping the decodes locked for a number of
// frames, specified as |kNumFramesToLock| in the implementation file.
//
// Note that it is safe to replace ImageController's cache without doing
// anything special with this class, since it retains only ids to the decode
// requests. When defunct ids are then used to try and unlock the image, they
// are silently ignored.
class CC_EXPORT DecodedImageTracker {
 public:
  DecodedImageTracker();
  ~DecodedImageTracker();

  // Request that the given image be decoded. This issues a callback upon
  // completion. The callback takes a bool indicating whether the decode was
  // successful or not.
  void QueueImageDecode(sk_sp<const SkImage> image,
                        const base::Callback<void(bool)>& callback);
  void NotifyFrameFinished();

 private:
  friend class TileManager;
  friend class DecodedImageTrackerTest;

  void set_image_controller(ImageController* controller) {
    image_controller_ = controller;
  }

  void ImageDecodeFinished(const base::Callback<void(bool)>& callback,
                           ImageController::ImageDecodeRequestId id,
                           ImageController::ImageDecodeResult result);

  ImageController* image_controller_ = nullptr;
  std::vector<std::pair<ImageController::ImageDecodeRequestId, int>>
      locked_images_;

  DISALLOW_COPY_AND_ASSIGN(DecodedImageTracker);
};

}  // namespace cc

#endif  // CC_TILES_DECODED_IMAGE_TRACKER_H_
