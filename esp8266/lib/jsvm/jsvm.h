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

#ifndef MCU2_INCLUDE_JSVM_H_
#define MCU2_INCLUDE_JSVM_H_

#include <user_config.h>
#include <SmingCore.h>

#include "jerry-core/include/jerryscript.h"

/**
 * Our JavaScript VM Engine
 */
class Jsvm {
public:
	/**
	* Initializes the JavaScript VM
	*/
	Jsvm(jerry_init_flag_t flags = JERRY_INIT_EMPTY);

	/*
	 * Parses the JavaScript code and prepares it for execution
	 */
	bool eval(String jsCode);

	/**
	 * Loads a snapshot from file and executes it.
	 * @return 0 on success
	 * 		   negative on error
	 */
	int exec(const char *fileName);

	/**
	 * Executes a snapshot file
	 */
	bool exec(uint8_t *snapshot, size_t snapshot_size);

	/**
	 * Runs a specified JavaScript function
	 *
	 * @return true if the function exists and was called successfully
	 */
	bool runFunction(String functionName);

	/**
	 * Runs the loop JavaScript function
	 */
	bool runLoop();

	int registerFunction(const char* name_p, jerry_external_handler_t handler);

	/**
	 * Binds Native C/C++ function to JavaScript
	 */
	bool bindNativeFunction() { return false; }

	~Jsvm();
};

#endif /* MCU2_INCLUDE_JSVM_H_ */
