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
};

// TODO: Put the name fields in PROGMEM. Simply using F() isn't allowed
// outside functions, so this is a bit tricky to get right.

namespace fields {

struct equipment_id : ParsedField<equipment_id> {
  String equipment_id;
  static constexpr ObisId id = ObisId(0, 0, 96, 1, 1);
  static constexpr char name[] = "equipment_id";

  ParseResult<void> parse(const char *str, const char *end) {
    ParseResult<String> res = StringParser::parse_string(0, 96, str, end);
    equipment_id = res.result;
    return res;
  }

  String& get() { return equipment_id; }
};

struct p1_version : ParsedField<p1_version> {
  String p1_version;
  static constexpr ObisId id = ObisId(1, 3, 0, 2, 8);
  static constexpr char name[] = "p1_version";

  ParseResult<void> parse(const char *str, const char *end) {
    ParseResult<String> res = StringParser::parse_string(2, 2, str, end);
    p1_version = res.result;
    return res;
  }
  String get() { return p1_version; }
};

struct identification : ParsedField<identification> {
  String identification;
  // Special (normally invalid) OBIS ID, that is explicitely passed by
  // DsmrP1Parser::parse for the initial identification line
  static constexpr ObisId id = ObisId(255, 255, 255, 255, 255, 255);
  static constexpr char name[] = "identification";

  ParseResult<void> parse(const char *str, const char *end) {
    concat_hack(identification, str, end - str);
    return ParseResult<void>();
  }

  String& get() { return identification; }
};

} // namespace fields

} // namespace dsmr

#endif // DSMR_INCLUDE_FIELDS_H
