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

#include "cli.h"

void Cli::init() {
	consoleCommands = new HashMap<char, CommandDefinition>;
	Serial.setCallback(
			StreamDataReceivedDelegate(&Cli::commandProcessor, this));
}

bool Cli::addCommand(char code, int size, consoleFuncDelegate func, uint8_t type/* =CLI_CMD_SIMPLE*/) {
	if (consoleCommands->contains(code)) {
		debug_w("Command is already registered");
		return false;
	}

	debug_d("Registering Command: %x, Size: %d, Type: %d", code, size, type);

	(*consoleCommands)[code].size = size;
	(*consoleCommands)[code].cmd = func;
	(*consoleCommands)[code].type = type;

	return true;
}

bool Cli::removeCommand(char code) {
	if (consoleCommands->contains(code)) {
		consoleCommands->remove(code);
		return true;
	}

	return false;
}

void Cli::setFlag(uint8_t flag) {
	this->flag |= flag;

	for(int i=0; i < pendingCommands.count(); i++) {
		uint8_t type = pendingCommands.keyAt(i);
		if(!bitsSet(this->flag, type)) {
			continue;
		}

		while(pendingCommands[type].available()) {
			commandProcessor(pendingCommands[type], pendingCommands[type].peek(), pendingCommands[type].available());
			WDT.alive();
		}
	}
}

void Cli::unsetFlag(uint8_t flag) {
	this->flag &= ~flag;
}

void Cli::commandProcessor(Stream& stream, char arrivedChar, unsigned short availableCharsCount) {
	debug_d("Command: %x, Arrived Char: %x, Chars: %d", currentCommand, arrivedChar, availableCharsCount);

	if (!currentCommand) {
		currentCommand = stream.read();
		availableCharsCount--;
		if (!consoleCommands->contains(currentCommand)) {
			debug_e("Invalid command: 0x%x. Skipping", currentCommand);
			currentCommand = 0;
			return;
		}
	}

	debug_d("Current Command: 0x%x, Expected Size: %d, Available size: %d",
			currentCommand,
			(*consoleCommands)[currentCommand].size,
			availableCharsCount);

	int expectedSize = (*consoleCommands)[currentCommand].size;
	if ( availableCharsCount >= expectedSize) {
		char params[expectedSize];
		for (int i = 0; i < expectedSize; i++) {
			params[i] = stream.read();
		}

		uint8_t type = (*consoleCommands)[currentCommand].type;
		if(!bitsSet(flag, type)) {
			debugf("Queuing command:  0x%x", currentCommand);

			pendingCommands[type].write(currentCommand);
			pendingCommands[type].write((uint8_t *)params, expectedSize);

			return;
		}

		debug_d("executing command: 0x%x", currentCommand);
		if(expectedSize > 0) {
			debug_d("First param: 0x%x", params[0]);
		}
		(*consoleCommands)[currentCommand].cmd(currentCommand, params);
		currentCommand = 0;
	}
}
