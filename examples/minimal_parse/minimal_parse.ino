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
const char msg[] =
  "/KFM5KAIFA-METER\r\n"
  "\r\n"
  "1-0:1.8.1(000671.578*kWh)\r\n"
  "1-0:1.7.0(00.318*kW)\r\n"
  "!1E1D\r\n";

/**
 * Define the data we're interested in, as well as the datastructure to
 * hold the parsed data.
 * Each template argument below results in a field of the same name.
 */
using MyData = ParsedData<
  /* String */ identification,
  /* FixedValue */ power_delivered
>;

void setup() {
  Serial.begin(115200);

  MyData data;
  ParseResult<void> res = P1Parser::parse(&data, msg, lengthof(msg));
  if (res.err) {
    // Parsing error, show it
    Serial.println(res.fullError(msg, msg + lengthof(msg)));
  } else if (!data.all_present()) {
    Serial.println("Some fields are missing");
  } else {
    // Succesfully parsed, print results:
    Serial.println(data.identification);
    Serial.print(data.power_delivered.int_val());
    Serial.println("W");
  }
}

void loop () {
}
