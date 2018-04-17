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

#ifndef MCU2_INCLUDE_CLI_H_
#define MCU2_INCLUDE_CLI_H_

#include <user_config.h>
#include <SmingCore.h>
#include "../SmingCore/DataSourceStream.h"

typedef Delegate<void(char command, char* params)> consoleFuncDelegate;

// Simple Commands that should be executed out of the box
#define CLI_CMD_SIMPLE bit(0)

// Commands that should be executed when WIFI is on
#define CLI_CMD_NETWORK bit(1)

// Defines critical commands
#define CLI_CMD_CRITICAL bit(2)

typedef struct {
	uint32_t size = 0; // Command Length Size in Bytes
	consoleFuncDelegate cmd = 0;
	uint8_t type = 0;
} CommandDefinition;

/**
 * Class that contains the console line processing code
 */
class Cli {
public:
	void init();
	bool addCommand(char code, int size, consoleFuncDelegate func, uint8_t type=CLI_CMD_SIMPLE);
	void setFlag(uint8_t flag); // used to enable types of commands
	void unsetFlag(uint8_t flag); // used to disable types of commands
	bool removeCommand(char code);
	void commandProcessor(Stream& stream, char arrivedChar, unsigned short availableCharsCount);

private:
	HashMap<char, CommandDefinition> *consoleCommands = nullptr;
	char currentCommand = 0;
	uint8_t flag = CLI_CMD_SIMPLE;
	HashMap<uint8_t, MemoryDataStream> pendingCommands;
};

#endif /* MCU2_INCLUDE_CLI_H_ */
