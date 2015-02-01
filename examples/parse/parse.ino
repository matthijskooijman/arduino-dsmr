/*
 * Permission is hereby granted, free of charge, to anyone
 * obtaining a copy of this document and accompanying files,
 * to do whatever they want with them without any restriction,
 * including, but not limited to, copying, modification and redistribution.
 * NO WARRANTY OF ANY KIND IS PROVIDED.
 *
 * Example that shows how to parse a P1 message and automatically print
 * the result.
*/

#include "dsmr.h"

// Data to parse
const char raw[] =
  "/KFM5KAIFA-METER\r\n"
  "\r\n"
  "1-3:0.2.8(40)\r\n"
  "0-0:1.0.0(150117185916W)\r\n"
  "0-0:96.1.1(0000000000000000000000000000000000)\r\n"
  "1-0:1.8.1(000671.578*kWh)\r\n"
  "1-0:1.8.2(000842.472*kWh)\r\n"
  "1-0:2.8.1(000000.000*kWh)\r\n"
  "1-0:2.8.2(000000.000*kWh)\r\n"
  "0-0:96.14.0(0001)\r\n"
  "1-0:1.7.0(00.333*kW)\r\n"
  "1-0:2.7.0(00.000*kW)\r\n"
  "0-0:17.0.0(999.9*kW)\r\n"
  "0-0:96.3.10(1)\r\n"
  "0-0:96.7.21(00008)\r\n"
  "0-0:96.7.9(00007)\r\n"
  "1-0:99.97.0(1)(0-0:96.7.19)(000101000001W)(2147483647*s)\r\n"
  "1-0:32.32.0(00000)\r\n"
  "1-0:32.36.0(00000)\r\n"
  "0-0:96.13.1()\r\n"
  "0-0:96.13.0()\r\n"
  "1-0:31.7.0(001*A)\r\n"
  "1-0:21.7.0(00.332*kW)\r\n"
  "1-0:22.7.0(00.000*kW)\r\n"
  "0-1:24.1.0(003)\r\n"
  "0-1:96.1.0(0000000000000000000000000000000000)\r\n"
  "0-1:24.2.1(150117180000W)(00473.789*m3)\r\n"
  "0-1:24.4.0(1)\r\n"
  "!6F4A\r\n";

/**
 * Define the data we're interested in, as well as the datastructure to
 * hold the parsed data.
 * Each template argument below results in a field of the same name.
 */
using MyData = ParsedData<
  /* String */ identification,
  /* String */ p1_version,
  /* String */ equipment_id
>;

/**
 * This illustrates looping over all parsed fields using the
 * ParsedData::applyEach method.
 *
 * When passed an instance of this Printer object, applyEach will loop
 * over each field and call Printer::apply, passing a reference to each
 * field in turn. This passes the actual field object, not the field
 * value, so each call to Printer::apply will have a differently typed
 * parameter.
 *
 * For this reason, Printer::apply is a template, resulting in one
 * distinct apply method for each field used. This allows looking up
 * things like Item::name, which is different for every field type,
 * without having to resort to virtual method calls (which result in
 * extra storage usage). The tradeoff is here that there is more code
 * generated (but due to compiler inlining, it's pretty much the same as
 * if you just manually printed all field names and values (with no
 * cost at all if you don't use the Printer).
 */
struct Printer {
  template<typename Item>
  void apply(Item &i) {
    Serial.print(Item::name);
    Serial.print(F(": "));
    Serial.print(i.val());
    Serial.print(Item::unit());
    Serial.println();
  }
};

void setup() {
  Serial.begin(115200);

  MyData data;
  ParseResult<void> res = P1Parser::parse(&data, raw, lengthof(raw));
  if (res.err) {
    // Parsing error, show it
    Serial.println(res.fullError(raw, raw + lengthof(raw)));
  } else {
    // Parsed succesfully, print all values
    data.applyEach(Printer());
  }
}

void loop () {
}
