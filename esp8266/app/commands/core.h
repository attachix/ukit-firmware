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

#ifndef __APP_COMMANDS_CORE_H__
#define __APP_COMMANDS_CORE_H__

// Commands
#define PROTOCOL_READY (char)0x5a

enum ProtocolCommand {
	PROTOCOL_COMMAND_CONFIG = 0x30,
	PROTOCOL_COMMAND_SMOKE,
	PROTOCOL_COMMAND_TEMPERATURE,
	PROTOCOL_COMMAND_MOTION,
	PROTOCOL_COMMAND_BATTERY
};

static char mode = 0;

/**
* Change Mode Command
*/
void cmdModeSmartConfig(char command, char* params);

/**
 * Smoke event
 */
void cmdEventSmoke(char command, char* params);

/*
 * General Event
 */
void cmdEvent(char command, char* params);

bool persistWifiConfig();
//void smartConfigCallback(sc_status status, void *pdata);
void checkSmartConfigConnection();

#if ENABLE_OTA == 1
void checkOTAConnection();
#endif /* ENABLE_OTA == 1  */

/* Implementation */

Timer wifiConnectionTimer;

void cmdEventSmoke(char command, char* params) {
	char value[25] = {0};
	ets_sprintf(value, "%d|%d", params[0], params[1]);

	sendAndHalt(String("dev/") + DEVICE_TYPE + "/" + deviceId + "/smoke",
				value,
				String(PROTOCOL_READY));
}

void cmdEvent(char command, char* params) {
	HashMap<char, String> commandTopic;
	commandTopic[PROTOCOL_COMMAND_MOTION] 	   = "motion";
	commandTopic[PROTOCOL_COMMAND_TEMPERATURE] = "temperature";
	commandTopic[PROTOCOL_COMMAND_BATTERY] 	   = "battery";

	if(!commandTopic.contains(command)) {
		debugf("Cannot handle this command: %x", command);
		return;
	}

	char value[5] = {0};
	ets_sprintf(value, "%d", params[0]);

	sendAndHalt(String("dev/") + DEVICE_TYPE+ "/" + deviceId + "/" + commandTopic[command],
				value,
				String(PROTOCOL_READY));
}

// Helper functions

bool persistWifiConfig() {
	station_config config = {0};
	bool cfgreaded = wifi_station_get_config(&config);
	if (!cfgreaded) {
		debugf("Can't read station configuration!");
		return false;
	}

	wifi_station_set_config(&config);
	return true;
}

static void smartConfigCallback(sc_status status, void *pdata) {
	bool finish = false;

    switch (status) {
		case SC_STATUS_WAIT:
			debugf("SC_STATUS_WAIT\n");
			break;
		case SC_STATUS_FIND_CHANNEL:
			debugf("SC_STATUS_FIND_CHANNEL\n");
//			pinMode(LED_PIN, OUTPUT);
//			if(!bitsSet(appData->data.flags, DATA_FLAG_STEALTHMODE)) {
//				blinkTimer.initializeMs(1000, blink).start();
//			}

			break;
		case SC_STATUS_GETTING_SSID_PSWD:
			debugf("SC_STATUS_GETTING_SSID_PSWD\n");
			break;
		case SC_STATUS_LINK: {
			debugf("SC_STATUS_LINK\n");
			station_config *sta_conf = (station_config *) pdata;
			char *ssid = (char*) sta_conf->ssid;
			char *password = (char*) sta_conf->password;
			password[sizeof(sta_conf->password)] = 0;
			bool appDataChanged = false;

			ScProto proto;
			ScCommands commands;

			debugf("SSID: %s, PW: %s", ssid, password);

			int keylen;
			uint8_t *key = (uint8_t *)loadPsk(&keylen);
			proto.parse(commands, password, key, keylen);

			debugf("Commands count: %d", commands.count());

			if(!commands.count()) {
				mode = 'X';
				finish = true;
				break;
			}

			for(int i=0, len = commands.count(); i<len; i++) {
				int k = commands.keyAt(i);
				String v = commands.valueAt(i);

				debugf("K: %d, V:[%s]", k, v.c_str());

				switch((ScProtoType)k) {
				case ScProtoType::TYPE_MODE:
					debugf("Change Mode:%s", v.c_str());
					mode = (v.c_str())[0];
					break;

				case ScProtoType::TYPE_OWNER:
					debugf("Change Owner:%s", v.c_str());
					strcpy(appData->data.owner, v.c_str());
					appDataChanged = true;
					break;

				case ScProtoType::TYPE_PASSWORD:
					debugf("Checking new WIFI password...");
					// set the password temporary
					WifiStation.config(ssid, v.c_str(), true, false);
					WifiStation.connect();
					// start a check connection timer...
					wifiConnectionTimer.initializeMs(5000, checkSmartConfigConnection).start();
					break;

#if ENABLE_OTA == 1
				case ScProtoType::TYPE_OTA:
					debugf("Perform OTA");
					wifiConnectionTimer.initializeMs(5000, checkOTAConnection).start();

					break;
#endif

				default:
					debugf("Unknown command: %d", k);
				}
			}

			if(!commands.contains((int)ScProtoType::TYPE_PASSWORD)) {
				WifiStation.connect();
				delay(100);
				WifiStation.smartConfigStop();
				if(appDataChanged) {
					appData->save();
				}
				finish = true;
			}
			free(key);

			break;
		}
		case SC_STATUS_LINK_OVER:
			debugf("SC_STATUS_LINK_OVER\n");
			wifiConnectionTimer.stop();
			WifiStation.smartConfigStop();
			persistWifiConfig();
			setBits(appData->data.flags, DATA_FLAG_WIFI);
			appData->save();
			finish = true;
			break;

		default:
			debugf("Unknown status: %d", (int)status);
			break;
	}

	if(finish) {
		if(!bitsSet(appData->data.flags, DATA_FLAG_STEALTHMODE)) {
			blinkTimer.stop();
			if(state) {
				blink();
			}
		}
		respondAndHalt(String(mode), 5);
	}
}

void cmdModeSmartConfig(char command, char* params) {
	debugf("Entering Smart Config Mode");
	WifiAccessPoint.enable(false);
	WifiStation.enable(true);
	smartconfig_stop();
	smartconfig_set_type(SC_TYPE_ESPTOUCH);
	smartconfig_start(smartConfigCallback); // use 1 as a second parameter to print debug messages.
}

void checkSmartConfigConnection()
{
	EStationConnectionStatus status = WifiStation.getConnectionStatus();
	bool stopTimer = false;
	if (status == eSCS_GotIP) {
		// we are connected -> store permanently the password
		WifiStation.smartConfigStop();
		persistWifiConfig();
		stopTimer = true;

		if(mqtt != nullptr) { // create a new mqtt object with the new owner
			delete mqtt;
		}
		getMqttClient();

		String topic = String("dev/") + DEVICE_TYPE + "/" + deviceId + "/reg";

		station_config config = {0};
		wifi_station_get_config(&config);

		String message((char *)config.ssid);
		mqtt->publish(topic, message, true);

		// TODO: wait for the communication to finish...

		respondAndHalt(String(mode), 5);
	}
	else if (status != eSCS_Connecting) {
		// if we are not connected reset the smartconfig
		WifiStation.smartConfigStop();
		cmdModeSmartConfig((char)NULL,(char *)NULL);
		stopTimer = true;
	}

	if(stopTimer) {
		wifiConnectionTimer.stop();
	}
}

#endif /* __APP_COMMANDS_CORE_H__ */

