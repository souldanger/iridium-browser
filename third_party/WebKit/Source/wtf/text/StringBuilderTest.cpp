/*
 * Copyright (C) 2011 Google Inc. All rights reserved.
 * Copyright (C) 2013 Apple Inc. All rights reserved.
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

#include "wtf/text/StringBuilder.h"

#include "testing/gtest/include/gtest/gtest.h"
#include "wtf/Assertions.h"
#include "wtf/text/CString.h"
#include "wtf/text/CharacterNames.h"
#include "wtf/text/WTFString.h"

namespace WTF {

namespace {

void expectBuilderContent(const String& expected,
                          const StringBuilder& builder) {
  // Not using builder.toString() because it changes internal state of builder.
  if (builder.is8Bit())
    EXPECT_EQ(expected, String(builder.characters8(), builder.length()));
  else
    EXPECT_EQ(expected, String(builder.characters16(), builder.length()));
}

void expectEmpty(const StringBuilder& builder) {
  EXPECT_EQ(0U, builder.length());
  EXPECT_TRUE(builder.isEmpty());
  EXPECT_EQ(0, builder.characters8());
}

}  // namespace

TEST(StringBuilderTest, DefaultConstructor) {
  StringBuilder builder;
  expectEmpty(builder);
}

TEST(StringBuilderTest, Append) {
  StringBuilder builder;
  builder.append(String("0123456789"));
  expectBuilderContent("0123456789", builder);
  builder.append("abcd");
  expectBuilderContent("0123456789abcd", builder);
  builder.append("efgh", 3);
  expectBuilderContent("0123456789abcdefg", builder);
  builder.append("");
  expectBuilderContent("0123456789abcdefg", builder);
  builder.append('#');
  expectBuilderContent("0123456789abcdefg#", builder);

  builder.toString();  // Test after reifyString().
  StringBuilder builder1;
  builder.append("", 0);
  expectBuilderContent("0123456789abcdefg#", builder);
  builder1.append(builder.characters8(), builder.length());
  builder1.append("XYZ");
  builder.append(builder1.characters8(), builder1.length());
  expectBuilderContent("0123456789abcdefg#0123456789abcdefg#XYZ", builder);

  StringBuilder builder2;
  builder2.reserveCapacity(100);
  builder2.append("xyz");
  const LChar* characters = builder2.characters8();
  builder2.append("0123456789");
  EXPECT_EQ(characters, builder2.characters8());

  StringBuilder builder3;
  builder3.append("xyz", 1, 2);
  expectBuilderContent("yz", builder3);

  StringBuilder builder4;
  builder4.append("abc", 5, 3);
  expectEmpty(builder4);

  StringBuilder builder5;
  builder5.append(StringView(StringView("def"), 1, 1));
  expectBuilderContent("e", builder5);

  // append() has special code paths for String backed StringView instead of
  // just char* backed ones.
  StringBuilder builder6;
  builder6.append(String("ghi"), 1, 2);
  expectBuilderContent("hi", builder6);

  // Test appending UChar32 characters to StringBuilder.
  StringBuilder builderForUChar32Append;
  UChar32 frakturAChar = 0x1D504;
  // The fraktur A is not in the BMP, so it's two UTF-16 code units long.
  builderForUChar32Append.append(frakturAChar);
  EXPECT_FALSE(builderForUChar32Append.is8Bit());
  EXPECT_EQ(2U, builderForUChar32Append.length());
  builderForUChar32Append.append(static_cast<UChar32>('A'));
  EXPECT_EQ(3U, builderForUChar32Append.length());
  const UChar resultArray[] = {U16_LEAD(frakturAChar), U16_TRAIL(frakturAChar),
                               'A'};
  expectBuilderContent(String(resultArray, WTF_ARRAY_LENGTH(resultArray)),
                       builderForUChar32Append);
}

TEST(StringBuilderTest, AppendSharingImpl) {
  String string("abc");
  StringBuilder builder1;
  builder1.append(string);
  EXPECT_EQ(string.impl(), builder1.toString().impl());
  EXPECT_EQ(string.impl(), builder1.toAtomicString().impl());

  StringBuilder builder2;
  builder2.append(string, 0, string.length());
  EXPECT_EQ(string.impl(), builder2.toString().impl());
  EXPECT_EQ(string.impl(), builder2.toAtomicString().impl());
}

TEST(StringBuilderTest, ToString) {
  StringBuilder builder;
  builder.append("0123456789");
  String string = builder.toString();
  EXPECT_EQ(String("0123456789"), string);
  EXPECT_EQ(string.impl(), builder.toString().impl());

  // Changing the StringBuilder should not affect the original result of
  // toString().
  builder.append("abcdefghijklmnopqrstuvwxyz");
  EXPECT_EQ(String("0123456789"), string);

  // Changing the StringBuilder should not affect the original result of
  // toString() in case the capacity is not changed.
  builder.reserveCapacity(200);
  string = builder.toString();
  EXPECT_EQ(String("0123456789abcdefghijklmnopqrstuvwxyz"), string);
  builder.append("ABC");
  EXPECT_EQ(String("0123456789abcdefghijklmnopqrstuvwxyz"), string);

  // Changing the original result of toString() should not affect the content of
  // the StringBuilder.
  String string1 = builder.toString();
  EXPECT_EQ(String("0123456789abcdefghijklmnopqrstuvwxyzABC"), string1);
  string1.append("DEF");
  EXPECT_EQ(String("0123456789abcdefghijklmnopqrstuvwxyzABC"),
            builder.toString());
  EXPECT_EQ(String("0123456789abcdefghijklmnopqrstuvwxyzABCDEF"), string1);

  // Resizing the StringBuilder should not affect the original result of
  // toString().
  string1 = builder.toString();
  builder.resize(10);
  builder.append("###");
  EXPECT_EQ(String("0123456789abcdefghijklmnopqrstuvwxyzABC"), string1);
}

TEST(StringBuilderTest, Clear) {
  StringBuilder builder;
  builder.append("0123456789");
  builder.clear();
  expectEmpty(builder);
}

TEST(StringBuilderTest, Array) {
  StringBuilder builder;
  builder.append("0123456789");
  EXPECT_EQ('0', static_cast<char>(builder[0]));
  EXPECT_EQ('9', static_cast<char>(builder[9]));
  builder.toString();  // Test after reifyString().
  EXPECT_EQ('0', static_cast<char>(builder[0]));
  EXPECT_EQ('9', static_cast<char>(builder[9]));
}

TEST(StringBuilderTest, Resize) {
  StringBuilder builder;
  builder.append("0123456789");
  builder.resize(10);
  EXPECT_EQ(10U, builder.length());
  expectBuilderContent("0123456789", builder);
  builder.resize(8);
  EXPECT_EQ(8U, builder.length());
  expectBuilderContent("01234567", builder);

  builder.toString();
  builder.resize(7);
  EXPECT_EQ(7U, builder.length());
  expectBuilderContent("0123456", builder);
  builder.resize(0);
  expectEmpty(builder);
}

TEST(StringBuilderTest, Equal) {
  StringBuilder builder1;
  StringBuilder builder2;
  EXPECT_TRUE(builder1 == builder2);
  EXPECT_TRUE(equal(builder1, static_cast<LChar*>(0), 0));
  EXPECT_TRUE(builder1 == String());
  EXPECT_TRUE(String() == builder1);
  EXPECT_TRUE(builder1 != String("abc"));

  builder1.append("123");
  builder1.reserveCapacity(32);
  builder2.append("123");
  builder1.reserveCapacity(64);
  EXPECT_TRUE(builder1 == builder2);
  EXPECT_TRUE(builder1 == String("123"));
  EXPECT_TRUE(String("123") == builder1);

  builder2.append("456");
  EXPECT_TRUE(builder1 != builder2);
  EXPECT_TRUE(builder2 != builder1);
  EXPECT_TRUE(String("123") != builder2);
  EXPECT_TRUE(builder2 != String("123"));
  builder2.toString();  // Test after reifyString().
  EXPECT_TRUE(builder1 != builder2);

  builder2.resize(3);
  EXPECT_TRUE(builder1 == builder2);

  builder1.toString();  // Test after reifyString().
  EXPECT_TRUE(builder1 == builder2);
}

TEST(StringBuilderTest, ToAtomicString) {
  StringBuilder builder;
  builder.append("123");
  AtomicString atomicString = builder.toAtomicString();
  EXPECT_EQ(String("123"), atomicString);

  builder.reserveCapacity(256);
  for (int i = builder.length(); i < 128; i++)
    builder.append('x');
  AtomicString atomicString1 = builder.toAtomicString();
  EXPECT_EQ(128u, atomicString1.length());
  EXPECT_EQ('x', atomicString1[127]);

  // Later change of builder should not affect the atomic string.
  for (int i = builder.length(); i < 256; i++)
    builder.append('x');
  EXPECT_EQ(128u, atomicString1.length());

  String string = builder.toString();
  AtomicString atomicString2 = builder.toAtomicString();
  // They should share the same StringImpl.
  EXPECT_EQ(atomicString2.impl(), string.impl());
}

TEST(StringBuilderTest, ToAtomicStringOnEmpty) {
  {  // Default constructed.
    StringBuilder builder;
    AtomicString atomicString = builder.toAtomicString();
    EXPECT_EQ(emptyAtom, atomicString);
  }
  {  // With capacity.
    StringBuilder builder;
    builder.reserveCapacity(64);
    AtomicString atomicString = builder.toAtomicString();
    EXPECT_EQ(emptyAtom, atomicString);
  }
  {  // AtomicString constructed from a null string.
    StringBuilder builder;
    builder.append(String());
    AtomicString atomicString = builder.toAtomicString();
    EXPECT_EQ(emptyAtom, atomicString);
  }
  {  // AtomicString constructed from an empty string.
    StringBuilder builder;
    builder.append(emptyString);
    AtomicString atomicString = builder.toAtomicString();
    EXPECT_EQ(emptyAtom, atomicString);
  }
  {  // AtomicString constructed from an empty StringBuilder.
    StringBuilder builder;
    StringBuilder emptyBuilder;
    builder.append(emptyBuilder);
    AtomicString atomicString = builder.toAtomicString();
    EXPECT_EQ(emptyAtom, atomicString);
  }
  {  // AtomicString constructed from an empty char* string.
    StringBuilder builder;
    builder.append("", 0);
    AtomicString atomicString = builder.toAtomicString();
    EXPECT_EQ(emptyAtom, atomicString);
  }
  {  // Cleared StringBuilder.
    StringBuilder builder;
    builder.append("WebKit");
    builder.clear();
    AtomicString atomicString = builder.toAtomicString();
    EXPECT_EQ(emptyAtom, atomicString);
  }
}

TEST(StringBuilderTest, Substring) {
  {  // Default constructed.
    StringBuilder builder;
    String substring = builder.substring(0, 10);
    EXPECT_EQ(emptyString, substring);
  }
  {  // With capacity.
    StringBuilder builder;
    builder.reserveCapacity(64);
    builder.append("abc");
    String substring = builder.substring(2, 10);
    EXPECT_EQ(String("c"), substring);
  }
}

TEST(StringBuilderTest, AppendNumberDoubleUChar) {
  const double someNumber = 1.2345;
  StringBuilder reference;
  reference.append(replacementCharacter);  // Make it UTF-16.
  reference.append(String::number(someNumber));
  StringBuilder test;
  test.append(replacementCharacter);
  test.appendNumber(someNumber);
  EXPECT_EQ(reference, test);
}

}  // namespace WTF
