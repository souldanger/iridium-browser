/*
 * Copyright (C) 2012 Google Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *     * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 * copyright notice, this list of conditions and the following disclaimer
 * in the documentation and/or other materials provided with the
 * distribution.
 *     * Neither the name of Google Inc. nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "core/dom/NodeListsNodeData.h"

#include "core/dom/LiveNodeList.h"

namespace blink {

void NodeListsNodeData::invalidateCaches(const QualifiedName* attrName) {
  for (const auto& cache : m_atomicNameCaches)
    cache.value->invalidateCacheForAttribute(attrName);

  if (attrName)
    return;

  for (auto& cache : m_tagCollectionCacheNS)
    cache.value->invalidateCache();
}

DEFINE_TRACE(NodeListsNodeData) {
  visitor->trace(m_childNodeList);
  visitor->trace(m_atomicNameCaches);
  visitor->trace(m_tagCollectionCacheNS);
}

DEFINE_TRACE_WRAPPERS(NodeListsNodeData) {
  visitor->traceWrappersWithManualWriteBarrier(m_childNodeList);
  for (const auto list : m_atomicNameCaches.values()) {
    if (isHTMLCollectionType(list->type())) {
      visitor->traceWrappersWithManualWriteBarrier(
          static_cast<const HTMLCollection*>(list.get()));
    } else {
      visitor->traceWrappersWithManualWriteBarrier(
          static_cast<const LiveNodeList*>(list.get()));
    }
  }
  for (const auto list : m_tagCollectionCacheNS.values()) {
    visitor->traceWrappersWithManualWriteBarrier(list.get());
  }
}

}  // namespace blink
