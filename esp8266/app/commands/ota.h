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

#ifndef __APP_COMMANDS_OTA_H__
#define __APP_COMMANDS_OTA_H__

#if ENABLE_OTA==1

#include "updater.h"

#define CPU1_PIN 4 // For the test use the LED pin
#define MIN_OTA_HEAP 25500

// Firmware Update Code
#define UPDATE_URL  "https://x.attachix.com/update/check"

/**
 * Performs OverTheAir(OTA) Update
 *
 * @param char* params If param[0]== 's' - update to stable new release, else update to experimental new release
 */
void cmdOtaUpdate(char command, char* params);


/* Helper Functions */

void OtaUpdate(const char* romURL, bool optionalUpdates = false);
void checkOTAConnection();

/* Implementation */

FwUpdater* fwUpdater = 0;

void cmdOtaUpdate(char command, char* params) {
	if(params[0] == 's') {
		OtaUpdate(UPDATE_URL);
	}
	else {
		OtaUpdate(UPDATE_URL, true);
	}
}

void OtaUpdate(const char* romURL, bool optionalUpdates /* = false */) {
	if(system_get_free_heap_size() < MIN_OTA_HEAP) {
		debugf("OTA: Not enough memory to start OTA update!");
		Serial.write('F');
		return;
	}

	DeviceInfo deviceInfo;
	deviceInfo.id = (char *)deviceId.c_str();
	// WARNING:: Hard-Coded values
	deviceInfo.fw1v = (char *)"1.20";
	deviceInfo.fw2v = (char *)FW_VERSION;

	HttpRequest *request = new HttpRequest(URL(UPDATE_URL));
	request->setSslOptions(SSL_SERVER_VERIFY_LATER);

#ifdef ENABLE_SSL

	const uint8_t publicKeyFingerprint[] = {
				0x7e, 0xc6, 0x61, 0x88, 0xbe, 0xc2, 0x6c, 0x06, 0xdf, 0x74, 0x4b, 0x85, 0x05, 0x2e, 0xbc, 0x31,
				0xb9, 0x4d, 0xd9, 0x56, 0x30, 0xc6, 0x50, 0x8b, 0x6f, 0xef, 0x04, 0xf5, 0x2f, 0xa5, 0x42, 0x09
	};

	SSLFingerprints fingerprint;
	fingerprint.pkSha256 = new uint8_t[SHA256_SIZE];
	memcpy(fingerprint.pkSha256, publicKeyFingerprint, SHA256_SIZE);

	request->pinCertificate(fingerprint);
//	request->setSslClientKeyCert(getDeviceClientCert());
#endif

	fwUpdater->setBaseRequest(request);

	fwUpdater->update(UPDATE_URL, deviceInfo, optionalUpdates);
}

void checkOTAConnection()
{
	EStationConnectionStatus status = WifiStation.getConnectionStatus();
	if (status == eSCS_GotIP) {
		// we are connected
		wifiConnectionTimer.stop();
		OtaUpdate(UPDATE_URL);
	}
}

#endif /* ENABLE_OTA==1 */

#endif /* __APP_COMMANDS_OTA_H__ */

