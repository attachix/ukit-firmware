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

#ifndef MCU2_LIB_CORE_CRASH_H_
#define MCU2_LIB_CORE_CRASH_H_

#include <user_config.h>
#include <SmingCore.h>
#include <rboot.h>

#define CRASH_DATA_MAGIC (0x48535243u)

#ifndef CRASH_MAX_STACK_FRAME_LINES
#define CRASH_MAX_STACK_FRAME_LINES 10
#endif /* CRASH_MAX_STACK_FRAME_LINES */

typedef struct {
	uint8_t count = 0; // the number of crashes per rom.
	rst_info reason; // reasons for the last crash
	uint32_t ms = 0; // milliseconds from start until the crash
	uint32_t sf[CRASH_MAX_STACK_FRAME_LINES * 5] = {0};
} CrashType;

typedef struct {
	uint32_t  magic=CRASH_DATA_MAGIC;
	CrashType rom[MAX_ROMS];
} CrashDataType;


#ifdef __cplusplus
extern "C" {
#endif

bool loadCrashData(CrashDataType* data);
bool saveCrashData(CrashDataType* data);

/**
 * Custom Crash Handler
 */
void custom_crash_callback( struct rst_info * rst_info, uint32_t stack, uint32_t stack_end );

#ifdef __cplusplus
}
#endif

#endif /* MCU2_LIB_CORE_CRASH_H_ */
