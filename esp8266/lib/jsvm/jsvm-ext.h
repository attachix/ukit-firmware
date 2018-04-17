/* * U:Kit ESP8266 Firmware - This is the 'smart' firmware for the U:Kit sensor kit
 * Copyright (C) 2016, 2018 Slavey Karadzhov <slav@attachix.com>
 *
 * This file is part of U:Kit ESP8266 Firmware.
 *
 * U:Kit ESP8266 Firmware is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * U:Kit ESP8266 Firmware is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with U:Kit ESP8266 Firmware.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef MCU2_INCLUDE_JSVM_EXT_H_
#define MCU2_INCLUDE_JSVM_EXT_H_

#include "jerry-core/include/jerryscript.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * External functions that are available to JerryScript
 */

/**
 * brief Function that prints data in the debug console
 */
jerry_value_t alertFunction (const jerry_value_t function_obj,
							 const jerry_value_t this_val,
							 const jerry_value_t args_p[],
							 const jerry_length_t args_cnt);

/**
 * brief Function that prints data in the standard console
 */
jerry_value_t printFunction (const jerry_value_t function_obj,
							 const jerry_value_t this_val,
							 const jerry_value_t args_p[],
							 const jerry_length_t args_cnt);

#ifdef __cplusplus
}
#endif

#endif /* MCU2_INCLUDE_JSVM_EXT_H_ */
