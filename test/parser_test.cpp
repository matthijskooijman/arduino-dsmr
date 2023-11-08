#include "dsmr/parser.h"
#include "dsmr/fields.h"

using namespace dsmr;
using namespace fields;
using namespace Catch::Matchers;

struct Printer {
  template<typename Item>
  void apply(Item &i) {
    if (i.present()) {
      std::cout << Item::get_name() << ": " << i.val() << Item::unit() << std::endl;
    }
  }
};

TEST_CASE("Should parse all fields in the DSMR message correctly")
{
  const auto &dsmr_message =
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

  ParsedData<
    /* String */ identification,
    /* String */ p1_version,
    /* String */ timestamp,
    /* String */ equipment_id,
    /* FixedValue */ energy_delivered_tariff1,
    /* FixedValue */ energy_delivered_tariff2,
    /* FixedValue */ energy_returned_tariff1,
    /* FixedValue */ energy_returned_tariff2,
    /* String */ electricity_tariff,
    /* FixedValue */ power_delivered,
    /* FixedValue */ power_returned,
    /* FixedValue */ electricity_threshold,
    /* uint8_t */ electricity_switch_position,
    /* uint32_t */ electricity_failures,
    /* uint32_t */ electricity_long_failures,
    /* String */ electricity_failure_log,
    /* uint32_t */ electricity_sags_l1,
    /* uint32_t */ electricity_sags_l2,
    /* uint32_t */ electricity_sags_l3,
    /* uint32_t */ electricity_swells_l1,
    /* uint32_t */ electricity_swells_l2,
    /* uint32_t */ electricity_swells_l3,
    /* String */ message_short,
    /* String */ message_long,
    /* FixedValue */ voltage_l1,
    /* FixedValue */ voltage_l2,
    /* FixedValue */ voltage_l3,
    /* FixedValue */ current_l1,
    /* FixedValue */ current_l2,
    /* FixedValue */ current_l3,
    /* FixedValue */ power_delivered_l1,
    /* FixedValue */ power_delivered_l2,
    /* FixedValue */ power_delivered_l3,
    /* FixedValue */ power_returned_l1,
    /* FixedValue */ power_returned_l2,
    /* FixedValue */ power_returned_l3,
    /* uint16_t */ gas_device_type,
    /* String */ gas_equipment_id,
    /* uint8_t */ gas_valve_position,
    /* TimestampedFixedValue */ gas_delivered,
    /* uint16_t */ thermal_device_type,
    /* String */ thermal_equipment_id,
    /* uint8_t */ thermal_valve_position,
    /* TimestampedFixedValue */ thermal_delivered,
    /* uint16_t */ water_device_type,
    /* String */ water_equipment_id,
    /* uint8_t */ water_valve_position,
    /* TimestampedFixedValue */ water_delivered,
    /* uint16_t */ slave_device_type,
    /* String */ slave_equipment_id,
    /* uint8_t */ slave_valve_position,
    /* TimestampedFixedValue */ slave_delivered> data;

  auto res = P1Parser::parse(&data, dsmr_message, lengthof(dsmr_message), true);
  REQUIRE(res.err == nullptr);

  // Print all values
  data.applyEach(Printer());

  // Check that all fields have correct values
  REQUIRE(data.identification == "KFM5KAIFA-METER");
  REQUIRE(data.p1_version == "40");
  REQUIRE(data.timestamp == "150117185916W");
  REQUIRE(data.equipment_id == "0000000000000000000000000000000000");
  REQUIRE(data.energy_delivered_tariff1 == 671.578f);
  REQUIRE(data.energy_delivered_tariff2 == 842.472f);
  REQUIRE(data.energy_returned_tariff1 == 0);
  REQUIRE(data.energy_returned_tariff2 == 0);
  REQUIRE(data.electricity_tariff == "0001");
  REQUIRE(data.power_delivered == 0.333f);
  REQUIRE(data.power_returned == 0);
  REQUIRE(data.electricity_threshold == 999.9f);
  REQUIRE(data.electricity_switch_position == 1);
  REQUIRE(data.electricity_failures == 8);
  REQUIRE(data.electricity_long_failures == 7);
  REQUIRE(data.electricity_failure_log == "(1)(0-0:96.7.19)(000101000001W)(2147483647*s)");
  REQUIRE(data.electricity_sags_l1 == 0);
  REQUIRE(data.electricity_swells_l1 == 0);
  REQUIRE(data.message_short.empty());
  REQUIRE(data.message_long.empty());
  REQUIRE(data.current_l1 == 1);
  REQUIRE(data.power_delivered_l1 == 0.332f);
  REQUIRE(data.power_returned_l1 == 0);
  REQUIRE(data.gas_device_type == 3);
  REQUIRE(data.gas_equipment_id == "0000000000000000000000000000000000");
  REQUIRE(data.gas_valve_position == 1);
  REQUIRE(data.gas_delivered == 473.789f);
}

TEST_CASE("Should report an error if the crc has incorrect format")
{
  // Data to parse
  const auto &dsmr_message =
    "/KFM5KAIFA-METER\r\n"
    "\r\n"
    "1-0:1.8.1(000671.578*kWh)\r\n"
    "1-0:1.7.0(00.318*kW)\r\n"
    "!1ED\r\n";

  ParsedData<
    /* String */ identification,
    /* FixedValue */ power_delivered> data;

  auto res = P1Parser::parse(&data, dsmr_message, lengthof(dsmr_message), true);
  CHECK_THAT(res.err, Equals("Incomplete or malformed checksum"));
}

TEST_CASE("Should report an error if the crc of a package is incorrect")
{
  // Data to parse
  const auto &dsmr_message =
    "/KFM5KAIFA-METER\r\n"
    "\r\n"
    "1-0:.8.1(000671.578*kWh)\r\n"
    "1-0:1.7.0(00.318*kW)\r\n"
    "!1E1D\r\n";

  ParsedData<
    /* String */ identification,
    /* FixedValue */ power_delivered> data;

  auto res = P1Parser::parse(&data, dsmr_message, lengthof(dsmr_message), true);
  CHECK_THAT(res.err, Equals("Checksum mismatch"));
}
