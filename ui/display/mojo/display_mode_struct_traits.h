// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_DISPLAY_MOJO_DISPLAY_MODE_STRUCT_TRAITS_H_
#define UI_DISPLAY_MOJO_DISPLAY_MODE_STRUCT_TRAITS_H_

#include "ui/display/types/display_mode.h"
#include "ui/display/mojo/display_mode.mojom.h"
#include "ui/gfx/geometry/size.h"

namespace mojo {

template <>
struct StructTraits<display::mojom::DisplayModeDataView,
                    std::unique_ptr<display::DisplayMode>> {
  static const gfx::Size& size(
      const std::unique_ptr<display::DisplayMode>& display_mode) {
    return display_mode->size();
  }

  static bool is_interlaced(
      const std::unique_ptr<display::DisplayMode>& display_mode) {
    return display_mode->is_interlaced();
  }

  static float refresh_rate(
      const std::unique_ptr<display::DisplayMode>& display_mode) {
    return display_mode->refresh_rate();
  }

  static bool Read(display::mojom::DisplayModeDataView data,
                   std::unique_ptr<display::DisplayMode>* out) {
    gfx::Size size;
    if (!data.ReadSize(&size))
      return false;
    *out = base::MakeUnique<display::DisplayMode>(size, data.is_interlaced(),
                                                  data.refresh_rate());
    return true;
  }
};

}  // namespace mojo

#endif  // UI_DISPLAY_MOJO_DISPLAY_MODE_STRUCT_TRAITS_H_
