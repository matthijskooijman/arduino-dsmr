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

#include "fields2.h"


using namespace dsmr;
using namespace dsmr::fields;

// Since C++11 it is possible to define the initial values for static
// const members in the class declaration, but if their address is
// taken, they still need a normal definition somewhere (to allocate
// storage).
constexpr char units::none[];
constexpr char units::kWh[];
constexpr char units::Wh[];
constexpr char units::kW[];
constexpr char units::W[];
constexpr char units::V[];
constexpr char units::mV[];
constexpr char units::A[];
constexpr char units::mA[];
constexpr char units::m3[];
constexpr char units::dm3[];
constexpr char units::GJ[];
constexpr char units::MJ[];

constexpr ObisId identification::id;
constexpr char identification::name_progmem[];
constexpr const __FlashStringHelper *identification::name;

constexpr ObisId p1_version::id;
constexpr char p1_version::name_progmem[];
constexpr const __FlashStringHelper *p1_version::name;

constexpr ObisId p1_versionBE::id;
constexpr char p1_versionBE::name_progmem[];
constexpr const __FlashStringHelper *p1_versionBE::name;

constexpr ObisId timestamp::id;
constexpr char timestamp::name_progmem[];
constexpr const __FlashStringHelper *timestamp::name;

constexpr ObisId equipment_id::id;
constexpr char equipment_id::name_progmem[];
constexpr const __FlashStringHelper *equipment_id::name;

constexpr ObisId energy_delivered_tariff1::id;
constexpr char energy_delivered_tariff1::name_progmem[];
constexpr const __FlashStringHelper *energy_delivered_tariff1::name;

constexpr ObisId energy_delivered_tariff2::id;
constexpr char energy_delivered_tariff2::name_progmem[];
constexpr const __FlashStringHelper *energy_delivered_tariff2::name;

constexpr ObisId energy_returned_tariff1::id;
constexpr char energy_returned_tariff1::name_progmem[];
constexpr const __FlashStringHelper *energy_returned_tariff1::name;

constexpr ObisId energy_returned_tariff2::id;
constexpr char energy_returned_tariff2::name_progmem[];
constexpr const __FlashStringHelper *energy_returned_tariff2::name;

constexpr ObisId electricity_tariff::id;
constexpr char electricity_tariff::name_progmem[];
constexpr const __FlashStringHelper *electricity_tariff::name;

constexpr ObisId power_delivered::id;
constexpr char power_delivered::name_progmem[];
constexpr const __FlashStringHelper *power_delivered::name;

constexpr ObisId power_returned::id;
constexpr char power_returned::name_progmem[];
constexpr const __FlashStringHelper *power_returned::name;

constexpr ObisId electricity_threshold::id;
constexpr char electricity_threshold::name_progmem[];
constexpr const __FlashStringHelper *electricity_threshold::name;

constexpr ObisId electricity_switch_position::id;
constexpr char electricity_switch_position::name_progmem[];
constexpr const __FlashStringHelper *electricity_switch_position::name;

constexpr ObisId electricity_failures::id;
constexpr char electricity_failures::name_progmem[];
constexpr const __FlashStringHelper *electricity_failures::name;

constexpr ObisId electricity_long_failures::id;
constexpr char electricity_long_failures::name_progmem[];
constexpr const __FlashStringHelper *electricity_long_failures::name;

constexpr ObisId electricity_failure_log::id;
constexpr char electricity_failure_log::name_progmem[];
constexpr const __FlashStringHelper *electricity_failure_log::name;

constexpr ObisId electricity_sags_l1::id;
constexpr char electricity_sags_l1::name_progmem[];
constexpr const __FlashStringHelper *electricity_sags_l1::name;

constexpr ObisId electricity_sags_l2::id;
constexpr char electricity_sags_l2::name_progmem[];
constexpr const __FlashStringHelper *electricity_sags_l2::name;

constexpr ObisId electricity_sags_l3::id;
constexpr char electricity_sags_l3::name_progmem[];
constexpr const __FlashStringHelper *electricity_sags_l3::name;

constexpr ObisId electricity_swells_l1::id;
constexpr char electricity_swells_l1::name_progmem[];
constexpr const __FlashStringHelper *electricity_swells_l1::name;

constexpr ObisId electricity_swells_l2::id;
constexpr char electricity_swells_l2::name_progmem[];
constexpr const __FlashStringHelper *electricity_swells_l2::name;

constexpr ObisId electricity_swells_l3::id;
constexpr char electricity_swells_l3::name_progmem[];
constexpr const __FlashStringHelper *electricity_swells_l3::name;

constexpr ObisId message_short::id;
constexpr char message_short::name_progmem[];
constexpr const __FlashStringHelper *message_short::name;

constexpr ObisId message_long::id;
constexpr char message_long::name_progmem[];
constexpr const __FlashStringHelper *message_long::name;

constexpr ObisId voltage_l1::id;
constexpr char voltage_l1::name_progmem[];
constexpr const __FlashStringHelper *voltage_l1::name;

constexpr ObisId voltage_l2::id;
constexpr char voltage_l2::name_progmem[];
constexpr const __FlashStringHelper *voltage_l2::name;

constexpr ObisId voltage_l3::id;
constexpr char voltage_l3::name_progmem[];
constexpr const __FlashStringHelper *voltage_l3::name;

constexpr ObisId current_l1::id;
constexpr char current_l1::name_progmem[];
constexpr const __FlashStringHelper *current_l1::name;

constexpr ObisId current_l2::id;
constexpr char current_l2::name_progmem[];
constexpr const __FlashStringHelper *current_l2::name;

constexpr ObisId current_l3::id;
constexpr char current_l3::name_progmem[];
constexpr const __FlashStringHelper *current_l3::name;

constexpr ObisId power_delivered_l1::id;
constexpr char power_delivered_l1::name_progmem[];
constexpr const __FlashStringHelper *power_delivered_l1::name;

constexpr ObisId power_delivered_l2::id;
constexpr char power_delivered_l2::name_progmem[];
constexpr const __FlashStringHelper *power_delivered_l2::name;

constexpr ObisId power_delivered_l3::id;
constexpr char power_delivered_l3::name_progmem[];
constexpr const __FlashStringHelper *power_delivered_l3::name;

constexpr ObisId power_returned_l1::id;
constexpr char power_returned_l1::name_progmem[];
constexpr const __FlashStringHelper *power_returned_l1::name;

constexpr ObisId power_returned_l2::id;
constexpr char power_returned_l2::name_progmem[];
constexpr const __FlashStringHelper *power_returned_l2::name;

constexpr ObisId power_returned_l3::id;
constexpr char power_returned_l3::name_progmem[];
constexpr const __FlashStringHelper *power_returned_l3::name;

constexpr ObisId mbus1_device_type::id;
constexpr char mbus1_device_type::name_progmem[];
constexpr const __FlashStringHelper *mbus1_device_type::name;

constexpr ObisId mbus1_equipmentTC_id::id;
constexpr char mbus1_equipmentTC_id::name_progmem[];
constexpr const __FlashStringHelper *mbus1_equipmentTC_id::name;

constexpr ObisId mbus1_equipmentNTC_id::id;
constexpr char mbus1_equipmentNTC_id::name_progmem[];
constexpr const __FlashStringHelper *mbus1_equipmentNTC_id::name;

constexpr ObisId mbus1_valve_position::id;
constexpr char mbus1_valve_position::name_progmem[];
constexpr const __FlashStringHelper *mbus1_valve_position::name;

constexpr ObisId mbus1_deliveredTC::id;
constexpr char mbus1_deliveredTC::name_progmem[];
constexpr const __FlashStringHelper *mbus1_deliveredTC::name;

constexpr ObisId mbus1_deliveredNTC::id;
constexpr char mbus1_deliveredNTC::name_progmem[];
constexpr const __FlashStringHelper *mbus1_deliveredNTC::name;

constexpr ObisId mbus2_device_type::id;
constexpr char mbus2_device_type::name_progmem[];
constexpr const __FlashStringHelper *mbus2_device_type::name;

constexpr ObisId mbus2_equipment_id::id;
constexpr char mbus2_equipment_id::name_progmem[];
constexpr const __FlashStringHelper *mbus2_equipment_id::name;

constexpr ObisId mbus2NTC_equipment_id::id;
constexpr char mbus2NTC_equipment_id::name_progmem[];
constexpr const __FlashStringHelper *mbus2NTC_equipment_id::name;

constexpr ObisId mbus2_valve_position::id;
constexpr char mbus2_valve_position::name_progmem[];
constexpr const __FlashStringHelper *mbus2_valve_position::name;

constexpr ObisId mbus2_deliveredTC::id;
constexpr char mbus2_deliveredTC::name_progmem[];
constexpr const __FlashStringHelper *mbus2_deliveredTC::name;

constexpr ObisId mbus2_deliveredTCNTC::id;
constexpr char mbus2_deliveredTCNTC::name_progmem[];
constexpr const __FlashStringHelper *mbus2_deliveredTCNTC::name;

constexpr ObisId mbus3_device_type::id;
constexpr char mbus3_device_type::name_progmem[];
constexpr const __FlashStringHelper *mbus3_device_type::name;

constexpr ObisId mbus3_equipment_id::id;
constexpr char mbus3_equipment_id::name_progmem[];
constexpr const __FlashStringHelper *mbus3_equipment_id::name;

constexpr ObisId mbus3NTC_equipment_id::id;
constexpr char mbus3NTC_equipment_id::name_progmem[];
constexpr const __FlashStringHelper *mbus3NTC_equipment_id::name;

constexpr ObisId mbus3_valve_position::id;
constexpr char mbus3_valve_position::name_progmem[];
constexpr const __FlashStringHelper *mbus3_valve_position::name;

constexpr ObisId mbus3_deliveredTC::id;
constexpr char mbus3_deliveredTC::name_progmem[];
constexpr const __FlashStringHelper *mbus3_deliveredTC::name;

constexpr ObisId mbus3_deliveredTCNTC::id;
constexpr char mbus3_deliveredTCNTC::name_progmem[];
constexpr const __FlashStringHelper *mbus3_deliveredTCNTC::name;

constexpr ObisId mbus4_device_type::id;
constexpr char mbus4_device_type::name_progmem[];
constexpr const __FlashStringHelper *mbus4_device_type::name;

constexpr ObisId mbus4_equipment_id::id;
constexpr char mbus4_equipment_id::name_progmem[];
constexpr const __FlashStringHelper *mbus4_equipment_id::name;

constexpr ObisId mbus4NTC_equipment_id::id;
constexpr char mbus4NTC_equipment_id::name_progmem[];
constexpr const __FlashStringHelper *mbus4NTC_equipment_id::name;

constexpr ObisId mbus4_valve_position::id;
constexpr char mbus4_valve_position::name_progmem[];
constexpr const __FlashStringHelper *mbus4_valve_position::name;

constexpr ObisId mbus4_deliveredTC::id;
constexpr char mbus4_deliveredTC::name_progmem[];
constexpr const __FlashStringHelper *mbus4_deliveredTC::name;

constexpr ObisId mbus4_deliveredTCNTC::id;
constexpr char mbus4_deliveredTCNTC::name_progmem[];
constexpr const __FlashStringHelper *mbus4_deliveredTCNTC::name;

