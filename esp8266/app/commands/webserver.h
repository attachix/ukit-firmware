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

#ifndef __APP_COMMANDS_WEBSERVER_H__
#define __APP_COMMANDS_WEBSERVER_H__

#if ENABLE_WEBSERVER==1

HttpServer* webServer = NULL;

/**
 * Starts or Stops the Web Server engine
 *
 * @param char* params If param[0]== '0' - Stop the Web Server, else start the Web Server (if allowed to be started)
 */
void cmdToggleWebServer(char command, char* params);

/* Implementation */

void onIndex(HttpRequest &request, HttpResponse &response);
void onFile(HttpRequest &request, HttpResponse &response);

void cmdToggleWebServer(char command, char* params) {
	if(params[0] == '0') {
		debugf("Stopping Web Server...");

		if(webServer == NULL) {
			return;
		}

		debugf("Before WebServer stop: %d", system_get_free_heap_size());

		webServer->shutdown();

		debugf("After WebServer stop: %d", system_get_free_heap_size());
	}
	else if (params[0] == '1') {
		debugf("Starting Web Server...");
		webServer = new HttpServer();
		webServer->listen(80);

		webServer->addPath("/", onIndex);
		webServer->setDefaultHandler(onFile);
	}
}

void onIndex(HttpRequest &request, HttpResponse &response)
{
	TemplateFileStream *tmpl = new TemplateFileStream("index.html");
	auto &vars = tmpl->variables();

	vars["smobotversion"] = "98";

	response.sendTemplate(tmpl); // will be automatically deleted
}

void onFile(HttpRequest &request, HttpResponse &response)
{
	String file = request.getPath();
	if (file[0] == '/')
		file = file.substring(1);

	if (file[0] == '.')
		response.forbidden();
	else
	{
		response.setCache(86400, true);
		response.sendFile(file);
	}
}
#endif /* ENABLE_WEBSERVER==1 */

#endif /* __APP_COMMANDS_WEBSERVER_H__ */

