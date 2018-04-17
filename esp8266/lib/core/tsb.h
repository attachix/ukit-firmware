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

#ifndef LIB_TSB_H_
#define LIB_TSB_H_

#include <user_config.h>
#include <SmingCore.h>

#include "HardwareSerial.h"

#define TSB_INFO_SIZE 2*8
#define TSB_CONFIRM   '!'
#define TSB_REQUEST   '?'

#define TSB_CMD_READ_FLASH   'f'
#define TSB_CMD_WRITE_FLASH   'F'
#define TSB_CMD_READ_EEPROM  'e'
#define TSB_CMD_WRITE_EEPROM  'E'
#define TSB_CMD_READ_USERDATA   'c'
#define TSB_CMD_WRITE_USERDATA   'C'
#define TSB_CMD_RUN        'r'

//#define TSB_DEFAULT_MARK_ADDR 0x300000
//#define TSB_CURRENT_MARK_ADDR 0x306000
//#define TSB_DEFAULT_ROM_ADDR 0x301000
//#define TSB_CURRENT_ROM_ADDR 0x307000

#ifndef TSB_CPU_FREQUENCY
#define TSB_CPU_FREQUENCY 1
#endif

typedef struct {
    uint8_t magic[3];
    uint8_t firmware_date[2];
    uint8_t firmware_status;
    uint8_t signature[3];
    uint8_t page_size;
    uint16_t mem_size;
    uint16_t eeprom_size;
    uint16_t extra;
} TsbDeviceInfo;

class Tsb {
public:

	Tsb(HardwareSerial *stream);

	/**
	 * Connects to the device
	 */
	bool connect(String password = "");

	/**
	 * Changes the password
	 */
	bool setPassword(String password);

	/**
	 * Changes the timeout
	 */
	bool setTimeout(int seconds);

	/**
	 * Writes all data at once.
	 *
	 * @return bytes actually written
	 */
	size_t writeFlash(uint8_t *bytes, int length);

	/**
	 * Forces the TSB loader to execute the APP
	 */
	bool run();

	uint8_t *readUserData();
	bool writeUserData(uint8_t *userData);
	bool readDeviceInfo();

private:
	TsbDeviceInfo *deviceInfo = nullptr;
	HardwareSerial *stream    = nullptr;
};



#endif /* LIB_TSB_H_ */
