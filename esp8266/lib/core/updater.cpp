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

#include "updater.h"
#include "tsb.h"
#include "misc.h"
#include <spi_flash.h>

FwUpdater::FwUpdater(int cpu1ResetPin) {
	this->cpu1ResetPin = cpu1ResetPin;
	rboot_get_rtc_data(&rtc);

	bootConfig = rboot_get_config();
}

FwUpdater::~FwUpdater() {
	if(baseRequest != NULL) {
		delete baseRequest;
	}
	if(httpClient != NULL) {
		delete httpClient;
	}
	if(otaUpdater != NULL) {
		delete otaUpdater;
	}
}

bool FwUpdater::update(const String& url, const DeviceInfo& info, bool optionalUpdates  /* = false */) {
	return check(url, info, RequestCompletedDelegate(&FwUpdater::checkCallback, this), optionalUpdates);
}

bool FwUpdater::check(const String& url, const DeviceInfo& info, RequestCompletedDelegate onCheckCallback, bool optionalUpdates /* = false */) {
	if(httpClient == NULL) {
		httpClient = new HttpClient();
	}

	HttpRequest *request;
	if(baseRequest != NULL) {
		request = baseRequest->clone();
	}
	else {
		request = httpClient->request(url);
	}

	request->setURL(URL(url));

	HttpHeaders requestHeaders;
	if(optionalUpdates) {
		requestHeaders["x-fw-debug"] = "1";
	}
	requestHeaders["x-fw-did"] = String(info.id);
	requestHeaders["x-fw-v1"] = String(info.fw1v);
	requestHeaders["x-fw-v2"] = String(info.fw2v);
	requestHeaders["Connection"] = "keep-alive";
	request->setHeaders(requestHeaders);

	request->onRequestComplete(onCheckCallback);

	return httpClient->send(request);
}

int FwUpdater::checkCallback(HttpConnection& connection, bool successful) {
	debug_d("after Check Callback");

	if(!successful) {
		debug_e("Update failed. Response Code: %d", connection.getResponseCode());
		return -1;
	}

	UpdateURL *urls = new UpdateURL();
	Vector<String> lines;
	Vector<String> parts;

	// type(FW1 | FW2)|download-url|version|min-version.
	String body = connection.getResponseString();
	splitString(body, '\n', lines);

	for(int i=0; i<lines.count(); i++) {
		String line = lines[i];
		splitString(line, '|', parts);

		if(parts[0] == "0") {
			(*urls)[FwType::TYPE_FW2] = parts[1];
		}
		else if (parts[0] == "1") {
			(*urls)[FwType::TYPE_FW1] = parts[1];
		}
	}

	lines.clear();
	parts.clear();

	if(httpClient != NULL) {
		delete httpClient;
		httpClient = NULL;
	}

	doUpdate(urls);

	return 0;
}

bool FwUpdater::doUpdate(UpdateURL *urls) {
	if(!urls->count()) {
		return false;
	}

	uint8_t slot;

	debug_d("Updating...");

	// need a clean object, otherwise if run before and failed will not run again
	if (otaUpdater)
		delete otaUpdater;
	otaUpdater = new rBootHttpUpdate();

#ifdef ENABLE_SSL
	otaUpdater->setBaseRequest(baseRequest);
#endif

	// select rom slot to flash
	slot = bootConfig.current_rom;
	if (slot == 0)
		slot = 1;
	else
		slot = 0;

	if (urls->contains(FwType::TYPE_FW2)) {
		fw |= (int)FwType::TYPE_FW2;
		otaUpdater->addItem(bootConfig.roms[slot], (*urls)[FwType::TYPE_FW2]);
	}

#if ENABLE_OTA_TSB == 1
	if (urls->contains(FwType::TYPE_FW1)) {
		fw |= (int)FwType::TYPE_FW1;
		otaUpdater->addItem(TSB_CURRENT_ROM_ADDR, (*urls)[FwType::TYPE_FW1]);
	}
#endif

	// and/or set a callback (called on failure or success without switching requested)
	otaUpdater->setCallback(OtaUpdateDelegate(&FwUpdater::updateCallBack, this));

	// start update
	otaUpdater->start();

	if(urls!=NULL) {
		delete urls;
	}

	return true;
}

void FwUpdater::updateCallBack(rBootHttpUpdate& client, bool result) {
	debug_d("In OTA callback...");

	if(!result) {
		debug_e("Firmware update failed!");
		return;
	}

	uint8_t slot = bootConfig.current_rom;;
	if (slot == 0) slot = 1; else slot = 0;

	if(fw & (int)FwType::TYPE_FW2) {
		if(!(fw & (int)FwType::TYPE_FW1)) { // if only FW2 is updated
			// set to boot new rom and then reboot
			debug_d("FW2 updated, setting temporary ROM to %d...", slot);
			rboot_set_temp_rom(slot);
			System.restart();
		}

		rboot_set_current_rom(slot);
	}

	slot = (fw & (int)FwType::TYPE_FW2) ? 1: 0;
	rBootHttpUpdateItem item = client.getItem(slot);

	debug_d("Slot: %d, Item.size: %d", slot, item.size);

	// TODO: Check the response headers for that information
//	String minVersion = client.getResponseHeader("X-FW2-Min");
	int minFW2Version = 0;

	bool success = true;
	if(item.size < 1) {
		success = false;
	}

#ifdef TSB_CHECK_HEX
	if(success) {
		// check FW1
		uint8_t *data = readFwData(item.size);
		if(data) {
			Tsb tsb(&Serial);
			success = tsb.checkHex(data, item.size);
			if (!success) {
				debug_e("HEX CheckSum failed.");
			}

			delete[] data;
		}
		else {
			debug_e("Unable to read the hex data.");
			success = false;
		}
	}
#endif

	debug_d("Preparing to restart...");

	if(!success) {
		if(fw & (int)FwType::TYPE_FW2) {
			System.restart();
		}

		Serial.write('X'); // this will inform the other side to NOT reset

		return;
	}

	FwUpdateMark mark;
	mark.size = item.size;
	mark.minVersion = minFW2Version;
	storeMark(mark);

	// Inform about RESET
	Serial.write('F'); // this will inform the other side to RESET

	// after the last command CPU1 should be reset causing also CPU2 to power off.

	// we should not be here but ...
	if(fw & (int)FwType::TYPE_FW2) {
		System.restart();
	}
}

void FwUpdater::setBaseRequest(HttpRequest *request) {
	baseRequest = request;
}

/**
 * Returns if there is a pending update that is not yet applied.
 * Basically it checks if we are using temporary ROM and the PENDING_UPDATE flag is set
 */
bool FwUpdater::pending() {
#if ENABLE_OTA_DEFAULT_ROM == 1
	if(bootConfig.mode == MODE_GPIO_ROM) {
		debug_d("************** RUNNING DEFAULT BOOT ROM ********************");
#if ENABLE_OTA_TSB == 1
		return true;
#endif
	}

	return false;
#else

#if ENABLE_OTA_TSB == 1
	FwUpdateMark mark = readMark();

	return (mark.size > 0);
#else
	return false;
#endif

#endif
}

bool FwUpdater::postUpdate(const String& password /*  = "" */) {

#if ENABLE_OTA_TSB == 1
	uint32_t markAddr = TSB_CURRENT_MARK_ADDR;
	uint32_t romAddr = TSB_CURRENT_ROM_ADDR;

#if ENABLE_OTA_DEFAULT_ROM == 1
	if(bootConfig.mode == MODE_GPIO_ROM) {
		markAddr = TSB_DEFAULT_MARK_ADDR;
		romAddr  = TSB_DEFAULT_ROM_ADDR;
	}
#endif

	FwUpdateMark mark = readMark(markAddr);

	debug_d("Mark size: %d", mark.size);

	if(mark.size < 1) {
		return false;
	}

	// TODO: Check minVersion
	uint8_t *firmwareData = readFwData(mark.size, romAddr);
	if(!firmwareData) {
		debug_e("Unable to read the FW1 data");

		return false;
	}

	Tsb tsb(&Serial);
	if (!tsb.connect(password)) {
		debug_e("Unable to communicate with TSB");
		logInfo("Unable to communicate with TSB");
		delete[] firmwareData;

		return false;
	}

	logInfo("Connected to TSB");

	size_t bytesWritten  = tsb.writeFlash(firmwareData, mark.size);

#ifdef REMOTE_DEBUG
	char value[250] = {0};
	ets_sprintf(value, "Finished: s=%d, wrtn=%d", mark.size, bytesWritten);
	logInfo(String(value));
#endif

	if (bytesWritten != mark.size) {
		debug_e("Unable to write the new flash data!");
		delete[] firmwareData;

		return false;
	}

	// IDEA: Count the unsuccessful attempts and clear the mark if more than X failures.
	if(bootConfig.mode != MODE_GPIO_ROM) {
		clearMark();
	}

	logInfo("TSB: Finish");

	delete[] firmwareData;

	tsb.run();

	return true;
#endif

	return true;
}

bool FwUpdater::storeMark(FwUpdateMark mark, uint32_t startAddr /*= TSB_CURRENT_MARK_ADDR */) {
	int markSize = sizeof(FwUpdateMark);

	// calculate checksum
	mark.checksum = mark.size ^ mark.minVersion;

	uint8_t *addr = (uint8 *)&mark;

	spi_flash_erase_sector(startAddr / SPI_FLASH_SEC_SIZE);
	int error = spi_flash_write(startAddr, (uint32*)((void*)addr), markSize);

	debug_d("Writing flash data. Error code: %d", error);

	return (!error);
}

FwUpdateMark FwUpdater::readMark(uint32_t startAddr /* = TSB_CURRENT_MARK_ADDR */) {
	FwUpdateMark mark;
	int markSize = sizeof(FwUpdateMark); // IMPORTANT - the size must always be aligned to the 4th byte boundary

	uint8_t *addr = (uint8 *)&mark;
	spi_flash_read(startAddr, (uint32*)((void*)addr), markSize);

	if(mark.checksum != (mark.size ^ mark.minVersion)) {
		mark.size = 0;
		mark.minVersion = 0;
		mark.checksum = 0;
	}

	return mark;
}

bool FwUpdater::clearMark() {
	FwUpdateMark mark;
	mark.size = 0;
	mark.minVersion = 0;
	mark.checksum = 0;

	return storeMark(mark);
}
uint8_t* FwUpdater::readFwData(int size,  uint32_t startAddr) {
	if(size % 4) { // align the size to 4 byte boundary
		size = ((size / 4 ) +1 ) * 4;
	}
	uint8_t *data = new uint8_t[size];

	if(!data) {
		debug_e("Unable to allocate memory for FW1");
		return NULL;
	}

	int  error = spi_flash_read(startAddr, (uint32*)((void*)data), size);
	if(error) {
		debug_e("FwUpdater::readFwData got error: %d", error);
		delete[] data;

		return NULL;
	}

	return data;
}
