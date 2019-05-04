Arduino Dutch Smart meter (DSMR) parser
=======================================
This is an Arduino library for interfacing with Dutch smart meters, through
their P1 port. This library can take care of controlling the "request" pin,
reading messages and parsing them.

This code was written for Arduino, but most of the parsing code it is pretty
generic C++ (except for the Arduino- and AVR-based string handling), so it
should be possible to adapt for use outside of the Arduino environment.

When using Arduino, version 1.6.6 or above is required because this
library needs C++11 support which was enabled in that version.

Protocol
--------
Every smart meter in the Netherlands has to comply with the Dutch Smart
Meter Requirements (DSMR). At the time of writing, DSMR 4.x is the
current version. The DSMR 5.0 P1 specification is available and expected to
be used in smart meters starting in 2016. This code should support both
the 4.x and 5.0 specifications. 3.x meters might also work, but this has
not been verified or tested (feedback welcome).

The DSMR specifications can be found on [the site of Netbeheer
Nederland][netbeheer], in particular on [this
page][dossier-slimme-meter]. Of particular interest is the "P1 companion
standard" that specifies the P1 port and protocol (though not very
clearly). Specifications can also be found in the `specs` subdirectory
of this repository (including some older versions that are no longer
online, which is why I started collecting them here).

[netbeheer]: http://www.netbeheernederland.nl
[dossier-slimme-meter]: https://www.netbeheernederland.nl/dossiers/slimme-meter-15/documenten

According to DSMR, every smart electricity meter needs to have a P1
port. This is a [6p6c socket][6p6c] (commonly, but incorrectly referred
to as RJ11 or RJ12). Telephone plugs will fit, provided that you have
some that actually have 6 pins wired, or you have just 4 and do not need
power from the P1 port.

[6p6c]: http://en.wikipedia.org/wiki/Modular_connector#6P6C

Pinouts and electrical specs can best be looked up in the spec (The 5.0
version is the most clear in this regard, though not everything may
apply to 4.x meters).

Note that the message format for the P1 port is based on the IEC 62056-21
"mode D" format. That spec is not available for free, though there seem
to be [a version available on the net][iec62056-21]. DLMS also seems a
related standard, but that apparently defines a binary format. It does
seem all of these use "OBIS identifiers" and "COSEM data objects"
(DLMS has [some lists of objects][objlists], of which 1001-7 seems to
somewhat match th DSMR specs), to describe the various properties,
though it's not entirely clear how all of these fit together. However,
the DSMR spec has a complete, though sometimes confusing list of fields
used.

[iec62056-21]: https://www.ungelesen.net/protagWork/media/downloads/solar-steuerung/iec62056-21%7Bed1.0%7Den_.pdf
[objlists]: http://www.dlms.com/documentation/listofstandardobiscodesandmaintenanceproces/index.html

A typical P1 message looks something like this:

	/KFM5KAIFA-METER

	1-0:1.8.1(000671.578*kWh)
	1-0:1.7.0(00.318*kW)
	!1E1D

This includes an identification header at the top, a checksum at the
bottom, and one or more lines of data in the middle. This example is
really stripped down, real messages will have more data in them.

The first part of the line (e.g. `1-0:1.8.1`) is the (OBIS) id of the
field, which defines the meaning and format of the rest of the line.

Parsing a message
-----------------
Unlike other solutions floating around (which typically do some pattern
matching to extract the data they need), this code properly parses
messages, verifying the checksum and really parses each line according
to the specifications. This should make for more reliable parsing, and
allows for useful parser error messages:

	1-0:1.8.1(000671.578*XWh)
	                     ^
	Error: Invalid unit

	1-0:1.8.1(0006#71.578*kWh)
	              ^
	Error: Invalid number

	!6F4A
	 ^
	Checksum mismatch

This library uses C++ templates extensively. This allows defining a
custom datatype by listing the fields you are interested in, and then
all necessary parsing will happen automatically. The code generated
parses each line in the message in turn and for each line loops over the
fields in the datatype to find one whose ID matches. If found, the value
is parsed and stored into the corresponding field.

As an example, consider we want to parse the identification and current
power fields in the example message above. We define a datatype:

	using MyData = ParsedData<
	  /* String */ identification,
	  /* FixedValue */ power_delivered
	>;

The syntax is a bit weird because of the template magic used, but the
above essentially defines a struct with members for each field to be
parsed. For each field, there is also an associated `xxx_present`
member, which can be used to check whether the field was present in the
parsed data (if it is false, the associated field contains uninitialized
data).  There is some extra stuff in the background, but the `MyData`
can be used just like the below struct. It also takes up the same amount
of space.

	struct MyData {
		bool identification_present;
		String identification;
		bool power_delivered_present;
		FixedValue power_delivered;
	};

After this, call the parser. By passing our custom datatype defined
above, the parser knows what fields to look for.

	  MyData data;
	  ParseResult<void> res = P1Parser::parse(&data, msg, lengthof(msg));

Finally, we can check if the parsing was succesful and access the parsed
values as members of `data`:

	  if (!res.err && res.all_present()) {
	    // Succesfully parsed, print results:
	    Serial.println(data.identification);
	    Serial.print(data.power_delivered.int_val());
	    Serial.println("W");
	  }

In this case, we check whether parsing was successful, but also check
that all defined fields were present in the parsed message (using the
`all_present()` method), to prevent printing undefined values. If you
want to support optional fields, you can use the `xxx_present` members
for each field individually instead.

Additionally, this template approach allows looping over all available
fields in a generic way, for example to print the parse results with
just a few lines of code. See the parse and read examples for how this
works.

Note that these examples contain the full list of supported fields,
which causes parsing and printing code to be generated for all those
fields, even if they are not present in the output you want to parse. It
is recommended to limit the list of fields to just the ones that you
need, to make the parsing and printing code smaller and faster.

Parsed value types
------------------
Some values are parsed to an Arduino `String` value or C++ integer type,
those should be fairly straightforward. There are three special types
that need some explanation: `FixedValue` and `TimestampedFixedValue`.

When looking at the DSMR P1 format, it defines a floating point format.
It is described as `Fn(x,y)`, where `n` is the total number of (decimal)
digits, of which at least `x` and at most `y` are behind the decimal
separator (e.g. fractional digits).

However, this floating point format is a lot more limited than the C
`float` format. For one, it is decimal-based, not binary. Furthermore,
the decimal separator doesn't float very far, the biggest value for `y`
used is 3. Even more, it seems that for any given field, there is no
actual floating involved, fields have `x` equal to `y`, so the number of
fractional digits is fixed.

Because of this, parsing into a `float` value isn't really useful (and
on the Arduino, which doesn't have an FPU, very inefficient too). For
this reason, we use the `FixedValue` type, which stores the value as an
integer, in thousands of the original unit. This means that a value of
1.234kWh is stored as 1234 (effectively the value has been translated to
Wh).

If you access the field directly, it will automatically be converted to
`float`, keeping the original value. Alternatively, if an integer
version is sufficient, you can call the `int_val()` method to get the
integer version returned.

	// Print as float, in kW
	Serial.print(data.power_delivered);
	// Print as integer, in W
	Serial.print(data.power_delivered.int_val());

Additionally there is a `TimestampedFixedValue` method, which works
identically, but additionally has a `timestamp()` method which returns
the timestamp sent along with the value.

These timestamps are returned as a string, exactly as present in the P1
message (YYMMDDhhmmssX, where X is S or W for summer- or wintertime).
Parsing these into something like a UNIX timestamp is tricky (think
leap years and seconds) and of limited use, so this just keeps the
original format.

Connecting the P1 port
----------------------
The P1 port essentially consists of three parts:
 - A 5V power supply (this was not present in 3.x).
 - A serial TX pin. This sends meter data using 0/5V signalling, using
   idle low. Note that this is the voltage level commonly referred to as
   "TTL serial", but the polarity is reversed (more like RS232). This
   port uses 115200 bps 8N1 (3.x and before used 9600 bps).
 - A request pin - 5V needs to be applied to this pin to start
   generating output on the TX pin.

To connect to an Arduino that has an unused hardware serial port (like
an Arduino Mega, Leonardo or Micro), the signal has to inverted. This
can be done using a dedicated inverter IC, or just a transistor and some
resistors.

It's also possible to do all of the serial reception, including the
inverting, in software using Arduino's SoftwareSerial library. However,
it seems there are still occasional reception errors when using
SoftwareSerial.

Slave meters
------------
In addition to a smart electricity meter, there can be additional
slave meters attached (e.g., gas, water, thermal and slave electricity
meter). These can talk to the main meter using the (wired or wireless)
MBUS protocol to regularly (hourly for 4.x, every 5 minutes for 5.0)
send over their meter readings. Based on the configuration / connection,
each of these slaves gets an MBUS identifier (1-4).

In the P1 message, this identifier is used as the second number in the
OBIS identifiers for the fields for the slave. Currently, the code has
the assignment from MBUS identifier to device type hardcoded in
`fields.h`. For the most common configuration of an electricity meter with
a single gas meter as slave, this works straight away. Other
configurations might need changes to `fields.h` to work.

License
-------
All of the code and documentation in this library is licensed under the
MIT license, with the exception of the examples, which are licensed
under an even more liberal license.
