// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROMECAST_MEDIA_BASE_MEDIA_CODEC_SUPPORT_H_
#define CHROMECAST_MEDIA_BASE_MEDIA_CODEC_SUPPORT_H_

#include <string>

#include "base/callback.h"
#include "chromecast/public/media/decoder_config.h"
#include "media/base/mime_util.h"
#include "media/base/video_codecs.h"

// TODO(slan|servolk): remove when this definition exists in //media.
namespace media {
typedef base::Callback<bool(const std::string&)> IsCodecSupportedCB;
}

namespace chromecast {
namespace media {

// Returns the callback to decide whether a given codec (passed in as a string
// representation of the codec id conforming to RFC 6381) is supported or not.
::media::IsCodecSupportedCB GetIsCodecSupportedOnChromecastCB();

// Converts ::media::VideoCodec to chromecast::media::VideoCodec. Any unknown or
// unsupported codec will be converted to chromecast::media::kCodecUnknown. Note
// |codec_profile| is ignored for codecs other than Dolby Vision.
// TODO(erickung): Remove |codec_profile| parameter by changing VideoCodec enum
// to align with ::media::VideoCodecs.
VideoCodec ToCastVideoCodec(const ::media::VideoCodec codec,
                            const ::media::VideoCodecProfile codec_profile);

// Converts ::media::VideoCodecProfile to chromecast::media::VideoProfile.
VideoProfile ToCastVideoProfile(const ::media::VideoCodecProfile profile);
CodecProfileLevel ToCastCodecProfileLevel(
    const ::media::CodecProfileLevel& codec_profile_level);
}  // namespace media
}  // namespace chromecast

#endif  // CHROMECAST_MEDIA_BASE_MEDIA_CODEC_SUPPORT_H_
