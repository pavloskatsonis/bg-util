/*
 * PortHandler.h
 *
 *  Created on: 23 Μαρ 2013
 *      Author: pavlos
 */

/* Copyright (C) 2013  Pavlos Katsonis.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef PORTHANDLER_H_
#define PORTHANDLER_H_
#include <iostream>
#include <termios.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/signal.h>
#include <string.h>

/*
 * PortHandler Handles serial port (opens, closes, configures, reads & writes to it).
 */
class PortHandler {
public:
	PortHandler(const char*);
	virtual ~PortHandler();

	int Open();
	int OpenAsync();
	int Close();
	int Configure(termios);
	int Configure(tcflag_t, tcflag_t, tcflag_t, tcflag_t);
	int Read(char *buffer);

protected:
	int Configure();

	/* definition of signal handler */
	void signal_handler_IO (int);

	const char* device;
	struct termios old_settings,settings;
	int port_descriptor;
	bool isopen, isasync, wait_flag;
	char buf[255];
};

#endif /* PORTHANDLER_H_ */
