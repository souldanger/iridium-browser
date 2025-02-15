// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MultiColumnFragmentainerGroup_h
#define MultiColumnFragmentainerGroup_h

#include "core/layout/LayoutMultiColumnFlowThread.h"
#include "wtf/Allocator.h"

namespace blink {

// A group of columns, that are laid out in the inline progression direction,
// all with the same column height.
//
// When a multicol container is inside another fragmentation context, and said
// multicol container lives in multiple outer fragmentainers (pages / columns),
// we need to put these inner columns into separate groups, with one group per
// outer fragmentainer. Such a group of columns is what comprises a "row of
// column boxes" in spec lingo.
//
// Column balancing, when enabled, takes place within a column fragmentainer
// group.
//
// Each fragmentainer group may have its own actual column count (if there are
// unused columns because of forced breaks, for example). If there are multiple
// fragmentainer groups, the actual column count must not exceed the used column
// count (the one calculated based on column-count and column-width from CSS),
// or they'd overflow the outer fragmentainer in the inline direction. If we
// need more columns than what a group has room for, we'll create another group
// and put them there (and make them appear in the next outer fragmentainer).
class MultiColumnFragmentainerGroup {
  DISALLOW_NEW_EXCEPT_PLACEMENT_NEW();

 public:
  MultiColumnFragmentainerGroup(const LayoutMultiColumnSet&);

  const LayoutMultiColumnSet& columnSet() const { return m_columnSet; }

  bool isFirstGroup() const;
  bool isLastGroup() const;

  // Position within the LayoutMultiColumnSet.
  LayoutUnit logicalTop() const { return m_logicalTop; }
  void setLogicalTop(LayoutUnit logicalTop) { m_logicalTop = logicalTop; }

  LayoutUnit logicalHeight() const { return m_columnHeight; }

  LayoutSize offsetFromColumnSet() const;

  // Return the block offset from the enclosing fragmentation context, if
  // nested. In the coordinate space of the enclosing fragmentation context.
  LayoutUnit blockOffsetInEnclosingFragmentationContext() const;

  // The top of our flow thread portion
  LayoutUnit logicalTopInFlowThread() const { return m_logicalTopInFlowThread; }
  void setLogicalTopInFlowThread(LayoutUnit logicalTopInFlowThread) {
    m_logicalTopInFlowThread = logicalTopInFlowThread;
  }

  // The bottom of our flow thread portion
  LayoutUnit logicalBottomInFlowThread() const {
    return m_logicalBottomInFlowThread;
  }
  void setLogicalBottomInFlowThread(LayoutUnit logicalBottomInFlowThread) {
    m_logicalBottomInFlowThread = logicalBottomInFlowThread;
  }

  // The height of the flow thread portion for the entire fragmentainer group.
  LayoutUnit logicalHeightInFlowThread() const {
    // Due to negative margins, logical bottom may actually end up above logical
    // top, but we never want to return negative logical heights.
    return (m_logicalBottomInFlowThread - m_logicalTopInFlowThread)
        .clampNegativeToZero();
  }
  // The height of the flow thread portion for the specified fragmentainer.
  // The last fragmentainer may not be using all available space.
  LayoutUnit logicalHeightInFlowThreadAt(unsigned columnIndex) const;

  void resetColumnHeight();
  bool recalculateColumnHeight(LayoutMultiColumnSet&);

  LayoutSize flowThreadTranslationAtOffset(LayoutUnit,
                                           LayoutBox::PageBoundaryRule,
                                           CoordinateSpaceConversion) const;
  LayoutUnit columnLogicalTopForOffset(LayoutUnit offsetInFlowThread) const;

  // If SnapToColumnPolicy is SnapToColumn, visualPointToFlowThreadPoint() won't
  // return points that lie outside the bounds of the columns: Before converting
  // to a flow thread position, if the block direction coordinate is outside the
  // column, snap to the bounds of the column, and reset the inline direction
  // coordinate to the start position in the column. The effect of this is that
  // if the block position is before the column rectangle, we'll get to the
  // beginning of this column, while if the block position is after the column
  // rectangle, we'll get to the beginning of the next column. This is behavior
  // that positionForPoint() depends on.
  enum SnapToColumnPolicy { DontSnapToColumn, SnapToColumn };
  LayoutPoint visualPointToFlowThreadPoint(
      const LayoutPoint& visualPoint,
      SnapToColumnPolicy = DontSnapToColumn) const;

  LayoutRect fragmentsBoundingBox(
      const LayoutRect& boundingBoxInFlowThread) const;

  LayoutRect flowThreadPortionRectAt(unsigned columnIndex) const;
  LayoutRect flowThreadPortionOverflowRectAt(unsigned columnIndex) const;

  // Get the first and the last column intersecting the specified block range.
  // Note that |logicalBottomInFlowThread| is an exclusive endpoint.
  void columnIntervalForBlockRangeInFlowThread(
      LayoutUnit logicalTopInFlowThread,
      LayoutUnit logicalBottomInFlowThread,
      unsigned& firstColumn,
      unsigned& lastColumn) const;

  // Get the first and the last column intersecting the specified visual
  // rectangle.
  void columnIntervalForVisualRect(const LayoutRect&,
                                   unsigned& firstColumn,
                                   unsigned& lastColumn) const;

  LayoutRect calculateOverflow() const;

  unsigned columnIndexAtOffset(LayoutUnit offsetInFlowThread,
                               LayoutBox::PageBoundaryRule) const;

  // The "CSS actual" value of column-count. This includes overflowing columns,
  // if any.
  // Returns 1 or greater, never 0.
  unsigned actualColumnCount() const;

 private:
  LayoutUnit heightAdjustedForRowOffset(LayoutUnit height) const;
  LayoutUnit calculateMaxColumnHeight() const;
  void setAndConstrainColumnHeight(LayoutUnit);

  LayoutUnit rebalanceColumnHeightIfNeeded() const;

  LayoutRect columnRectAt(unsigned columnIndex) const;
  LayoutUnit logicalTopInFlowThreadAt(unsigned columnIndex) const {
    return m_logicalTopInFlowThread + columnIndex * m_columnHeight;
  }

  // Return the column that the specified visual point belongs to. Only the
  // coordinate on the column progression axis is relevant. Every point belongs
  // to a column, even if said point is not inside any of the columns.
  unsigned columnIndexAtVisualPoint(const LayoutPoint& visualPoint) const;

  const LayoutMultiColumnSet& m_columnSet;

  LayoutUnit m_logicalTop;
  LayoutUnit m_logicalTopInFlowThread;
  LayoutUnit m_logicalBottomInFlowThread;

  LayoutUnit m_columnHeight;

  LayoutUnit m_maxColumnHeight;  // Maximum column height allowed.
};

// List of all fragmentainer groups within a column set. There will always be at
// least one group. Deleting the one group is not allowed (or possible). There
// will be more than one group if the owning column set lives in multiple outer
// fragmentainers (e.g. multicol inside paged media).
class CORE_EXPORT MultiColumnFragmentainerGroupList {
  DISALLOW_NEW();

 public:
  MultiColumnFragmentainerGroupList(LayoutMultiColumnSet&);
  ~MultiColumnFragmentainerGroupList();

  // Add an additional fragmentainer group to the end of the list, and return
  // it.
  MultiColumnFragmentainerGroup& addExtraGroup();

  // Remove all fragmentainer groups but the first one.
  void deleteExtraGroups();

  MultiColumnFragmentainerGroup& first() { return m_groups.front(); }
  const MultiColumnFragmentainerGroup& first() const {
    return m_groups.front();
  }
  MultiColumnFragmentainerGroup& last() { return m_groups.back(); }
  const MultiColumnFragmentainerGroup& last() const { return m_groups.back(); }

  typedef Vector<MultiColumnFragmentainerGroup, 1>::iterator iterator;
  typedef Vector<MultiColumnFragmentainerGroup, 1>::const_iterator
      const_iterator;

  iterator begin() { return m_groups.begin(); }
  const_iterator begin() const { return m_groups.begin(); }
  iterator end() { return m_groups.end(); }
  const_iterator end() const { return m_groups.end(); }

  size_t size() const { return m_groups.size(); }
  MultiColumnFragmentainerGroup& operator[](size_t i) { return m_groups.at(i); }
  const MultiColumnFragmentainerGroup& operator[](size_t i) const {
    return m_groups.at(i);
  }

  void append(const MultiColumnFragmentainerGroup& group) {
    m_groups.push_back(group);
  }
  void shrink(size_t size) { m_groups.shrink(size); }

 private:
  LayoutMultiColumnSet& m_columnSet;

  Vector<MultiColumnFragmentainerGroup, 1> m_groups;
};

}  // namespace blink

#endif  // MultiColumnFragmentainerGroup_h
