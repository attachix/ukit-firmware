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

#ifndef MCU2_INCLUDE_SCPROTO_H_
#define MCU2_INCLUDE_SCPROTO_H_

#include <user_config.h>
#include <SmingCore.h>

enum class ScProtoType {
	TYPE_CONTINUATION = 0,
	TYPE_PASSWORD,
	TYPE_OWNER,
	TYPE_MODE,
	TYPE_OTA
};

typedef HashMap<int,String> ScCommands;

class ScProto {
public:
	void parse(ScCommands& commands, const char *in,  uint8_t *key = NULL, int keylen = 0);
private:
	int32_t hashCode(uint8_t *data, int length);
	bool compareChecksum(int32_t n, uint8_t *bytes);
};

#endif /* MCU2_INCLUDE_SCPROTO_H_ */
