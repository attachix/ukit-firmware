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

#ifndef MCU2_LIB_CORE_INFO_H_
#define MCU2_LIB_CORE_INFO_H_

#include <user_config.h>
#include <SmingCore.h>

#define RBOOT_SIZE 3680 /* !!!! Change this value if the size of rBOOT changes */
//#define MANUFACTURER_DATA_ADDR  0x00000ea0
//#define DEVICE_DATA_ADDR 0x200000

typedef struct {
	uint32_t id; // Id of the manufacturer
	uint32_t factory; // Factory number of the manufacturer
	char serial[15]; // serial number
} ManufacturerData;


typedef struct {
	char password[50];
	uint8_t hasClientCert = 0;
} DeviceData;

/**
 * Gets the manufacturer data for the device
 */
ManufacturerData* getManufacturerData();

/**
 * Gets the unique device data
 */
DeviceData* getDeviceData();

#ifdef ENABLE_SSL

/**
 * Gets the client certificate for this device
 *
 * @return SSLKeyCertPair client certificate or NULL if none exists
 */
SSLKeyCertPair getDeviceClientCert();

#endif /* ENABLE_SSL */

/**
 * Loads the device PSK
 */
char* loadPsk(int *keylen);

/**
 * Gets device unique id
 */
String getDevUID();

#endif /* MCU2_LIB_CORE_INFO_H_ */
