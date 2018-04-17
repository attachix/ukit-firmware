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

#ifndef MCU2_INCLUDE_FWUPDATER_H_
#define MCU2_INCLUDE_FWUPDATER_H_

#include <user_config.h>
#include <SmingCore.h>

#include <Network/HttpClient.h>

#include "tsb.h"

enum class FwType {
	TYPE_FW1 = 1, // Firmware for CPU1 - Attiny
	TYPE_FW2      //  Firmware for CPU2 - ESP8266
};

typedef struct {
	char* id;
	char* fw1v;
	char* fw2v;
} DeviceInfo;


typedef struct {
	uint32_t size = 0;
	uint32_t minVersion = 0;
	uint32_t checksum = 0;
	uint32_t reserved;
} FwUpdateMark; // IMPORTANT - the size must always be aligned to the 4th byte boundary

typedef HashMap<FwType, String> UpdateURL;

class FwUpdater {
public:

	int fw = 0; /* 0b00- no firmware was update, 0b01 - FW1 was update, 0b10 - FW2 was update, 0b11-both FW1 and FW2 were updated

	/*
	 * Sets the URL from where the update information will be fetched
	 */
	FwUpdater(int cpu1ResetPin);
	~FwUpdater();


	/**
	* Checks if there are new versions and if yes, uploads and applies them
	*
	* @param url checkURL
	* @return true if the update process can be started
	*/
	bool update(const String& url, const DeviceInfo& info, bool optionalUpdates = false);

	/**
	 * Returns if there is a pending update that is not yet applied.
	 * Basically it checks if we are using temporary ROM and the PENDING_UPDATE flag is set
	 */
	bool pending();

	/**
	 * Applies an update after restart
	 * @param String password the password that CPU1 has
	 */
	bool postUpdate(const String& password = "");

	/**
	 * Checks if there is a new firmware update.
	 * Sends information about the current Firmware and Hardware used
	 *
	 * Returns list of firmware URLs
	 */
	bool check(const String& url, const DeviceInfo& info, RequestCompletedDelegate onCheckCallback, bool optionalUpdates = false);

	/**
	 * Performs URL on the provided URLs.
	 * It does not check at all if the firmware version installed is newer then the one from the remote url.
	 */
	bool doUpdate(UpdateURL *urls);

	/**
	 * Callback handler that is called after the check has finished
	 */
	int checkCallback(HttpConnection& client, bool successful);

	/**
	 * Called When the Update is read
	 */
	void updateCallBack(rBootHttpUpdate& client, bool result);

	/* Sets the base request that can be used to pass
	 * - default request parameters, like request headers...
	 * - default SSL options
	 * - default SSL fingeprints
	 * - default SSL client certificates
	 *
	 * @param HttpRequest *
	 */
	void setBaseRequest(HttpRequest *request);

private:
	/**
	 * Stores the minimum FW2 version and the size of FW1 in Flash
	 */
	bool storeMark(FwUpdateMark info, uint32_t startAddr = TSB_CURRENT_MARK_ADDR);

	/**
	 * Reads the size of FW1 and the minVersion from flash
	 */
	FwUpdateMark readMark(uint32_t startAddr = TSB_CURRENT_MARK_ADDR);

	/**
	 * Zeros the minimum FW version, size and checksum.
	 */
	bool clearMark();

	/**
	 * Reads the FW1 data
	 * @param int size the size of the data to read
	 *
	 * @return pointer to the read data or NULL on error
	 */
	uint8_t* readFwData(int size, uint32_t startAddr = TSB_CURRENT_ROM_ADDR);

	rboot_rtc_data rtc;
	rboot_config bootConfig;
	int cpu1ResetPin;
	rBootHttpUpdate* otaUpdater = 0;
	HttpClient* httpClient = 0;

	HttpRequest* baseRequest = NULL;
};

#endif /* MCU2_INCLUDE_FWUPDATER_H_ */
