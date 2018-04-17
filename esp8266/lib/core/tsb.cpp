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

#include "tsb.h"

#include <math.h>

#include "misc.h"

Tsb::Tsb(HardwareSerial *stream) {
	this->stream = stream;
}

bool Tsb::connect(String password /* = "" */) {
	stream->write("@@@");
	if(password.length()) {
		stream->write(password.c_str());
	}
	stream->flush();

	return readDeviceInfo();
}

bool Tsb::setPassword(String password) {
	uint8_t *userData = readUserData();
	if(!userData) {
		debugf("TSB: Unable to read user data.");
		return false;
	}

	int passwordLength = password.length();
	for(int i=0; i< deviceInfo->page_size - 3; i++) {
		if(i < passwordLength) {
			userData[3+i] = password.charAt(i);
		}
		else {
			userData[3+i] = 0xFF;
		}
	}

	bool res = writeUserData((uint8_t *)userData);

	delete[] userData;

	return res;
}


bool Tsb::setTimeout(int seconds) {
	uint8_t *userData = readUserData();
	if (!userData) {
		debugf("TSB: Unable to read user data.");
		return false;
	}

	userData[2] = (TSB_CPU_FREQUENCY * (int)pow(10, 6) * seconds)/ 196600;

	bool res = writeUserData((uint8_t *)userData);

	delete[] userData;

	return res;
}

size_t Tsb::writeFlash(uint8_t *bytes, int length) {
	if (!deviceInfo) {
		// should connect first!
		return false;
	}

	char buffer[1] = {0};
	int pages = (length / deviceInfo->page_size) + 1;
	int offset = 0;
	size_t bytesWritten = 0;

	stream->write(TSB_CMD_WRITE_FLASH);
	stream->flush();

	logInfo("Write To Flash");

#ifdef REMOTE_DEBUG
	char value[25] = {0};
#endif

	for(int i=0; i< pages; i++) {
		// The first response is taking around 580 ms
		// Next requests are taking around 158 ms
		if(!stream->readBytes(buffer, 1) || buffer[0] != TSB_REQUEST) {
#ifdef REMOTE_DEBUG
			ets_sprintf(value, "E: i=%d, b=%x, Av: %d", i, buffer[0], stream->available());
			logInfo(String(value));
#endif
			return 0;
		}

		offset = i * deviceInfo->page_size;

		system_soft_wdt_feed();

		stream->write(TSB_CONFIRM);
		if(offset + deviceInfo->page_size > length) {
			bytesWritten += stream->write(bytes+offset, length-offset);
			int remainingLength = (deviceInfo->page_size - (length % deviceInfo->page_size));
			char padding[remainingLength];
			for(int j=0; j<remainingLength; j++) {
				padding[j] = 0xFF;
			}
			stream->write(padding, remainingLength);
			stream->flush();

			if(!stream->readBytes(buffer, 1) || buffer[0] != TSB_REQUEST) {
#ifdef REMOTE_DEBUG
				ets_sprintf(value, "FP Error: i=%d, buffer[0]=%x", i, buffer[0]);
				logInfo(String(value));
#endif
				return 0; // bytesWritten
			}
		}
		else {
			bytesWritten += stream->write(bytes+offset, (int)deviceInfo->page_size);
			stream->flush();
		}
	}

	stream->write(TSB_REQUEST);

	if(!stream->readBytes(buffer, 1) || buffer[0] != TSB_CONFIRM) {
#ifdef REMOTE_DEBUG
		ets_sprintf(value, "End error: buffer[0]=%x", buffer[0]);
		logInfo(String(value));
#endif
		return 0;
	}

	return bytesWritten;
}

bool Tsb::run() {
	stream->write(TSB_CMD_RUN);
	stream->flush();
	return true;
}

uint8_t* Tsb::readUserData() {
	if(!deviceInfo) {
		// should connect first!
		return NULL;
	}

#ifdef REMOTE_DEBUG
	logInfo("Getting User Data");
	char value[250] = {0};
#endif

	char *userData = new char[deviceInfo->page_size+1];
	if(!userData) {
		return NULL;
	}

	stream->write(TSB_CMD_READ_USERDATA);
	stream->flush();

	if(!stream->readBytes(userData, deviceInfo->page_size+1)) {
#ifdef REMOTE_DEBUG
		ets_sprintf(value, "Available: %d, Requested: %d", stream->available(), deviceInfo->page_size);
		Serial.write(value);

		logInfo("Error: Unable to read user data?!");
#endif
		delete[] userData;

		return NULL;
	}

#ifdef REMOTE_DEBUG
	ets_sprintf(value, "User data: %s", txt2hex(userData, deviceInfo->page_size));
	logInfo(String(value));
#endif

	return (uint8_t *)userData;
}

bool Tsb::writeUserData(uint8_t *userData) {
	stream->write(TSB_CMD_WRITE_USERDATA);
	stream->flush();

	char buffer[1]= {0};
	if (!stream->readBytes(buffer, 1) ||
		buffer[0] != TSB_REQUEST) {
		return false;
	}

	stream->write(TSB_CONFIRM);
	stream->write(userData, deviceInfo->page_size);
	stream->flush();

	if(!readDeviceInfo()) {
		return false;
	}

	if (!stream->readBytes(buffer, 1) ||
		buffer[0] != TSB_CONFIRM) {
		return false;
	}

	return true;
}

bool Tsb::readDeviceInfo() {
	int infoSize = 14;
	int dataSize = 17; // INFOLEN  = 8 ; *Words* of Device Info => 2*8+1 (TSB_CONFIRM) = 17
	// read the device data
	char* data = new char[dataSize];

	if(!stream->readBytes(data, dataSize)) {
		logInfo("No TSB DeviceInfo sent.");
		delete[] data;
		return false;
	}

#ifdef REMOTE_DEBUG
	char value[250] = {0};
#endif

	if (!(data[0] == 't' && data[1] == 's' && data[2] == 'b')) {
#ifdef REMOTE_DEBUG
		ets_sprintf(value, "DeviceInfo Error: buffer=%s", txt2hex(data, dataSize));
		logInfo(String(value));
#endif
		delete[] data;
		return false;
	}

	logInfo("Device Info");

	deviceInfo = (TsbDeviceInfo *) malloc(infoSize);
	memcpy((void *) deviceInfo, (void *) data, infoSize);

	deviceInfo->mem_size *= 2; //
	deviceInfo->eeprom_size += 1; // ord(header[12]) + ord(header[13])*256+1
	deviceInfo->page_size *= 2; // size in "words" = 2 * bytes

	delete[] data;

	return true;
}
