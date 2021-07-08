/**
 * Arduino DSMR parser.
 *
 * This software is licensed under the MIT License.
 *
 * Copyright (c) 2015 Matthijs Kooijman <matthijs@stdin.nl>
 * 
 * Changed by Willem Aandewiel to incorporate Non Temperature Compensated
 * GAS meter readings. GAS meter is no longer hardcoded on M-BUS 1 and
 * the obis for the Belgium P1_Version (p1_versionBE) is available.
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
 * Main included file. If you include this, you'll get everything,
 * imported into global scope
 */

#ifndef DSMR_INCLUDE_DSMR_H
#define DSMR_INCLUDE_DSMR_H

#include "dsmr2/parser2.h"
#include "dsmr2/reader2.h"
#include "dsmr2/fields2.h"

// Allow using everything without the namespace prefixes
using namespace dsmr;
using namespace dsmr::fields;

#endif // DSMR_INCLUDE_DSMR_H
