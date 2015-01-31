/*
 * Permission is hereby granted, free of charge, to anyone
 * obtaining a copy of this document and accompanying files,
 * to do whatever they want with them without any restriction,
 * including, but not limited to, copying, modification and redistribution.
 * NO WARRANTY OF ANY KIND IS PROVIDED.
 *
 * Example that shows how to periodically read a P1 message from a
 * serial port and automatically print the result.
*/

#include "dsmr.h"

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
    Serial.println();
  }
};

// Set up to read from the second serial port, and use D2 as the request
// pin. On boards with only one (USB) serial port, you can also use
// SoftwareSerial.
P1Reader reader(&Serial1, D2);

unsigned long last;

void setup() {
  Serial.begin(115200);
  Serial1.begin(115200);
  pinMode(VCC_ENABLE, OUTPUT);
  digitalWrite(VCC_ENABLE, HIGH);

  // start a read right away
  reader.enable(true);
  last = millis();
}

void loop () {
  // Allow the reader to check the serial buffer regularly
  reader.loop();

  // Every minute, fire off a one-off reading
  unsigned long now = millis();
  if (now - last > 60000) {
    reader.enable(true);
    last = now;
  }

  if (reader.available()) {
    MyData data;
    String err;
    if (reader.parse(&data, &err)) {
      // Parse succesful, print result
      data.applyEach(Printer());
    } else {
      // Parser error, print error
      Serial.println(err);
    }
  }
}
