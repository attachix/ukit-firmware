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

#ifndef __APP_COMMANDS_JSVM_H__
#define __APP_COMMANDS_JSVM_H__

#if ENABLE_JSVM==1
/**
 * Starts or Stops the JSVM engine
 *
 * @param char* params If param[0]== '0' - Stop the JSVM, else start the JSVM (if allowed to be started)
 */
void cmdPlayJS(char command, char* params);

/* Implementation */
#include "../../lib/jsvm/jsvm.h"

Timer jsLoopTimer;
Jsvm* jsVm;

#define TASK_JS_Q_LENGTH 3
#define TASK_JS_QUEUE 2
#define TASK_JS_MAINLOOP 0

os_event_t *jsTaskQueue;

#define MAIN_JS_FILE "main.js.snap"

void jsLoopTicker() {
	system_os_post(TASK_JS_QUEUE, TASK_JS_MAINLOOP, 0);
}

void jsLoop() {
	if(!jsVm->runFunction("loop")) {
		debugf("Failed running 'loop()' function in code.");
		jsLoopTimer.stop();
	}
}

static void jsEventHandler(os_event_t *event) {
	switch (event->sig) {
	  case TASK_JS_MAINLOOP:
		  jsLoop();
		  break;

	  default:
	    debugf("jsEventHandler: Unsupported type: %d", event->sig);
	    break;
	}
}

void cmdPlayJS(char command, char* params) {
	if(params[0] == '0') {
		debugf("Stopping JSVM...");

		if(jsVm == NULL) {
			return;
		}

		debugf("Before JsVM stop: %d", system_get_free_heap_size());

		jsLoopTimer.stop();

		free(jsTaskQueue);

		delete jsVm;
		jsVm = NULL;

		debugf("After JsVM stop: %d", system_get_free_heap_size());
	}
	else if (params[0] == '1') {
		debugf("Starting JSVM...");
		// Look for a file called main.js. If that one exists load it and run it.
		if(!fileExist(MAIN_JS_FILE)) {
			debugf("Unable to read file: %s", MAIN_JS_FILE);
			return;
		}

		delete jsVm;
		jsVm = new Jsvm();
		// Load the snapshot file and run it
		if(jsVm->exec(MAIN_JS_FILE) < 0 ) {
			debugf("Failed executing the following script: %s", MAIN_JS_FILE);
			delete jsVm;
			jsVm = NULL;
			return;
		}

		// Run function setup in it.
		if(!jsVm->runFunction("setup")) {
			debugf("Failed running 'setup()' function in code: %s", MAIN_JS_FILE);
			delete jsVm;
			jsVm = NULL;
		}

		debugf("After JsVM run setup: %d", system_get_free_heap_size());

		// Add ticker for the function loop
		if(!jsVm->runFunction("loop")) {
			debugf("Failed running 'loop()'function in code: %s", MAIN_JS_FILE);
			delete jsVm;
			jsVm = NULL;
			return;
		}


		free(jsTaskQueue);
		jsTaskQueue = (os_event_t *) malloc(sizeof(os_event_t) * TASK_JS_Q_LENGTH);
		system_os_task(jsEventHandler, TASK_JS_QUEUE, jsTaskQueue, TASK_JS_Q_LENGTH);
		jsLoopTimer.initializeMs(100, jsLoopTicker).start();
	} /* if params[0] == '1' */
}
#endif /* ENABLE_JSVM==1 */


#endif /* __APP_COMMANDS_JSVM_H__ */

