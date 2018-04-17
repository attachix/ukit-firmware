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

#include "scproto.h"

int32_t ScProto::hashCode(uint8_t *data, int length)
{
	int32_t hash = 0;
    if (length == 0) {
    	return hash;
    }

    for (int i = 0; i < length; i++) {
      hash = ((hash << 5)-hash) + data[i];
    }
    return hash;
}

bool ScProto::compareChecksum(int32_t n, uint8_t *bytes) {
	return  (bytes[0] == ((n >> 24) & 0xFF)) &&
			(bytes[1] == ((n >> 16) & 0xFF)) &&
			(bytes[2] == ((n >> 8) & 0xFF)) &&
			(bytes[3] == (n & 0xFF));
}

void ScProto::parse(ScCommands& commands, const char *in, uint8_t *key /*= NULL */, int keylen /*=0 */)
{
	int inLength = strlen(in);
	uint8_t *out = (uint8_t *)malloc(sizeof(uint8_t) * inLength);
	int outlen = inLength;

	/**
	 * Data:
	 * Byte 0: Encoding
	 * Bytes 1-4: Checksum
	 * Bytes 5-: Commands
	 *
	 * Commands:
	 * Byte 1: Type
	 * Byte 2: Length
	 * Byte 3: Value
	 */

	// 1. Base 64 Decode
	int result = base64_decode(in, inLength, out, &outlen);
	if(out[0]!=0 && keylen==0) {
		free(out);
		debugf("No key specified.");
		return;
	}

	// 2. Checking Checksum
	uint8_t bytes[4] = {0};
	for(int i=0; i<4; i++) {
		bytes[i]= out[i+1];
	}

	int offset = 5;
	int32_t crc = hashCode(out+offset, outlen-offset);
	bool success = compareChecksum(crc, bytes);
	if(!success) {
		free(out);
		debugf("Invalid CRC.");
		return;
	}

	// 3. Getting Clear Text Data
	int clearDataLength = outlen-offset;
	uint8_t *clearData = (uint8_t *)malloc(sizeof(uint8_t) * clearDataLength);

	switch(out[0]) {
	case 0:
		memcpy(clearData, out+offset, clearDataLength);
		break;
	case 1:
	case 2:
		// TODO: AES encoding
	default:
		for(int i=0; i < clearDataLength; i++) {
			clearData[i] = out[i+offset] ^ key[i % keylen];
		}
		break;
	}
	free(out);

	char value[255] = {0};
	// 4. Processing Commands
	for(int i=0; i<clearDataLength;) {
		int type =clearData[i++];
		int length =clearData[i++];

		debugf("TL: %d,%d", type, length);

		if(i+length > clearDataLength) {
			// detect broken TLVs
			break;
		}

		memcpy(value, clearData+i, length);
		value[length] = 0;

		debugf("TLV: %d,%d,%s", type, length, (char *)value);

		commands[type] = String(value, length);
		i += length;
	}

	free(clearData);
}
