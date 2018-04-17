//
// U:Kit ESP8266 Firmware - This is the 'smart' firmware for the U:Kit sensor kit
// Copyright (C) 2016, 2018 Slavey Karadzhov <slav@attachix.com>
//
// This file is part of U:Kit ESP8266 Firmware.
//
// U:Kit ESP8266 Firmware is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// U:Kit ESP8266 Firmware is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with U:Kit ESP8266 Firmware.  If not, see <http://www.gnu.org/licenses/>.
//

#include "misc.h"

/* Basic Logging */

#ifdef REMOTE_DEBUG
String logData = "";

void logInfo(String message) {
	logData += "|" + message;
}
#else
#define logInfo(A)
#endif

/* Utility functions */
char* txt2hex(const char *text, int size) {
	char *hex = (char *)malloc(size*2)+1;
	char *ptr = hex;
	for(int i=0; i<size; i++) {
		ptr += ets_sprintf(ptr, "%02X", text[i]);
	}
	*(ptr + 1) = '\0';
	return hex;
}
