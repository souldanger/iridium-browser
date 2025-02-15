// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// This file has been auto-generated by code_generator_v8.py.
// DO NOT MODIFY!

// This file has been generated from the Jinja2 template in
// third_party/WebKit/Source/bindings/templates/dictionary_impl.cpp.tmpl

// clang-format off
#include "TestDictionary.h"

#include "bindings/core/v8/DoubleOrString.h"
#include "bindings/core/v8/TestInterface2OrUint8Array.h"
#include "bindings/tests/idls/core/TestInterfaceGarbageCollected.h"
#include "bindings/tests/idls/core/TestInterfaceImplementation.h"
#include "bindings/tests/idls/core/TestObject.h"
#include "core/dom/Element.h"
#include "core/events/EventTarget.h"
#include "wtf/Vector.h"

namespace blink {

TestDictionary::TestDictionary() {
  setDoubleOrStringMember(DoubleOrString::fromDouble(3.14));
  setEnumMember("foo");
  setLongMember(1);
  setOtherDoubleOrStringMember(DoubleOrString::fromString("default string value"));
  setRestrictedDoubleMember(3.14);
  setStringOrNullMember("default string value");
  setStringSequenceMember(Vector<String>());
  setTestInterfaceGarbageCollectedSequenceMember(HeapVector<Member<TestInterfaceGarbageCollected>>());
  setTestInterfaceSequenceMember(HeapVector<Member<TestInterfaceImplementation>>());
  setUnrestrictedDoubleMember(3.14);
}

TestDictionary::~TestDictionary() {}

TestDictionary::TestDictionary(const TestDictionary&) = default;

TestDictionary& TestDictionary::operator=(const TestDictionary&) = default;

bool TestDictionary::hasAnyMember() const {
  return !(m_anyMember.isEmpty() || m_anyMember.isNull() || m_anyMember.isUndefined());
}
ScriptValue TestDictionary::anyMember() const {
  return m_anyMember;
}
void TestDictionary::setAnyMember(ScriptValue value) {
  m_anyMember = value;
}
bool TestDictionary::hasBooleanMember() const {
  return m_hasBooleanMember;
}
bool TestDictionary::booleanMember() const {
  DCHECK(m_hasBooleanMember);
  return m_booleanMember;
}
void TestDictionary::setBooleanMember(bool value) {
  m_booleanMember = value;
  m_hasBooleanMember = true;
}
bool TestDictionary::hasCreateMember() const {
  return m_hasCreateMember;
}
bool TestDictionary::createMember() const {
  DCHECK(m_hasCreateMember);
  return m_createMember;
}
void TestDictionary::setCreateMember(bool value) {
  m_createMember = value;
  m_hasCreateMember = true;
}
bool TestDictionary::hasDictionaryMember() const {
  return !m_dictionaryMember.isUndefinedOrNull();
}
Dictionary TestDictionary::dictionaryMember() const {
  return m_dictionaryMember;
}
void TestDictionary::setDictionaryMember(Dictionary value) {
  m_dictionaryMember = value;
}
bool TestDictionary::hasDoubleOrNullMember() const {
  return m_hasDoubleOrNullMember;
}
double TestDictionary::doubleOrNullMember() const {
  DCHECK(m_hasDoubleOrNullMember);
  return m_doubleOrNullMember;
}
void TestDictionary::setDoubleOrNullMember(double value) {
  m_doubleOrNullMember = value;
  m_hasDoubleOrNullMember = true;
}
void TestDictionary::setDoubleOrNullMemberToNull() {
  m_hasDoubleOrNullMember = false;
}
bool TestDictionary::hasDoubleOrStringMember() const {
  return !m_doubleOrStringMember.isNull();
}
const DoubleOrString& TestDictionary::doubleOrStringMember() const {
  return m_doubleOrStringMember;
}
void TestDictionary::setDoubleOrStringMember(const DoubleOrString& value) {
  m_doubleOrStringMember = value;
}
bool TestDictionary::hasDoubleOrStringSequenceMember() const {
  return m_hasDoubleOrStringSequenceMember;
}
const HeapVector<DoubleOrString>& TestDictionary::doubleOrStringSequenceMember() const {
  DCHECK(m_hasDoubleOrStringSequenceMember);
  return m_doubleOrStringSequenceMember;
}
void TestDictionary::setDoubleOrStringSequenceMember(const HeapVector<DoubleOrString>& value) {
  m_doubleOrStringSequenceMember = value;
  m_hasDoubleOrStringSequenceMember = true;
}
bool TestDictionary::hasElementOrNullMember() const {
  return m_elementOrNullMember;
}
Element* TestDictionary::elementOrNullMember() const {
  return m_elementOrNullMember;
}
void TestDictionary::setElementOrNullMember(Element* value) {
  m_elementOrNullMember = value;
}
void TestDictionary::setElementOrNullMemberToNull() {
  m_elementOrNullMember = Member<Element>();
}
bool TestDictionary::hasEnumMember() const {
  return !m_enumMember.isNull();
}
String TestDictionary::enumMember() const {
  return m_enumMember;
}
void TestDictionary::setEnumMember(String value) {
  m_enumMember = value;
}
bool TestDictionary::hasEnumSequenceMember() const {
  return m_hasEnumSequenceMember;
}
const Vector<String>& TestDictionary::enumSequenceMember() const {
  DCHECK(m_hasEnumSequenceMember);
  return m_enumSequenceMember;
}
void TestDictionary::setEnumSequenceMember(const Vector<String>& value) {
  m_enumSequenceMember = value;
  m_hasEnumSequenceMember = true;
}
bool TestDictionary::hasEventTargetMember() const {
  return m_eventTargetMember;
}
EventTarget* TestDictionary::eventTargetMember() const {
  return m_eventTargetMember;
}
void TestDictionary::setEventTargetMember(EventTarget* value) {
  m_eventTargetMember = value;
}
bool TestDictionary::hasInternalDictionarySequenceMember() const {
  return m_hasInternalDictionarySequenceMember;
}
const HeapVector<InternalDictionary>& TestDictionary::internalDictionarySequenceMember() const {
  DCHECK(m_hasInternalDictionarySequenceMember);
  return m_internalDictionarySequenceMember;
}
void TestDictionary::setInternalDictionarySequenceMember(const HeapVector<InternalDictionary>& value) {
  m_internalDictionarySequenceMember = value;
  m_hasInternalDictionarySequenceMember = true;
}
bool TestDictionary::hasLongMember() const {
  return m_hasLongMember;
}
int TestDictionary::longMember() const {
  DCHECK(m_hasLongMember);
  return m_longMember;
}
void TestDictionary::setLongMember(int value) {
  m_longMember = value;
  m_hasLongMember = true;
}
bool TestDictionary::hasObjectMember() const {
  return !(m_objectMember.isEmpty() || m_objectMember.isNull() || m_objectMember.isUndefined());
}
ScriptValue TestDictionary::objectMember() const {
  return m_objectMember;
}
void TestDictionary::setObjectMember(ScriptValue value) {
  m_objectMember = value;
}
bool TestDictionary::hasObjectOrNullMember() const {
  return !(m_objectOrNullMember.isEmpty() || m_objectOrNullMember.isNull() || m_objectOrNullMember.isUndefined());
}
ScriptValue TestDictionary::objectOrNullMember() const {
  return m_objectOrNullMember;
}
void TestDictionary::setObjectOrNullMember(ScriptValue value) {
  m_objectOrNullMember = value;
}
void TestDictionary::setObjectOrNullMemberToNull() {
  m_objectOrNullMember = ScriptValue();
}
bool TestDictionary::hasOtherDoubleOrStringMember() const {
  return !m_otherDoubleOrStringMember.isNull();
}
const DoubleOrString& TestDictionary::otherDoubleOrStringMember() const {
  return m_otherDoubleOrStringMember;
}
void TestDictionary::setOtherDoubleOrStringMember(const DoubleOrString& value) {
  m_otherDoubleOrStringMember = value;
}
bool TestDictionary::hasPrefixGetMember() const {
  return !(m_prefixGetMember.isEmpty() || m_prefixGetMember.isNull() || m_prefixGetMember.isUndefined());
}
ScriptValue TestDictionary::getPrefixGetMember() const {
  return m_prefixGetMember;
}
void TestDictionary::setPrefixGetMember(ScriptValue value) {
  m_prefixGetMember = value;
}
bool TestDictionary::hasRestrictedDoubleMember() const {
  return m_hasRestrictedDoubleMember;
}
double TestDictionary::restrictedDoubleMember() const {
  DCHECK(m_hasRestrictedDoubleMember);
  return m_restrictedDoubleMember;
}
void TestDictionary::setRestrictedDoubleMember(double value) {
  m_restrictedDoubleMember = value;
  m_hasRestrictedDoubleMember = true;
}
bool TestDictionary::hasRuntimeMember() const {
  return m_hasRuntimeMember;
}
bool TestDictionary::runtimeMember() const {
  DCHECK(m_hasRuntimeMember);
  return m_runtimeMember;
}
void TestDictionary::setRuntimeMember(bool value) {
  m_runtimeMember = value;
  m_hasRuntimeMember = true;
}
bool TestDictionary::hasStringArrayMember() const {
  return m_hasStringArrayMember;
}
const Vector<String>& TestDictionary::stringArrayMember() const {
  DCHECK(m_hasStringArrayMember);
  return m_stringArrayMember;
}
void TestDictionary::setStringArrayMember(const Vector<String>& value) {
  m_stringArrayMember = value;
  m_hasStringArrayMember = true;
}
bool TestDictionary::hasStringMember() const {
  return !m_stringMember.isNull();
}
String TestDictionary::stringMember() const {
  return m_stringMember;
}
void TestDictionary::setStringMember(String value) {
  m_stringMember = value;
}
bool TestDictionary::hasStringOrNullMember() const {
  return !m_stringOrNullMember.isNull();
}
String TestDictionary::stringOrNullMember() const {
  return m_stringOrNullMember;
}
void TestDictionary::setStringOrNullMember(String value) {
  m_stringOrNullMember = value;
}
void TestDictionary::setStringOrNullMemberToNull() {
  m_stringOrNullMember = String();
}
bool TestDictionary::hasStringSequenceMember() const {
  return m_hasStringSequenceMember;
}
const Vector<String>& TestDictionary::stringSequenceMember() const {
  DCHECK(m_hasStringSequenceMember);
  return m_stringSequenceMember;
}
void TestDictionary::setStringSequenceMember(const Vector<String>& value) {
  m_stringSequenceMember = value;
  m_hasStringSequenceMember = true;
}
bool TestDictionary::hasTestInterface2OrUint8ArrayMember() const {
  return !m_testInterface2OrUint8ArrayMember.isNull();
}
const TestInterface2OrUint8Array& TestDictionary::testInterface2OrUint8ArrayMember() const {
  return m_testInterface2OrUint8ArrayMember;
}
void TestDictionary::setTestInterface2OrUint8ArrayMember(const TestInterface2OrUint8Array& value) {
  m_testInterface2OrUint8ArrayMember = value;
}
bool TestDictionary::hasTestInterfaceGarbageCollectedMember() const {
  return m_testInterfaceGarbageCollectedMember;
}
TestInterfaceGarbageCollected* TestDictionary::testInterfaceGarbageCollectedMember() const {
  return m_testInterfaceGarbageCollectedMember;
}
void TestDictionary::setTestInterfaceGarbageCollectedMember(TestInterfaceGarbageCollected* value) {
  m_testInterfaceGarbageCollectedMember = value;
}
bool TestDictionary::hasTestInterfaceGarbageCollectedOrNullMember() const {
  return m_testInterfaceGarbageCollectedOrNullMember;
}
TestInterfaceGarbageCollected* TestDictionary::testInterfaceGarbageCollectedOrNullMember() const {
  return m_testInterfaceGarbageCollectedOrNullMember;
}
void TestDictionary::setTestInterfaceGarbageCollectedOrNullMember(TestInterfaceGarbageCollected* value) {
  m_testInterfaceGarbageCollectedOrNullMember = value;
}
void TestDictionary::setTestInterfaceGarbageCollectedOrNullMemberToNull() {
  m_testInterfaceGarbageCollectedOrNullMember = Member<TestInterfaceGarbageCollected>();
}
bool TestDictionary::hasTestInterfaceGarbageCollectedSequenceMember() const {
  return m_hasTestInterfaceGarbageCollectedSequenceMember;
}
const HeapVector<Member<TestInterfaceGarbageCollected>>& TestDictionary::testInterfaceGarbageCollectedSequenceMember() const {
  DCHECK(m_hasTestInterfaceGarbageCollectedSequenceMember);
  return m_testInterfaceGarbageCollectedSequenceMember;
}
void TestDictionary::setTestInterfaceGarbageCollectedSequenceMember(const HeapVector<Member<TestInterfaceGarbageCollected>>& value) {
  m_testInterfaceGarbageCollectedSequenceMember = value;
  m_hasTestInterfaceGarbageCollectedSequenceMember = true;
}
bool TestDictionary::hasTestInterfaceMember() const {
  return m_testInterfaceMember;
}
TestInterfaceImplementation* TestDictionary::testInterfaceMember() const {
  return m_testInterfaceMember;
}
void TestDictionary::setTestInterfaceMember(TestInterfaceImplementation* value) {
  m_testInterfaceMember = value;
}
bool TestDictionary::hasTestInterfaceOrNullMember() const {
  return m_testInterfaceOrNullMember;
}
TestInterfaceImplementation* TestDictionary::testInterfaceOrNullMember() const {
  return m_testInterfaceOrNullMember;
}
void TestDictionary::setTestInterfaceOrNullMember(TestInterfaceImplementation* value) {
  m_testInterfaceOrNullMember = value;
}
void TestDictionary::setTestInterfaceOrNullMemberToNull() {
  m_testInterfaceOrNullMember = Member<TestInterfaceImplementation>();
}
bool TestDictionary::hasTestInterfaceSequenceMember() const {
  return m_hasTestInterfaceSequenceMember;
}
const HeapVector<Member<TestInterfaceImplementation>>& TestDictionary::testInterfaceSequenceMember() const {
  DCHECK(m_hasTestInterfaceSequenceMember);
  return m_testInterfaceSequenceMember;
}
void TestDictionary::setTestInterfaceSequenceMember(const HeapVector<Member<TestInterfaceImplementation>>& value) {
  m_testInterfaceSequenceMember = value;
  m_hasTestInterfaceSequenceMember = true;
}
bool TestDictionary::hasTestObjectSequenceMember() const {
  return m_hasTestObjectSequenceMember;
}
const HeapVector<Member<TestObject>>& TestDictionary::testObjectSequenceMember() const {
  DCHECK(m_hasTestObjectSequenceMember);
  return m_testObjectSequenceMember;
}
void TestDictionary::setTestObjectSequenceMember(const HeapVector<Member<TestObject>>& value) {
  m_testObjectSequenceMember = value;
  m_hasTestObjectSequenceMember = true;
}
bool TestDictionary::hasUint8ArrayMember() const {
  return m_uint8ArrayMember;
}
DOMUint8Array* TestDictionary::uint8ArrayMember() const {
  return m_uint8ArrayMember;
}
void TestDictionary::setUint8ArrayMember(DOMUint8Array* value) {
  m_uint8ArrayMember = value;
}
bool TestDictionary::hasUnrestrictedDoubleMember() const {
  return m_hasUnrestrictedDoubleMember;
}
double TestDictionary::unrestrictedDoubleMember() const {
  DCHECK(m_hasUnrestrictedDoubleMember);
  return m_unrestrictedDoubleMember;
}
void TestDictionary::setUnrestrictedDoubleMember(double value) {
  m_unrestrictedDoubleMember = value;
  m_hasUnrestrictedDoubleMember = true;
}

DEFINE_TRACE(TestDictionary) {
  visitor->trace(m_doubleOrStringMember);
  visitor->trace(m_doubleOrStringSequenceMember);
  visitor->trace(m_elementOrNullMember);
  visitor->trace(m_eventTargetMember);
  visitor->trace(m_internalDictionarySequenceMember);
  visitor->trace(m_otherDoubleOrStringMember);
  visitor->trace(m_testInterface2OrUint8ArrayMember);
  visitor->trace(m_testInterfaceGarbageCollectedMember);
  visitor->trace(m_testInterfaceGarbageCollectedOrNullMember);
  visitor->trace(m_testInterfaceGarbageCollectedSequenceMember);
  visitor->trace(m_testInterfaceMember);
  visitor->trace(m_testInterfaceOrNullMember);
  visitor->trace(m_testInterfaceSequenceMember);
  visitor->trace(m_testObjectSequenceMember);
  visitor->trace(m_uint8ArrayMember);
  IDLDictionaryBase::trace(visitor);
}

}  // namespace blink
