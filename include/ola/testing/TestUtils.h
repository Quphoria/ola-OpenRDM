/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 * TestUtils.h
 * Useful functions that improve upon CPPUNIT's test functions
 * Copyright (C) 2012 Simon Newton
 */

#ifndef INCLUDE_OLA_TESTING_TESTUTILS_H_
#define INCLUDE_OLA_TESTING_TESTUTILS_H_

#include <stdint.h>
#include <cppunit/Asserter.h>
#include <cppunit/SourceLine.h>
#include <cppunit/extensions/HelperMacros.h>
#include <sstream>
#include <vector>


namespace ola {
namespace testing {

using std::vector;

// Assert that two data blocks are the same.
void ASSERT_DATA_EQUALS(unsigned int line,
                        const uint8_t *expected,
                        unsigned int expected_length,
                        const uint8_t *actual,
                        unsigned int actual_length);

// Private, use OLA_ASSERT_VECTOR_EQ below
template <typename T>
void _AssertVectorEq(const CPPUNIT_NS::SourceLine &source_line,
                     const vector<T> &t1,
                     const vector<T> &t2) {
  CPPUNIT_NS::assertEquals(t1.size(), t2.size(), source_line,
                           "Vector sizes not equal");

  typename vector<T>::const_iterator iter1 = t1.begin();
  typename vector<T>::const_iterator iter2 = t2.begin();
  while (iter1 != t1.end())
    CPPUNIT_NS::assertEquals(*iter1++, *iter2++, source_line,
                             "Vector elements not equal");
}

// Useful macros. This allows us to switch between unit testing frameworks in
// the future.
#define OLA_ASSERT(condition)  \
  CPPUNIT_ASSERT(condition)

#define OLA_ASSERT_TRUE(condition)  \
  CPPUNIT_ASSERT(condition)

#define OLA_ASSERT_FALSE(condition)  \
  CPPUNIT_ASSERT(!(condition))

#define OLA_ASSERT_EQ(expected, output)  \
  CPPUNIT_ASSERT_EQUAL(expected, output)

#define OLA_ASSERT_NE(expected, output)  \
  CPPUNIT_ASSERT((expected) != (output))

#define OLA_ASSERT_LT(expected, output)  \
  CPPUNIT_ASSERT((expected) < (output))

#define OLA_ASSERT_GT(expected, output)  \
  CPPUNIT_ASSERT((expected) > (output))

#define OLA_ASSERT_VECTOR_EQ(expected, output)  \
  ola::testing::_AssertVectorEq(CPPUNIT_SOURCELINE(), (expected), (output))

#define OLA_ASSERT_NULL(value) \
  CPPUNIT_NS::Asserter::failIf( \
    NULL != value, \
    CPPUNIT_NS::Message("Expression: " #value " != NULL"), \
    CPPUNIT_SOURCELINE())
}  // testing
}  // ola
#endif  // INCLUDE_OLA_TESTING_TESTUTILS_H_
