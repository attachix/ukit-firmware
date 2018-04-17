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

#ifndef MCU2_LIB_CORE_MISC_H_
#define MCU2_LIB_CORE_MISC_H_

#include <user_config.h>
#include <SmingCore.h>

/* Basic Logging */

#ifdef REMOTE_DEBUG
extern String logData;

void logInfo(String message);
#else
#define logInfo(A)
#endif

/* Utility functions */
char* txt2hex(const char *text, int size);

#endif /* MCU2_LIB_CORE_MISC_H_ */
