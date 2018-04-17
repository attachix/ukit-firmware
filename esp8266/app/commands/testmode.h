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

#ifndef __APP_COMMANDS_TESTMODE_H__
#define __APP_COMMANDS_TESTMODE_H__

#include "crash.h"

#ifdef TEST_MODE
/**
 * Tests the CLI processor. Type "test" to invoke it
 */
void cmdTest(char command, char* params);

/**
 * Shows available free heap
 */
void cmdFreeHeap(char command, char* params);

/**
 * Performs software reset
 */
void cmdSoftReset(char command, char* params);

/**
 * Toggles the WIFI flag to on or off.
 *
 * @param char* params If param[0]== '0' - wifi is OFF, else wifi is ON
 *
 */
void cmdSaveData(char command, char* params);

/**
 * Load and Show the current configuration data
 */
void cmdShowData(char command, char* params);

/**
 * Simulates detected motion
 */
void cmdFakeMotion(char command, char* params);

/**
 * Performs ROM switching
 *
 * @param char* params If param[0]== '0' - perform temporary switch, else perform permanent switch
 */
void cmdRomSwitch(char command, char* params);

/**
 * Crashes the device and causes it to produce exception
 */
void cmdCrash(char command, char* params);

/* Implementation */

void cmdTest(char command, char* params) {
	int argc = sizeof(params);
	debugf("Got command: %c with %d params", command, argc);
	if(argc > 0) {
		debugf("Last parameter was: 0x%x", params[argc-1]);
	}
}

void cmdFreeHeap(char command, char* params) {
	debugf("Free heap: %d", system_get_free_heap_size());
}

void cmdSoftReset(char command, char* params) {
	System.restart();
}

void cmdSaveData(char command, char* params) {
	debugf("Saving: ...");
	if(params[0] == 0x30) {
		clearBits(appData->data.flags, DATA_FLAG_WIFI);
	}
	else if (params[0] == 0x31) {
		setBits(appData->data.flags, DATA_FLAG_WIFI);
	}
	appData->save();
	debugf("Saving: Done.");
}

void cmdShowData(char command, char* params) {
	debugf("================================");
	debugf("Mqtt Host: %s", appData->data.mqttHost);
	debugf("Mqtt Port: %d", appData->data.mqttPort);
	debugf("ownerId: %s", appData->data.owner);
	debugf("Flags: %x", appData->data.flags);
	debugf("================================");
}

void cmdFakeMotion(char command, char* params) {
	char p[] = {'a','b'};
	cmdEvent(PROTOCOL_COMMAND_MOTION, p);
}

void cmdRomSwitch(char command, char* params) {
	uint8_t slot = 0;
	if(params[0] == '0') { // zero was presses
		rboot_get_last_boot_rom(&slot);
		rboot_set_temp_rom(slot ? 0: 1);
	}
	if(params[0] == '2') { // switch to default rom temporary
		rboot_set_temp_rom(2);
	}
	else {
		// we do permanent switch to the other rom
		slot = rboot_get_current_rom();
		rboot_set_current_rom(slot ? 0: 1);
	}
	System.restart();
}

void cmdCrash(char command, char* params)
{
	if(params[0] == '1') { // crash the device
		Vector<int> *v;

		if(v->contains(1)) { // << the crash should happen here...
			debugf("?!"); // << that code should never be called!
		}
	}

	// load the crash
	CrashDataType data;
	loadCrashData(&data);
	uint8_t slot = 0;
	rboot_rtc_data rtc;
	rboot_get_rtc_data(&rtc);
	slot = rtc.last_rom;

	debugf("======================== Crash Stats ========================");
	debugf("Crashes for the current ROM(%d): %d", slot, data.rom[slot].count);
	debugf("... after %d ms", data.rom[slot].ms);
	if(data.rom[slot].count > 0) {
		debugf("Reason for last exception: %d", data.rom[slot].reason.reason);
		uint32_t* values = (uint32_t*)&(data.rom[slot].sf);
		int offset = 0;
		for (int i = 0; i < CRASH_MAX_STACK_FRAME_LINES ; i++) {
			debugf("%08x:  %08x %08x %08x %08x", values[offset], values[offset + 1], values[offset + 2], values[offset + 3], values[offset + 4]);
			offset += 5;
		}
	}
	debugf("==============================================================");
}

#endif /* TEST_MODE */

#endif /* __APP_COMMANDS_TESTMODE_H__ */

