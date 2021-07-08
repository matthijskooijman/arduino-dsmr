/**
 * Arduino DSMR parser.
 *
 * This software is licensed under the MIT License.
 *
 * Copyright (c) 2015 Matthijs Kooijman <matthijs@stdin.nl>
 *
 *------------------------------------------------------------------------------
 * Changed by Willem Aandewiel
 * In the original library it is assumed that the Mbus GAS meter is 
 * always connected to MBUS_ID 1. But this is wrong. Mostly on
 * an initial installation the GAS meter is at MBUS_ID 1 but if an other
 * meter is installed it is connected to the first free MBUS_ID. 
 * So you cannot make any assumption about what mbus is connected to
 * which MBUS_ID. Therfore it is also not possible to check the units
 * on the basis of the MBUS_ID. It can be anything.
 * My assumption is that the device_type of a GAS meter is always "3"
 * and that of, f.i. a water meter is always "5".
 * I hope I'm right but have not been able to verify this with the
 * original documenation. 
 *------------------------------------------------------------------------------
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
#include "parser2.h"

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

struct TimestampedFixedValue : public FixedValue {
  String timestamp;
};

// Some numerical values are prefixed with a timestamp. This is simply
// both of them concatenated, e.g. 0-1:24.2.1(150117180000W)(00473.789*m3)
template <typename T, const char *_unit, const char *_int_unit>
struct TimestampedFixedField : public FixedField<T, _unit, _int_unit> {
  ParseResult<void> parse(const char *str, const char *end) {
    // First, parse timestamp
    ParseResult<String> res = StringParser::parse_string(13, 13, str, end);
    if (res.err)
      return res;

    static_cast<T*>(this)->val().timestamp = res.result;

    // Which is immediately followed by the numerical value
    return FixedField<T, _unit, _int_unit>::parse(res.next, end);
  }
};

// Some gas meters follow different specifications and output
// something like this, e.g.:
// 0-1:24.3.0(150623120000)(00)(60)(1)(0-1:24.2.1)(m3)
// (01100.658)
// Note that the output spans two lines
template <typename T, const char *_unit, const char *_int_unit>
struct DoubleLineTimestampedFixedField : public FixedField<T, _unit, _int_unit> {
  ParseResult<void> parse(const char *str, const char *end) {
    // First, parse timestamp
    ParseResult<String> res = StringParser::parse_string(12, 12, str, end);
    if (res.err)
      return res;

    static_cast<T*>(this)->val().timestamp = res.result;

    // The timestamp is followed by 3 sets of numerical values, parse them
    ParseResult<uint32_t> numres = NumParser::parse(0, NULL, res.next, end);
    if (numres.err)
      return numres;

    numres = NumParser::parse(0, NULL, numres.next, end);
    if (res.err)
      return numres;

    numres = NumParser::parse(0, NULL, numres.next, end);
    if (numres.err)
      return numres;

    // Afther the numerical values, another ObisID is presented,
    // skip the first ')'
    ParseResult<ObisId> idres = ObisIdParser::parse(numres.next + 1, end);
    if (idres.err)
      return idres;

    // The last item on the line is the unit, again skip the closing ')'
    size_t unit_size = strnlen(_unit, 3);
    ParseResult<String> unitres = StringParser::parse_string(unit_size, unit_size, idres.next + 1, end);
    if (unitres.err)
      return unitres;
    
    // Verify the unit.
    const char *unit = unitres.result.c_str();
    if(memcmp(unit, _unit, unit_size) != 0) {
      return unitres.fail((const __FlashStringHelper*)INVALID_UNIT, idres.next + 1);
    }

    // Now move to the next line.
    const char *start = unitres.next;
    if (*start == '\r')
      ++start;

    if (*start == '\n')
      ++start;

    // Since the start line is moved, also move the end line.
    const char *newend = start;
    while (*newend != '\r' && *newend != '\n' && newend != end)
      ++newend;

    // Finally parse the value.
    numres = NumParser::parse(3, NULL, start, newend);
    if (!numres.err)
      static_cast<T*>(this)->val()._value = numres.result;

    return numres;
  }
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

// A RawField is not parsed, the entire value (including any
// parenthesis around it) is returned as a string.
template <typename T>
struct RawField : ParsedField<T> {
  ParseResult<void> parse(const char *str, const char *end) {
    // Just copy the string verbatim value without any parsing
    concat_hack(static_cast<T*>(this)->val(), str, end - str);
    return ParseResult<void>().until(end);
  }
};

namespace fields {

struct units {
  // These variables are inside a struct, since that allows us to make
  // them constexpr and define their values here, but define the storage
  // in a cpp file. Global const(expr) variables have implicitly
  // internal linkage, meaning each cpp file that includes us will have
  // its own copy of the variable. Since we take the address of these
  // variables (passing it as a template argument), this would cause a
  // compiler warning. By putting these in a struct, this is prevented.
  static constexpr char none[] = "";
  static constexpr char kWh[] = "kWh";
  static constexpr char Wh[] = "Wh";
  static constexpr char kW[] = "kW";
  static constexpr char W[] = "W";
  static constexpr char V[] = "V";
  static constexpr char mV[] = "mV";
  static constexpr char A[] = "A";
  static constexpr char mA[] = "mA";
  static constexpr char m3[] = "m3";
  static constexpr char dm3[] = "dm3";
  static constexpr char GJ[] = "GJ";
  static constexpr char MJ[] = "MJ";
};

/*
  added as of https://github.com/matthijskooijman/arduino-dsmr/issues/36
*/
template <typename FieldT>
struct NameConverter {
  public:
    operator const __FlashStringHelper*() const { return reinterpret_cast<const __FlashStringHelper*>(&FieldT::name_progmem); }
};

/*
  changed as of https://github.com/matthijskooijman/arduino-dsmr/issues/36
  
  changed:
    static constexpr const __FlashStringHelper *name = reinterpret_cast<const __FlashStringHelper*>(&name_progmem); \
  to:
    static constexpr NameConverter<dsmr::fields::fieldname> name = {}; \
    
  as by commit "29b1d33fb4397a779b9647f8a3e29ecf9dee116e"
*/

#define DEFINE_FIELD(fieldname, value_t, obis, field_t, field_args...) \
  struct fieldname : field_t<fieldname, ##field_args> { \
    value_t fieldname; \
    bool fieldname ## _present = false; \
    static constexpr ObisId id = obis; \
    static constexpr char name_progmem[] DSMR_PROGMEM = #fieldname; \
    static constexpr NameConverter<dsmr::fields::fieldname> name = {}; \
    value_t& val() { return fieldname; } \
    bool& present() { return fieldname ## _present; } \
  }

/* Meter identification. This is not a normal field, but a
 * specially-formatted first line of the message */
DEFINE_FIELD(identification, String, ObisId(255, 255, 255, 255, 255, 255), RawField);

/* Version information for P1 output */
DEFINE_FIELD(p1_version, String, ObisId(1, 3, 0, 2, 8), StringField, 2, 2);
/* Version information for P1 output (Belgium)*/
DEFINE_FIELD(p1_version_be, String, ObisId(0, 0, 96, 1, 4), StringField, 0, 5);

/* Date-time stamp of the P1 message */
DEFINE_FIELD(timestamp, String, ObisId(0, 0, 1, 0, 0), TimestampField);

/* Equipment identifier */
DEFINE_FIELD(equipment_id, String, ObisId(0, 0, 96, 1, 1), StringField, 0, 96);

/* Meter Reading electricity delivered to client (Tariff 1) in 0,001 kWh */
DEFINE_FIELD(energy_delivered_tariff1, FixedValue, ObisId(1, 0, 1, 8, 1), FixedField, units::kWh, units::Wh);
/* Meter Reading electricity delivered to client (Tariff 2) in 0,001 kWh */
DEFINE_FIELD(energy_delivered_tariff2, FixedValue, ObisId(1, 0, 1, 8, 2), FixedField, units::kWh, units::Wh);
/* Meter Reading electricity delivered by client (Tariff 1) in 0,001 kWh */
DEFINE_FIELD(energy_returned_tariff1, FixedValue, ObisId(1, 0, 2, 8, 1), FixedField, units::kWh, units::Wh);
/* Meter Reading electricity delivered by client (Tariff 2) in 0,001 kWh */
DEFINE_FIELD(energy_returned_tariff2, FixedValue, ObisId(1, 0, 2, 8, 2), FixedField, units::kWh, units::Wh);

/* Tariff indicator electricity. The tariff indicator can also be used
 * to switch tariff dependent loads e.g boilers. This is the
 * responsibility of the P1 user */
DEFINE_FIELD(electricity_tariff, String, ObisId(0, 0, 96, 14, 0), StringField, 4, 4);

/* Actual electricity power delivered (+P) in 1 Watt resolution */
DEFINE_FIELD(power_delivered, FixedValue, ObisId(1, 0, 1, 7, 0), FixedField, units::kW, units::W);
/* Actual electricity power received (-P) in 1 Watt resolution */
DEFINE_FIELD(power_returned, FixedValue, ObisId(1, 0, 2, 7, 0), FixedField, units::kW, units::W);

/* The actual threshold Electricity in kW. Removed in 4.0.7 / 4.2.2 / 5.0 */
DEFINE_FIELD(electricity_threshold, FixedValue, ObisId(0, 0, 17, 0, 0), FixedField, units::kW, units::W);

/* Switch position Electricity (in/out/enabled). Removed in 4.0.7 / 4.2.2 / 5.0 */
DEFINE_FIELD(electricity_switch_position, uint8_t, ObisId(0, 0, 96, 3, 10), IntField, units::none);

/* Number of power failures in any phase */
DEFINE_FIELD(electricity_failures, uint32_t, ObisId(0, 0, 96, 7, 21), IntField, units::none);
/* Number of long power failures in any phase */
DEFINE_FIELD(electricity_long_failures, uint32_t, ObisId(0, 0, 96, 7, 9), IntField, units::none);

/* Power Failure Event Log (long power failures) */
DEFINE_FIELD(electricity_failure_log, String, ObisId(1, 0, 99, 97, 0), RawField);

/* Number of voltage sags in phase L1 */
DEFINE_FIELD(electricity_sags_l1, uint32_t, ObisId(1, 0, 32, 32, 0), IntField, units::none);
/* Number of voltage sags in phase L2 (polyphase meters only) */
DEFINE_FIELD(electricity_sags_l2, uint32_t, ObisId(1, 0, 52, 32, 0), IntField, units::none);
/* Number of voltage sags in phase L3 (polyphase meters only) */
DEFINE_FIELD(electricity_sags_l3, uint32_t, ObisId(1, 0, 72, 32, 0), IntField, units::none);

/* Number of voltage swells in phase L1 */
DEFINE_FIELD(electricity_swells_l1, uint32_t, ObisId(1, 0, 32, 36, 0), IntField, units::none);
/* Number of voltage swells in phase L2 (polyphase meters only) */
DEFINE_FIELD(electricity_swells_l2, uint32_t, ObisId(1, 0, 52, 36, 0), IntField, units::none);
/* Number of voltage swells in phase L3 (polyphase meters only) */
DEFINE_FIELD(electricity_swells_l3, uint32_t, ObisId(1, 0, 72, 36, 0), IntField, units::none);

/* Text message codes: numeric 8 digits (Note: Missing from 5.0 spec)
 * */
DEFINE_FIELD(message_short, String, ObisId(0, 0, 96, 13, 1), StringField, 0, 16);
/* Text message max 2048 characters (Note: Spec says 1024 in comment and
 * 2048 in format spec, so we stick to 2048). */
DEFINE_FIELD(message_long, String, ObisId(0, 0, 96, 13, 0), StringField, 0, 2048);

/* Instantaneous voltage L1 in 0.1V resolution (Note: Spec says V
 * resolution in comment, but 0.1V resolution in format spec. Added in
 * 5.0) */
DEFINE_FIELD(voltage_l1, FixedValue, ObisId(1, 0, 32, 7, 0), FixedField, units::V, units::mV);
/* Instantaneous voltage L2 in 0.1V resolution (Note: Spec says V
 * resolution in comment, but 0.1V resolution in format spec. Added in
 * 5.0) */
DEFINE_FIELD(voltage_l2, FixedValue, ObisId(1, 0, 52, 7, 0), FixedField, units::V, units::mV);
/* Instantaneous voltage L3 in 0.1V resolution (Note: Spec says V
 * resolution in comment, but 0.1V resolution in format spec. Added in
 * 5.0) */
DEFINE_FIELD(voltage_l3, FixedValue, ObisId(1, 0, 72, 7, 0), FixedField, units::V, units::mV);

/* Instantaneous current L1 in A resolution */
//DEFINE_FIELD(current_l1, uint16_t, ObisId(1, 0, 31, 7, 0), IntField, units::A);
/* Instantaneous current L1 in mA resolution */
DEFINE_FIELD(current_l1, FixedValue, ObisId(1, 0, 31, 7, 0), FixedField, units::A, units::mA);
/* Instantaneous current L2 in A resolution */
//DEFINE_FIELD(current_l2, uint16_t, ObisId(1, 0, 51, 7, 0), IntField, units::A);
/* Instantaneous current L2 in mA resolution */
DEFINE_FIELD(current_l2, FixedValue, ObisId(1, 0, 51, 7, 0), FixedField, units::A, units::mA);
/* Instantaneous current L3 in A resolution */
//DEFINE_FIELD(current_l3, uint16_t, ObisId(1, 0, 71, 7, 0), IntField, units::A);
/* Instantaneous current L3 in mA resolution */
DEFINE_FIELD(current_l3, FixedValue, ObisId(1, 0, 71, 7, 0), FixedField, units::A, units::mA);

/* Instantaneous active power L1 (+P) in W resolution */
DEFINE_FIELD(power_delivered_l1, FixedValue, ObisId(1, 0, 21, 7, 0), FixedField, units::kW, units::W);
/* Instantaneous active power L2 (+P) in W resolution */
DEFINE_FIELD(power_delivered_l2, FixedValue, ObisId(1, 0, 41, 7, 0), FixedField, units::kW, units::W);
/* Instantaneous active power L3 (+P) in W resolution */
DEFINE_FIELD(power_delivered_l3, FixedValue, ObisId(1, 0, 61, 7, 0), FixedField, units::kW, units::W);

/* Instantaneous active power L1 (-P) in W resolution */
DEFINE_FIELD(power_returned_l1, FixedValue, ObisId(1, 0, 22, 7, 0), FixedField, units::kW, units::W);
/* Instantaneous active power L2 (-P) in W resolution */
DEFINE_FIELD(power_returned_l2, FixedValue, ObisId(1, 0, 42, 7, 0), FixedField, units::kW, units::W);
/* Instantaneous active power L3 (-P) in W resolution */
DEFINE_FIELD(power_returned_l3, FixedValue, ObisId(1, 0, 62, 7, 0), FixedField, units::kW, units::W);


/* Device-Type */
DEFINE_FIELD(mbus1_device_type,   uint16_t, ObisId(0, 1, 24, 1, 0), IntField, units::none);
/* Equipment identifier (temperature corrected) */
DEFINE_FIELD(mbus1_equipment_id_tc,    String, ObisId(0, 1, 96, 1, 0), StringField, 0, 96);
/* Equipment identifier (Not Temp. Corrected) */
DEFINE_FIELD(mbus1_equipment_id_ntc,  String, ObisId(0, 1, 96, 1, 1), StringField, 0, 96);
/* Valve position (on/off/released) (Note: Removed in 4.0.7 / 4.2.2 / 5.0). */
DEFINE_FIELD(mbus1_valve_position, uint8_t, ObisId(0, 1, 24, 4, 0), IntField, units::none);
/* Last 5-minute value (temperature converted), delivered to client
 * in m3, including decimal values and capture time (Note: 4.x spec has
 * "hourly value") */
DEFINE_FIELD(mbus1_delivered, TimestampedFixedValue, ObisId(0, 1, 24, 2, 1), TimestampedFixedField, units::m3, units::dm3);
// OBIS: Last value of ‘not temperature corrected’ volume, including decimal values and capture time
DEFINE_FIELD(mbus1_delivered_ntc, TimestampedFixedValue, ObisId(0, 1, 24, 2, 3), TimestampedFixedField, units::m3, units::dm3);
/* Last hourly value (temperature compensated or not, depending on the display
 * setting of the device), volume in m3, including decimal values 
 *  double line */
DEFINE_FIELD(mbus1_delivered_dbl, TimestampedFixedValue, ObisId(0, 1, 24, 3, 0), DoubleLineTimestampedFixedField, units::m3, units::dm3);


/* Device-Type */
DEFINE_FIELD(mbus2_device_type, uint16_t, ObisId(0, 2, 24, 1, 0), IntField, units::none);
/* Equipment identifier (temperature corrected) */
DEFINE_FIELD(mbus2_equipment_id_tc, String, ObisId(0, 2, 96, 1, 0), StringField, 0, 96);
/* Equipment identifier (Not Temp. Corrected) */
DEFINE_FIELD(mbus2_equipment_id_ntc,  String, ObisId(0, 2, 96, 1, 1), StringField, 0, 96);
/* Valve position (on/off/released) (Note: Removed in 4.0.7 / 4.2.2 / 5.0). */
DEFINE_FIELD(mbus2_valve_position, uint8_t, ObisId(0, 2, 24, 4, 0), IntField, units::none);
/* Last 5-minute Meter reading and capture time
 * (Note: 4.x spec has "hourly meter reading") */
DEFINE_FIELD(mbus2_delivered, TimestampedFixedValue, ObisId(0, 2, 24, 2, 1), TimestampedFixedField, units::GJ, units::MJ);
// OBIS: Last value of ‘not temperature corrected’ volume, including decimal values and capture time
DEFINE_FIELD(mbus2_delivered_ntc, TimestampedFixedValue, ObisId(0, 2, 24, 2, 3), TimestampedFixedField, units::m3, units::dm3);
/* Last hourly value (temperature compensated or not, depending on the display
 * setting of the device), volume in m3, including decimal values 
 *  double line */
DEFINE_FIELD(mbus2_delivered_dbl, TimestampedFixedValue, ObisId(0, 2, 24, 3, 0), DoubleLineTimestampedFixedField, units::m3, units::dm3);


/* Device-Type */
DEFINE_FIELD(mbus3_device_type, uint16_t, ObisId(0, 3, 24, 1, 0), IntField, units::none);
/* Equipment identifier  (temperature corrected) */
DEFINE_FIELD(mbus3_equipment_id_tc, String, ObisId(0, 3, 96, 1, 0), StringField, 0, 96);
/* Equipment identifier (Not Temp. Corrected) */
DEFINE_FIELD(mbus3_equipment_id_ntc,  String, ObisId(0, 3, 96, 1, 1), StringField, 0, 96);
/* Valve position (on/off/released) (Note: Removed in 4.0.7 / 4.2.2 / 5.0). */
DEFINE_FIELD(mbus3_valve_position, uint8_t, ObisId(0, 3, 24, 4, 0), IntField, units::none);
/* Last 5-minute Meter reading and capture time
 * (Note: 4.x spec has "hourly meter reading") */
DEFINE_FIELD(mbus3_delivered, TimestampedFixedValue, ObisId(0, 3, 24, 2, 1), TimestampedFixedField, units::m3, units::dm3);
// OBIS: Last value of ‘not temperature corrected’ volume, including decimal values and capture time
DEFINE_FIELD(mbus3_delivered_ntc, TimestampedFixedValue, ObisId(0, 3, 24, 2, 3), TimestampedFixedField, units::m3, units::dm3);
/* Last hourly value (temperature compensated or not, depending on the display
 * setting of the device), volume in m3, including decimal values 
 *  double line */
DEFINE_FIELD(mbus3_delivered_dbl, TimestampedFixedValue, ObisId(0, 3, 24, 3, 0), DoubleLineTimestampedFixedField, units::m3, units::dm3);

/* Device-Type */
DEFINE_FIELD(mbus4_device_type, uint16_t, ObisId(0, 4, 24, 1, 0), IntField, units::none);
/* Equipment identifier  (temperature corrected) */
DEFINE_FIELD(mbus4_equipment_id_tc, String, ObisId(0, 4, 96, 1, 0), StringField, 0, 96);
/* Equipment identifier (Not Temp. Corrected) */
DEFINE_FIELD(mbus4_equipment_id_ntc,  String, ObisId(0, 4, 96, 1, 1), StringField, 0, 96);
/* Valve position (on/off/released) (Note: Removed in 4.0.7 / 4.2.2 / 5.0). */
DEFINE_FIELD(mbus4_valve_position, uint8_t, ObisId(0, 4, 24, 4, 0), IntField, units::none);
/* Last 5-minute Meter reading and capture time (e.g. mbus
 * E meter) (Note: 4.x spec has "hourly meter reading") */
DEFINE_FIELD(mbus4_delivered, TimestampedFixedValue, ObisId(0, 4, 24, 2, 1), TimestampedFixedField, units::m3, units::dm3);
// OBIS: Last value of ‘not temperature corrected’ volume , including decimal values and capture time
DEFINE_FIELD(mbus4_delivered_ntc, TimestampedFixedValue, ObisId(0, 4, 24, 2, 3), TimestampedFixedField, units::m3, units::dm3);
/* Last hourly value (temperature compensated or not, depending on the display
 * setting of the device), volume in m3, including decimal values 
 *  double line */
DEFINE_FIELD(mbus4_delivered_dbl, TimestampedFixedValue, ObisId(0, 4, 24, 3, 0), DoubleLineTimestampedFixedField, units::m3, units::dm3);

} // namespace fields

} // namespace dsmr

#endif // DSMR_INCLUDE_FIELDS_H
