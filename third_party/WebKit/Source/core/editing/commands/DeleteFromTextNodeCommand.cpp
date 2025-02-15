/*
 * Copyright (C) 2005, 2008 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE COMPUTER, INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE COMPUTER, INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "core/editing/commands/DeleteFromTextNodeCommand.h"

#include "bindings/core/v8/ExceptionState.h"
#include "core/dom/Text.h"
#include "core/editing/EditingUtilities.h"

namespace blink {

DeleteFromTextNodeCommand::DeleteFromTextNodeCommand(Text* node,
                                                     unsigned offset,
                                                     unsigned count)
    : SimpleEditCommand(node->document()),
      m_node(node),
      m_offset(offset),
      m_count(count) {
  DCHECK(m_node);
  DCHECK_LE(m_offset, m_node->length());
  DCHECK_LE(m_offset + m_count, m_node->length());
}

void DeleteFromTextNodeCommand::doApply(EditingState*) {
  DCHECK(m_node);

  document().updateStyleAndLayoutTree();
  if (!hasEditableStyle(*m_node))
    return;

  DummyExceptionStateForTesting exceptionState;
  m_text = m_node->substringData(m_offset, m_count, exceptionState);
  if (exceptionState.hadException())
    return;

  m_node->deleteData(m_offset, m_count, exceptionState);
}

void DeleteFromTextNodeCommand::doUnapply() {
  DCHECK(m_node);

  if (!hasEditableStyle(*m_node))
    return;

  m_node->insertData(m_offset, m_text, IGNORE_EXCEPTION_FOR_TESTING);
}

DEFINE_TRACE(DeleteFromTextNodeCommand) {
  visitor->trace(m_node);
  SimpleEditCommand::trace(visitor);
}

}  // namespace blink
