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

#include "dsmr2.h"

/**
 * Define the data we're interested in, as well as the datastructure to
 * hold the parsed data. This list shows all supported fields, remove
 * any fields you are not using from the below list to make the parsing
 * and printing code smaller.
 * Each template argument below results in a field of the same name.
 */
using MyData = ParsedData<
  /* String */                 identification
  /* String */                ,p1_version
  /* String */                ,p1_version_be
  /* String */                ,timestamp
  /* String */                ,equipment_id
  /* FixedValue */            ,energy_delivered_tariff1
  /* FixedValue */            ,energy_delivered_tariff2
  /* FixedValue */            ,energy_returned_tariff1
  /* FixedValue */            ,energy_returned_tariff2
  /* String */                ,electricity_tariff
  /* FixedValue */            ,power_delivered
  /* FixedValue */            ,power_returned
  /* FixedValue */            ,electricity_threshold
  /* uint8_t */               ,electricity_switch_position
  /* uint32_t */              ,electricity_failures
  /* uint32_t */              ,electricity_long_failures
  /* String */                ,electricity_failure_log
  /* uint32_t */              ,electricity_sags_l1
  /* uint32_t */              ,electricity_sags_l2
  /* uint32_t */              ,electricity_sags_l3
  /* uint32_t */              ,electricity_swells_l1
  /* uint32_t */              ,electricity_swells_l2
  /* uint32_t */              ,electricity_swells_l3
  /* String */                ,message_short
  /* String */                ,message_long
  /* FixedValue */            ,voltage_l1
  /* FixedValue */            ,voltage_l2
  /* FixedValue */            ,voltage_l3
  /* FixedValue */            ,current_l1
  /* FixedValue */            ,current_l2
  /* FixedValue */            ,current_l3
  /* FixedValue */            ,power_delivered_l1
  /* FixedValue */            ,power_delivered_l2
  /* FixedValue */            ,power_delivered_l3
  /* FixedValue */            ,power_returned_l1
  /* FixedValue */            ,power_returned_l2
  /* FixedValue */            ,power_returned_l3
  /* uint16_t */              ,mbus1_device_type
  /* String */                ,mbus1_equipment_id_tc
  /* String */                ,mbus1_equipment_id_ntc
  /* uint8_t */               ,mbus1_valve_position
  /* TimestampedFixedValue */ ,mbus1_delivered
  /* TimestampedFixedValue */ ,mbus1_delivered_ntc
  /* TimestampedFixedValue */ ,mbus1_delivered_dbl
  /* uint16_t */              ,mbus2_device_type
  /* String */                ,mbus2_equipment_id_tc
  /* String */                ,mbus2_equipment_id_ntc
  /* uint8_t */               ,mbus2_valve_position
  /* TimestampedFixedValue */ ,mbus2_delivered
  /* TimestampedFixedValue */ ,mbus2_delivered_ntc
  /* TimestampedFixedValue */ ,mbus2_delivered_dbl
  /* uint16_t */              ,mbus3_device_type
  /* String */                ,mbus3_equipment_id_tc
  /* String */                ,mbus3_equipment_id_ntc
  /* uint8_t */               ,mbus3_valve_position
  /* TimestampedFixedValue */ ,mbus3_delivered
  /* TimestampedFixedValue */ ,mbus3_delivered_ntc
  /* TimestampedFixedValue */ ,mbus3_delivered_dbl
  /* uint16_t */              ,mbus4_device_type
  /* String */                ,mbus4_equipment_id_tc
  /* String */                ,mbus4_equipment_id_ntc
  /* uint8_t */               ,mbus4_valve_position
  /* TimestampedFixedValue */ ,mbus4_delivered
  /* TimestampedFixedValue */ ,mbus4_delivered_ntc
  /* TimestampedFixedValue */ ,mbus4_delivered_dbl
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
    if (i.present()) {
      Serial.print(Item::name);
      Serial.print(F(": "));
      Serial.print(i.val());
      Serial.print(Item::unit());
      Serial.println();
    }
  }
};

// Set up to read from the second serial port, and use D2 as the request
// pin. On boards with only one (USB) serial port, you can also use
// SoftwareSerial.
#ifdef ARDUINO_ARCH_ESP32
// Create Serial1 connected to UART 1
HardwareSerial Serial1(1);
#endif
P1Reader reader(&Serial1, 2);

unsigned long last;

void setup() {
  Serial.begin(115200);
  Serial1.begin(115200);
  delay(250);
  Serial.println("\r\nAnd then it all starts ...\r\n");
  
  #ifdef VCC_ENABLE
  // This is needed on Pinoccio Scout boards to enable the 3V3 pin.
  pinMode(VCC_ENABLE, OUTPUT);
  digitalWrite(VCC_ENABLE, HIGH);
  #endif

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
