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

#ifndef MCU2_INCLUDE_APP_DATA_H_
#define MCU2_INCLUDE_APP_DATA_H_

#include <user_config.h>
#include <SmingCore.h>

#define DATA_MAGIC (0x58545441u)

#define DATA_FLAG_WIFI 	 bit(1)
#define DATA_FLAG_STEALTHMODE bit(2)

typedef struct {
	uint32_t magic=DATA_MAGIC;
	uint32_t version = 2;
	char owner[50] = "\0";
	char mqttHost[50] = "attachix.com";
	uint32_t mqttPort = 8883;
	uint32_t flags= DATA_FLAG_STEALTHMODE;
	/* Reserved for future versions. The total structure can hold up to 4K of data */
} AppDataType;

class AppData {
public:
	AppDataType data;

	AppData(uint32_t startAddress);
	bool load();
	bool save();
	/**
	 * Migrates configuration from older version to newer version
	 * @return true if the data was migrated, false if there was no need for migration
	 */
	bool migrate();
private:
	uint32_t startAddress = 0;
};

#endif /* MCU2_INCLUDE_CLI_H_ */
