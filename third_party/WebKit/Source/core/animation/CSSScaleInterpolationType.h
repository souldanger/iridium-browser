// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CSSScaleInterpolationType_h
#define CSSScaleInterpolationType_h

#include "core/animation/CSSInterpolationType.h"

namespace blink {

class CSSScaleInterpolationType : public CSSInterpolationType {
 public:
  CSSScaleInterpolationType(PropertyHandle property)
      : CSSInterpolationType(property) {
    DCHECK_EQ(cssProperty(), CSSPropertyScale);
  }

  InterpolationValue maybeConvertStandardPropertyUnderlyingValue(
      const ComputedStyle&) const final;
  void composite(UnderlyingValueOwner&,
                 double underlyingFraction,
                 const InterpolationValue&,
                 double interpolationFraction) const final;
  void applyStandardPropertyValue(const InterpolableValue&,
                                  const NonInterpolableValue*,
                                  StyleResolverState&) const final;

 private:
  InterpolationValue maybeConvertNeutral(const InterpolationValue& underlying,
                                         ConversionCheckers&) const final;
  InterpolationValue maybeConvertInitial(const StyleResolverState&,
                                         ConversionCheckers&) const final;
  InterpolationValue maybeConvertInherit(const StyleResolverState&,
                                         ConversionCheckers&) const final;
  InterpolationValue maybeConvertValue(const CSSValue&,
                                       const StyleResolverState*,
                                       ConversionCheckers&) const final;
  void additiveKeyframeHook(InterpolationValue&) const final;

  PairwiseInterpolationValue maybeMergeSingles(
      InterpolationValue&&,
      InterpolationValue&&) const final;
};

}  // namespace blink

#endif  // CSSScaleInterpolationType_h
