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
#include <ArduinoJson.h>  // needs version 6+

//--- if you have a real Slimme Meter connected ---
//--- activate the next two #defines --------------
//#define READSLIMMEMETER
//#define DTR_ENABLE 12   // GPIO-pin to use for DTR


// Data to parse
//---  LandisGyr E350  KMP5  DSMR50
const char msg1[] =
  "/XMX5LGBBLB2410065887\r\n"
  "\r\n"   
  "1-3:0.2.8(50)\r\n"                                   // p1_version
  "0-0:1.0.0(200408063501S)\r\n"                        // timestamp
  "0-0:96.1.1(4530303336303000000000000000000040)\r\n"  // equiptment_id
  "1-0:1.8.1(000234.191*kWh)\r\n"                       // energy_delivered_tariff1
  "1-0:1.8.2(000402.930*kWh)\r\n"                       // energy_delivered_tariff2
  "1-0:2.8.1(000119.045*kWh)\r\n"                       // energy_returned_tariff1
  "1-0:2.8.2(000079.460*kWh)\r\n"                       // energy_returned_tariff2
  "0-0:96.14.0(0001)\r\n"                               // electricity_tariff
  "1-0:1.7.0(001.22*kW)\r\n"                            // power_delivered
  "1-0:2.7.0(001.11*kW)\r\n"                            // power_returned
  "0-0:96.7.21(00010)\r\n"                              // electricity_failures
  "0-0:96.7.9(00000)\r\n"                               // electricity_long_failures
  "1-0:99.97.0(0)(0-0:96.7.19)\r\n"                     // electricity_failure_log
  "1-0:32.32.0(00002)\r\n"                              // electricity_sags_l1
  "1-0:52.32.0(00003)\r\n"                              // electricity_sags_l2
  "1-0:72.32.0(00003)\r\n"                              // electricity_sags_l3
  "1-0:32.36.0(00000)\r\n"                              // electricity_swells_l1
  "1-0:52.36.0(00000)\r\n"                              // electricity_swells_l2
  "1-0:72.36.0(00000)\r\n"                              // electricity_swells_l3
  "0-0:96.13.0()\r\n"                                   // message_long
  "1-0:32.7.0(241.0*V)\r\n"                             // voltage_l1
  "1-0:52.7.0(237.0*V)\r\n"                             // voltage_l2
  "1-0:72.7.0(235.0*V)\r\n"                             // voltage_l3
  "1-0:31.7.0(000*A)\r\n"                               // current_l1
  "1-0:51.7.0(000*A)\r\n"                               // current_l2
  "1-0:71.7.0(000*A)\r\n"                               // current_l3
  "1-0:21.7.0(00.536*kW)\r\n"                           // power_delivered_l1
  "1-0:41.7.0(00.194*kW)\r\n"                           // power_delivered_l2
  "1-0:61.7.0(00.487*kW)\r\n"                           // power_delivered_l3
  "1-0:22.7.0(00.013*kW)\r\n"                           // power_returned_l1
  "1-0:42.7.0(00.611*kW)\r\n"                           // power_returned_l2
  "1-0:62.7.0(00.486*kW)\r\n"                           // power_returned_l3
  "0-1:24.1.0(003)\r\n"                                 // mbus1_device_type
  "0-1:96.1.0(4730303339303031363532303530323136)\r\n"  // mbus1_equipment_id_tc
  "0-1:24.2.1(200408063501S)(00169.156*m3)\r\n"         // mbus1_delivered_tc
  "!0876\r\n";

//---  Sagemcom XS210 ESMR5 (1Fase)
const char msg2[] =
  "/Ene5\\XS210 ESMR 5.0\r\n"
  "\r\n"
  "1-3:0.2.8(50)\r\n"                                   // p1_version
  "0-0:1.0.0(190508094821S)\r\n"                        // timestamp
  "0-0:96.1.1(4530303437303030123456789134343137)\r\n"  // equiptment_id
  "1-0:1.8.1(000769.736*kWh)\r\n"                       // energy_delivered_tariff1
  "1-0:1.8.2(000664.646*kWh)\r\n"                       // energy_delivered_tariff2
  "1-0:2.8.1(000000.016*kWh)\r\n"                       // energy_returned_tariff1
  "1-0:2.8.2(000000.000*kWh)\r\n"                       // energy_returned_tariff2
  "0-0:96.14.0(0002)\r\n"                               // electricity_tariff
  "1-0:1.7.0(00.037*kW)\r\n"                            // power_delivered
  "1-0:2.7.0(00.000*kW)\r\n"                            // power_returned
  "0-0:96.7.21(00204)\r\n"                              // electricity_failures
  "0-0:96.7.9(00147)\r\n"               // electricity_long_failures  // electricity_failure_log
  "1-0:99.97.0(10)(0-0:96.7.19)(190508094303S)(0000055374*s)(190507165813S)(0000007"
    "991*s)(190507141021S)(0000000274*s)(190507135954S)(0000000649*s)(190507134811S)("
    "0000083213*s)(190506143928S)(0000090080*s)(190505123501S)(0000073433*s)(19050415"
    "2603S)(0000003719*s)(190504120844S)(0000337236*s)(190430142638S)(0000165493*s)\r\n"
  "1-0:32.32.0(00149)\r\n"                              // electricity_sags_l1
  "1-0:32.36.0(00000)\r\n"                              // electricity_swells_l1
  "0-0:96.13.0()\r\n"                                   // message_long
  "1-0:32.7.0(231.0*V)\r\n"                             // voltage_l1
  "1-0:31.7.0(000*A)\r\n"                               // current_l1
  "1-0:21.7.0(00.037*kW)\r\n"                           // power_delivered_l1
  "1-0:22.7.0(00.000*kW)\r\n"                           // power_returned_l1
  "0-1:24.1.0(003)\r\n"                                 // mbus1_device_type
  "0-1:96.1.0(4730303533303987654321373431393137)\r\n"  // mbus1_equipment_id_tc
  "0-1:24.2.1(632525252525S)(00000.000)\r\n"            // mbus1_delivered_tc <-- error (no unit)
  "!DE4A\r\n";

//--- Sagemcom Fluvius ? --(Belgie)
const char msg3[] =
  "/FLU5\\253769484_A\r\n"      
  "\r\n"                   
  "0-0:96.1.4(50213)\r\n"                               // p1_version_be     
  "0-0:96.1.1(3153414123456789303638373236)\r\n"        // equiptment_id
  "0-0:1.0.0(191204184601W)\r\n"                        // timestamp
  "1-0:1.8.1(000050.069*kWh)\r\n"                       // energy_delivered_tariff1
  "1-0:1.8.2(000055.085*kWh)\r\n"                       // energy_delivered_tariff2
  "1-0:2.8.1(000019.870*kWh)\r\n"                       // energy_returned_tariff1
  "1-0:2.8.2(000005.678*kWh)\r\n"                       // energy_returned_tariff2
  "0-0:96.14.0(0001)\r\n"                               // electricity_tariff
  "1-0:1.7.0(00.655*kW)\r\n"                            // power_delivered
  "1-0:2.7.0(00.000*kW)\r\n"                            // power_returned
  "1-0:32.7.0(225.1*V)\r\n"                             // voltage_l1
  "1-0:52.7.0(000.0*V)\r\n"                             // voltage_l2
  "1-0:72.7.0(225.7*V)\r\n"                             // voltage_l3
  "1-0:31.7.0(001*A)\r\n"                               // current_l1
  "1-0:51.7.0(002*A)\r\n"                               // current_l2
  "1-0:71.7.0(001*A)\r\n"                               // current_l3
  "0-0:96.3.10(1)\r\n"                                  // electricity_switch_position
  "0-0:17.0.0(999.9*kW)\r\n"                            // electricity_threshold
  "1-0:31.4.0(999*A)\r\n"                               // fuse_treshold_l1
  "0-0:96.13.0()\r\n"                                   // message_long
  "0-1:24.1.0(3)\r\n"                                   // mbus1_device_type
  "0-1:96.1.0(4730301234567031363532303511111111)\r\n"  // mbus1_equipment_id_tc
  "0-1:24.3.0(140101004100)(08)(60)(1)(0-1:24.2.1)(m3)\r\n" // mbus1_delivered_dbl
  "(00111.006)\r\n"                                         // mbus1_delivered_dbl (line 2)
  "0-3:24.1.0(3)\r\n"                                   // mbus3_device_type
  "0-3:96.1.0(4730301234567031363532303333333333)\r\n"  // mbus3_equipment_id_tc
  "0-3:24.3.0(140101004100)(08)(60)(1)(0-3:24.2.1)(m3)\r\n" // mbus3_delivered_dbl
  "(00333.006)\r\n"
  "0-2:24.1.0(3)\r\n"                                   // mbus2_device_type
  "0-2:96.1.0(4730301234567031363532303222222222)\r\n"  // mbus2_equipment_id_tc
  "0-2:24.3.0(140101004100)(08)(60)(1)(0-2:24.2.1)(m3)\r\n" // mbus2_delivered_dbl
  "(00222.006)\r\n"
  "0-4:24.1.0(3)\r\n"                                   // mbus4_device_type
  "0-4:96.1.0(47303012345670313635323034444444444)\r\n" // mbus4_equipment_id_tc
  "0-4:24.3.0(140101004100)(08)(60)(1)(0-4:24.2.1)(m3)\r\n" // mbus4_delivered_dbl
  "(00444.006)\r\n"
  "!7934\r\n";    //<-- wrong checksum! don't check it!    

/**
 * Define the data we're interested in, as well as the datastructure to
 * hold the parsed data.
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

#if defined(READSLIMMEMETER)
  #ifdef DTR_ENABLE
    P1Reader    slimmeMeter(&Serial, DTR_ENABLE);
  #else
    P1Reader    slimmeMeter(&Serial, 0);
  #endif
#endif

//===========================GLOBAL VAR'S======================================
  MyData      DSMRdata;
  DynamicJsonDocument jsonDoc(4000);  // generic doc to return, clear() before use!
  JsonObject  jsonObj;
  uint32_t    readTimer;
  char        jsonArrayName[20] = "";


struct buildJson 
{
    JsonArray root = jsonDoc.createNestedArray(jsonArrayName);
    
    template<typename Item>
    void apply(Item &i) 
    {
      String Name = String(Item::name);

      if (i.present()) 
      {
        JsonObject nested = root.createNestedObject();
        nested["name"]  = Name;
        String Unit = String(Item::unit());
        nested["value"] = value_to_json(i.val());
        
        if (Unit.length() > 0)
        {
          nested["unit"]  = Unit;
        }
      }
  }
  
  template<typename Item>
  Item& value_to_json(Item& i) 
  {
    return i;
  }

  String value_to_json(TimestampedFixedValue i) 
  {
    return String(i);
  }
  
  float value_to_json(FixedValue i) 
  {
    return i;
  }

}; // buildjson{} 


//=======================================================================
void makeJson() 
{
  String toReturn;

  jsonDoc.clear();

  strcpy(jsonArrayName, "fields");
  DSMRdata.applyEach(buildJson());
  Serial.println();

  jsonObj = jsonDoc.as<JsonObject>();

  //serializeJson(jsonObj, toReturn);         // for production
  serializeJsonPretty(jsonObj, toReturn);     // for human readable testing
  Serial.printf("JSON String is %d chars\r\n", toReturn.length());
  Serial.println(toReturn);

} // makeJson()


//---------------------------------------------------------------------------
void setup() 
{
  Serial.begin(115200);
  delay(250);
  while (!Serial) {/* wait a while */ delay(100); }
  
  Serial.println("\n\nAnd now it begins ...\n");

  Serial.println(ESP.getResetReason());
  if (   ESP.getResetReason() == "Exception" 
      || ESP.getResetReason() == "Software Watchdog"
      || ESP.getResetReason() == "Soft WDT reset"
      ) 
  {
    while (1) 
    {
      delay(500);
      digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
    }
  }


#if defined(READSLIMMEMETER)
  slimmeMeter.enable(true);
#else
  ParseResult<void> res;
  //--- read first telegram ---
  Serial.println("\r\n====================================================");
  Serial.println("Start parsing telegram 1 ");
  DSMRdata = {};
  //--------------------------------- do check CheckSum! vvvv
  res = P1Parser::parse(&DSMRdata, msg1, lengthof(msg1), true);
  if (res.err) 
  {
    // Parsing error, show it
    Serial.println(res.fullError(msg1, msg1 + lengthof(msg1)));
  } 
  else if (!DSMRdata.all_present()) 
  {
    Serial.println("DSMR: Some fields are missing");
    Serial.println("\r\nAs with this \"dsmr2Lib\" library we check for the \"mbusx_delivered_\"");
    Serial.println("\"Temperature Corrected (tc)\", \"Not Temperature Corrected (ntc)\" and ");
    Serial.println("\"double lines (dbl)\" fields."); 
    Serial.println("Normaly only one or the other is in a telegram.");
    Serial.println("So, the object \"all_present()\" will always return false!");  } 
  // Succesfully parsed, make JSON:
  makeJson();

  //--- read second telegram ---
  Serial.println("\r\n====================================================");
  Serial.println("Start parsing telegram 2");
  DSMRdata = {};
  //--------------------------------- do check CheckSum! vvvv
  res = P1Parser::parse(&DSMRdata, msg2, lengthof(msg2), true);
  if (res.err) 
  {
    // Parsing error, show it
    Serial.println(res.fullError(msg2, msg2 + lengthof(msg2)));
  } 
  else if (!DSMRdata.all_present()) 
  {
    Serial.println("DSMR: Some fields are missing");
  } 
  // Succesfully parsed, make JSON:
  makeJson();

  //--- read third telegram ---
  Serial.println("\r\n====================================================");
  Serial.println("Start parsing telegram 3 (do NOT check CheckSum)");
  DSMRdata = {};
  //----------------------------- do NOT check CheckSum! vvvvv
  res = P1Parser::parse(&DSMRdata, msg3, lengthof(msg3), false); 
  if (res.err) 
  {
    // Parsing error, show it
    Serial.println(res.fullError(msg3, msg3 + lengthof(msg3)));
  } 
  else if (!DSMRdata.all_present()) 
  {
    Serial.println("DSMR: Some fields are missing");
    Serial.println("\r\nAs with this \"dsmr2Lib\" library we check for the \"mbusx_delivered_\"");
    Serial.println("\"Temperature Corrected (tc)\", \"Not Temperature Corrected (ntc)\" and ");
    Serial.println("\"double lines (dbl)\" fields."); 
    Serial.println("Normaly only one or the other is in a telegram.");
    Serial.println("So, the object \"all_present()\" will always return false!");
  } 
  // Succesfully parsed, make JSON:
  makeJson();

  //--- read third telegram ---
  Serial.println("\r\n====================================================");
  Serial.println("Start parsing telegram 3 (do check CheckSum)");
  DSMRdata = {};
  //--------------------------------- do check CheckSum! vvvv
  res = P1Parser::parse(&DSMRdata, msg3, lengthof(msg3), true); 
  if (res.err) 
  {
    // Parsing error, show it
    Serial.println(res.fullError(msg3, msg3 + lengthof(msg3)));
  } 
  else if (!DSMRdata.all_present()) 
  {
    Serial.println("DSMR: Some fields are missing");
    Serial.println("\r\nAs with this \"dsmr2Lib\" library we check for the \"mbusx_delivered_\"");
    Serial.println("\"Temperature Corrected (tc)\", \"Not Temperature Corrected (ntc)\" and ");
    Serial.println("\"double lines (dbl)\" fields."); 
    Serial.println("Normaly only one or the other is in a telegram.");
    Serial.println("So, the object \"all_present()\" will always return false!");
  } 
  // Succesfully parsed, make JSON:
  makeJson();

#endif

} // setup()


//---------------------------------------------------------------------------
void loop () {
#if defined(READSLIMMEMETER)
  slimmeMeter.loop();
  slimmeMeter.enable(true);
  if (millis() - readTimer > 10000)
  {
    readTimer = millis();
    if (slimmeMeter.available()) 
    {
      DSMRdata = {};
      String DSMRerror;
      
      if (slimmeMeter.parse(&DSMRdata, &DSMRerror))   // Parse succesful, print result
      {
        buildJson();
      }
    }
  }
#endif
} // loop()
