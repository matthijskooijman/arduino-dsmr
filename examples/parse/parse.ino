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

#include "dsmr2.h"

// Data to parse
const char rawcrc[] =
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
  
// Data to parse
const char rawnocrc[] =
     "/KMP5 KA6U001585654321\r\n"
    "\r\n"
    "0-0:96.1.1(4530303336303033373839373331234567)\r\n"
    "1-0:1.8.1(000180.670*kWh)\r\n"
    "1-0:1.8.2(000091.890*kWh)\r\n"
    "1-0:2.8.1(000117.100*kWh)\r\n"
    "1-0:2.8.2(000079.500*kWh)\r\n"
    "0-0:96.14.0(0002)\r\n"
    "1-0:1.7.0(212.33*kW)\r\n"
    "1-0:2.7.0(029.73*kW)\r\n"
    "0-0:96.13.0()\r\n"
    "0-0:96.13.1()\r\n"
    "0-1:24.1.0(3)\r\n"
    "0-1:96.1.0(4730301234567031363532303530323136)\r\n"
    "0-1:24.3.0(140101004100)(08)(60)(1)(0-1:24.2.1)(m3)\r\n"
    "(00100.006)\r\n"
    "0-3:24.1.0(3)\r\n"
    "0-3:96.1.0(4730301234567031363532303530323136)\r\n"
    "0-3:24.3.0(140101004100)(08)(60)(1)(0-1:24.2.1)(m3)\r\n"
    "(00300.006)\r\n"
    "0-2:24.1.0(3)\r\n"
    "0-2:96.1.0(4730301234567031363532303530323136)\r\n"
    "0-2:24.3.0(140101004100)(08)(60)(1)(0-1:24.2.1)(m3)\r\n"
    "(00200.006)\r\n"
    "0-4:24.1.0(3)\r\n"
    "0-4:96.1.0(4730301234567031363532303530323136)\r\n"
    "0-4:24.3.0(140101004100)(08)(60)(1)(0-1:24.2.1)(m3)\r\n"
    "(00400.006)\r\n"
    "!\r\n";

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

void setup() {
  Serial.begin(115200);
  while(!Serial) {/*wait a while*/ delay(100);}
  delay(2000);
  Serial.println("\r\n----------------------------------------------------");

  Serial.print(rawcrc);
  Serial.println("----------------------------------------------------");
  MyData data;
  ParseResult<void> res1 = P1Parser::parse(&data, rawcrc, lengthof(rawcrc), true, true);
  if (res1.err) {
    // Parsing error, show it
    Serial.println("P1Parser: Error found!");
    Serial.println(res1.fullError(rawcrc, rawcrc + lengthof(rawcrc)));
  } else {
    // Parsed succesfully, print all values
    Serial.println("P1Parser: OK!\r\n");
    data.applyEach(Printer());
  }

  Serial.println("\r\n----------------------------------------------------");
  Serial.print(rawnocrc);
  Serial.println("----------------------------------------------------");
  data = {};
  ParseResult<void> res2 = P1Parser::parse(&data, rawnocrc, lengthof(rawnocrc), true, false);
  if (res2.err) {
    // Parsing error, show it
    Serial.println("P1Parser: Error found!");
    Serial.println(res2.fullError(rawnocrc, rawnocrc + lengthof(rawnocrc)));
  } else {
    // Parsed succesfully, print all values
    Serial.println("P1Parser: OK!\r\n");
    data.applyEach(Printer());
  }
  
}

void loop () {
}
