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

#include <user_config.h>
#include <SmingCore/SmingCore.h>

#include "cli.h"
#include "scproto.h"
#include "app-data.h"
#include "info.h"
#include "misc.h"

#ifndef FW_VERSION
#define FW_VERSION "X"
#endif

int start = millis();

// [Console Line Interpreter Init]
Cli cli;

// [AppData Init]
AppData* appData = NULL;

// typeid=1/model=u2h/hw_version=1
#define DEVICE_TYPE "1/u2h/1"

String deviceId = getDevUID();
String devicePsk = "";

#ifndef RELEASE
void ShowInfo()
{
    debugf("\n");
    debugf("Firmware version: %s", FW_VERSION);
    debugf("SDK: v%s", system_get_sdk_version());
    debugf("Free Heap: %d", system_get_free_heap_size());
    debugf("CPU Frequency: %d MHz", system_get_cpu_freq());
    debugf("System Chip ID: %x", system_get_chip_id());
    debugf("SPI Flash ID: %x", spi_flash_get_id());
    debugf("SPI Flash Size: %d", (1 << ((spi_flash_get_id() >> 16) & 0xff)));

    rboot_config conf;
    conf = rboot_get_config();

    debugf("Count: %d", conf.count);
    debugf("ROM 0: %d", conf.roms[0]);
    debugf("ROM 1: %d", conf.roms[1]);
    debugf("ROM 2: %d", conf.roms[2]);
    debugf("GPIO ROM: %d", conf.gpio_rom);

	// mount spiffs
	uint8_t slot = 0;
	rboot_rtc_data rtc;
	rboot_get_rtc_data(&rtc);
	slot = rtc.last_rom;

	debugf("Current rom used: %d. Mode: %d", slot, rtc.last_mode);
#ifdef RBOOT_GPIO_ENABLED
	debugf("=== GPIO ENABLED ===");
#endif /* RBOOT_GPIO_ENABLED */

	int i = 0;
	char *psk = loadPsk(&i);
	debugf("PSK: %s", psk);

}
#endif /* RELEASE */

// Blinker
#define LED_PIN 2 // GPIO2
Timer blinkTimer;
bool state = true;

void blink()
{
	digitalWrite(LED_PIN, state);
	state = !state;
}

void respondAndHalt(String content, uint32_t msDelay=50);

// MQTT client
MqttClient *mqtt = nullptr;
String globalResponse;

MqttClient* getMqttClient()
{
	if (mqtt == nullptr) {
		mqtt = new MqttClient(appData->data.mqttHost, appData->data.mqttPort);
		mqtt->addSslOptions(SSL_SERVER_VERIFY_LATER);
		mqtt->connect(deviceId, appData->data.owner, devicePsk, true);
	}

	return mqtt;
}

void mqttPublishCompleted(uint16_t msgId, int type)
{
	respondAndHalt(globalResponse, 0);
}

void respondAndHalt(String content, uint32_t msDelay /* =50 */)
{
	Serial.print(content);

	debugf("Finished after: %d ms", millis() - start);
	start = millis();
#ifdef RELEASE
	delayMilliseconds(msDelay);
	System.deepSleep(0);
#endif /* RELEASE */
}

void sendAndHalt(String topic, String message, String response)
{
	getMqttClient();
	if(mqtt->publishWithQoS(topic, message, 1, true, mqttPublishCompleted)) {
		globalResponse = response;
		return;
	}

	debugf("Unable to publish mqtt message.");
}

#include "commands/core.h"
#include "commands/testmode.h"
#include "commands/ota.h"
#include "commands/jsvm.h"
#include "commands/webserver.h"

void ready()
{
	// Send the ready response
	Serial.print(PROTOCOL_READY);

	// Commented out temporary. It is part of the original code!
#if 0
	if(rtc.last_mode == MODE_TEMP_ROM) {
		// nothing died until now, switch permanently to this ROM
		debugf("Switching permanently to ROM %d", slot);
		rboot_set_current_rom(slot);
	}
#endif
}

void connectOk(IPAddress ip, IPAddress netmask, IPAddress gateway)
{
	// Enable networking commands
	debug_d("===Connected to the network!====");
	cli.setFlag(CLI_CMD_NETWORK);
}

void connectFail(String ssid, uint8_t ssidLength, uint8_t bssid[6], uint8_t reason)
{
	// disable networking commands
	cli.unsetFlag(CLI_CMD_NETWORK);
}

void init()
{
	Serial.begin(SERIAL_BAUD_RATE); // 115200 by default

#ifdef DEBUG
	HardwareSerial Serial1(UART1);
	Serial1.begin(SERIAL_BAUD_RATE);
	Serial1.systemDebugOutput(true); // Debug output to UART1
	ShowInfo();
#else
	Serial.systemDebugOutput(false);
#endif /* DEBUG */
	
#if ENABLE_OTA == 1
	// up to here already 300ms have elapsed
	fwUpdater = new FwUpdater(CPU1_PIN);
	if(fwUpdater->pending()) {
		debugf("== Entering OTA mode ...");
		pinMode(LED_PIN, OUTPUT);
		blink();
		fwUpdater->postUpdate(); // TODO: Add later the password
		debugf("== Out of OTA mode ...");
	}
#endif /* ENABLE_OTA == 1 */

	spiffs_mount_manual(RBOOT_SPIFFS_0, SPIFF_SIZE);
	debugf("After Spiff Mount: %d", system_get_free_heap_size());

	// [ Load Application Data ]
	appData = new AppData(APP_DATA_ADDR);
	appData->load();

	// [ Register commands ]
	cli.init();
	cli.addCommand(PROTOCOL_COMMAND_CONFIG, 	 0, cmdModeSmartConfig);
	// -- commands that require valid network connection
	cli.addCommand(PROTOCOL_COMMAND_SMOKE, 		 1, cmdEventSmoke, CLI_CMD_NETWORK);
	cli.addCommand(PROTOCOL_COMMAND_TEMPERATURE, 1, cmdEvent, CLI_CMD_NETWORK);
	cli.addCommand(PROTOCOL_COMMAND_MOTION,		 1, cmdEvent, CLI_CMD_NETWORK);
	cli.addCommand(PROTOCOL_COMMAND_BATTERY, 	 1, cmdEvent, CLI_CMD_NETWORK);
#if ENABLE_OTA == 1
	cli.addCommand('o', 1, cmdOtaUpdate, CLI_CMD_NETWORK); // perform OverTheAir Update
#endif /* ENABLE_OTA == 1 */

	System.onReady(ready);

#ifdef TEST_MODE
	clearBits(appData->data.flags, DATA_FLAG_STEALTHMODE);

	debugf("== Entering Test Mode ==");
	debugf("	c - Simulates ChangeMode command.");
	debugf("	l - Loads configuration from FS.");
	debugf("	w[0|1] - Set 'wifiIsSet' flag. w0 - wifi is OFF, w1 - wifi is ON.");
#if ENABLE_JSVM==1
	debugf("	j[0|1] - Toggle JSVM: j1 - start JSVM. j0 - Stop JSVM. ");
#endif /* ENABLE_JSVM==1 */
#if ENABLE_WEBSERVER==1
	debugf("	s[0|1] - Toggle WebServer: s1 - start WebServer. s0 - Stop WebServer. ");
#endif /* ENABLE_WEBSERVER==1 */
	debugf("	m - Simulates detected motion.");
#if ENABLE_OTA == 1
	debugf("	o[s|t] - Makes over the air (OTA) update. S-stable, T-testing");
#endif /* ENABLE_OTA == 1 */
	debugf("	r[0|1|2] - Switches the ROMs. r0 - temporary switch to next ROM, r1 - permanent switch to next rom., r2 - temp switch to default rom");
	debugf("	f - Shows the free heap.");
	debugf("	k[0|1] - Crash handler tests. k0 - load and print crash data for current rom, k1 - crash the device");
	debugf("	test - Performs check for the CLI processor.");

	cli.addCommand('c', 0, cmdModeSmartConfig); // Simulates ChangeMode command.
	cli.addCommand('l', 0, cmdShowData); // Load configuration
	cli.addCommand('w', 1, cmdSaveData);  // Toggle Wifi flag and save it w0 - wifi is OFF, w1 - wifi is ON
#if ENABLE_JSVM==1
	cli.addCommand('j', 1, cmdPlayJS);  // 0 - stop JSVM, 1 - start JSVM
#endif /* ENABLE_JSVM==1 */
#if ENABLE_WEBSERVER==1
	cli.addCommand('s', 1, cmdToggleWebServer);  // 0 - stop JSVM, 1 - start JSVM
#endif /* ENABLE_WEBSERVER==1 */

	cli.addCommand('m', 0, cmdFakeMotion, CLI_CMD_NETWORK);  // Motion
	cli.addCommand('r', 1, cmdRomSwitch); // r0 - perform temporary switch r1 - perform permanent switch
	cli.addCommand('f', 0, cmdFreeHeap);
	cli.addCommand('x', 0, cmdSoftReset);
	cli.addCommand('t', 3, cmdTest); // type "test" - performs check for the cli processor

	cli.addCommand('k', 1, cmdCrash); // 0 - load and print crash data for current rom, 1 - crash the device
#endif /* TEST_MODE */

	debugf("Connection status: %d", WifiStation.getConnectionStatus());

	WifiEvents.onStationGotIP(connectOk);
	WifiEvents.onStationDisconnect(connectFail);

	if(!bitsSet(appData->data.flags, DATA_FLAG_WIFI)) {
		return;
	}

	WifiAccessPoint.enable(false);
	WifiStation.enable(true);
}
