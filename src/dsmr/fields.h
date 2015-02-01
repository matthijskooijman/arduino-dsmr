/**
 * Arduino DSMR parser.
 *
 * This software is licensed under the MIT License.
 *
 * Copyright (c) 2015 Matthijs Kooijman <matthijs@stdin.nl>
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT.  IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * Field parsing functions
 */

#ifndef DSMR_INCLUDE_FIELDS_H
#define DSMR_INCLUDE_FIELDS_H

#include "util.h"
#include "parser.h"

namespace dsmr {

/**
 * Superclass for data items in a P1 message.
 */
template <typename T>
struct ParsedField {
  template <typename F>
  void apply(F& f) {
    f.apply(*static_cast<T*>(this));
  }
  // By defaults, fields have no unit
  static const char *unit() { return ""; }
};

template <typename T, size_t minlen, size_t maxlen>
struct StringField : ParsedField<T> {
  ParseResult<void> parse(const char *str, const char *end) {
    ParseResult<String> res = StringParser::parse_string(minlen, maxlen, str, end);
    if (!res.err)
      static_cast<T*>(this)->val() = res.result;
    return res;
  }
};

// A timestamp is essentially a string using YYMMDDhhmmssX format (where
// X is W or S for wintertime or summertime). Parsing this into a proper
// (UNIX) timestamp is hard to do generically. Parsing it into a
// single integer needs > 4 bytes top fit and isn't very useful (you
// cannot really do any calculation with those values). So we just parse
// into a string for now.
template <typename T>
struct TimestampField : StringField<T, 13, 13> { };

// Value that is parsed as a three-decimal float, but stored as an
// integer (by multiplying by 1000). Supports val() (or implicit cast to
// float) to get the original value, and int_val() to get the more
// efficient integer value. The unit() and int_unit() methods on
// FixedField return the corresponding units for these values.
struct FixedValue {
  operator float() { return val();}
  float val() { return _value / 1000.0;}
  uint32_t int_val() { return _value; }

  uint32_t _value;
};

// Floating point numbers in the message never have more than 3 decimal
// digits. To prevent inefficient floating point operations, we store
// them as a fixed-point number: an integer that stores the value in
// thousands. For example, a value of 1.234 kWh is stored as 1234. This
// effectively means that the integer value is het value in Wh. To allow
// automatic printing of these values, both the original unit and the
// integer unit is passed as a template argument.
template <typename T, const char *_unit, const char *_int_unit>
struct FixedField : ParsedField<T> {
  ParseResult<void> parse(const char *str, const char *end) {
    ParseResult<uint32_t> res = NumParser::parse(3, _unit, str, end);
    if (!res.err)
      static_cast<T*>(this)->val()._value = res.result;
    return res;
  }

  static const char *unit() { return _unit; }
  static const char *int_unit() { return _int_unit; }
};

// A integer number is just represented as an integer.
template <typename T, const char *_unit>
struct IntField : ParsedField<T> {
  ParseResult<void> parse(const char *str, const char *end) {
    ParseResult<uint32_t> res = NumParser::parse(0, _unit, str, end);
    if (!res.err)
      static_cast<T*>(this)->val() = res.result;
    return res;
  }

  static const char *unit() { return _unit; }
};

// TODO: Put the name fields in PROGMEM. Simply using F() isn't allowed
// outside functions, so this is a bit tricky to get right.

namespace fields {

struct equipment_id : StringField<equipment_id, 0, 96> {
  String equipment_id;
  static constexpr ObisId id = ObisId(0, 0, 96, 1, 1);
  static constexpr char name[] = "equipment_id";
  String& val() { return equipment_id; }
};

struct p1_version : StringField<p1_version, 2, 2> {
  String p1_version;
  static constexpr ObisId id = ObisId(1, 3, 0, 2, 8);
  static constexpr char name[] = "p1_version";
  String& val() { return p1_version; }
};

struct identification : ParsedField<identification> {
  String identification;
  // Special (normally invalid) OBIS ID, that is explicitely passed by
  // DsmrP1Parser::parse for the initial identification line
  static constexpr ObisId id = ObisId(255, 255, 255, 255, 255, 255);
  static constexpr char name[] = "identification";

  ParseResult<void> parse(const char *str, const char *end) {
    // Just copy the string verbatim value without any parsing
    concat_hack(identification, str, end - str);
    return ParseResult<void>();
  }

  String& val() { return identification; }
};

} // namespace fields

} // namespace dsmr

#endif // DSMR_INCLUDE_FIELDS_H
