/*
 * PortHandler.cpp
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

#include "PortHandler.h"

PortHandler::PortHandler(const char* new_device) {
	// TODO Auto-generated constructor stub
	isopen = false;
	isasync = false;
	wait_flag = false;
	device = new_device;
	port_descriptor = -1;
}

int PortHandler::Open() {
	isasync = false;

	port_descriptor = open(device, O_RDWR | O_NOCTTY);
	if (port_descriptor <0) {perror(device); return -1; }
	/* save current port settings */
	tcgetattr(port_descriptor,&old_settings);
	wait_flag = true;
	return port_descriptor;
}

int PortHandler::OpenAsync() {
	isasync = true;

	port_descriptor = open(device, O_RDWR | O_NOCTTY | O_NONBLOCK);
	if (port_descriptor <0) {perror(device); return -1; }
	/* save current port settings */
	tcgetattr(port_descriptor,&old_settings);

	struct sigaction saio;

	/* install the signal handler before making the device asynchronous */
	sigemptyset(&saio.sa_mask);
	saio.sa_flags = 0; //saio.sa_flags|SA_SIGINFO;
	saio.sa_restorer = NULL;
	//saio.sa_handler = this->signal_handler_IO();
	sigaction(SIGIO,&saio,NULL);

	/* allow the process to receive SIGIO */
	if(fcntl(port_descriptor, F_SETOWN, getpid()) == -1) {
		printf("Could not set owner process.");
		close(port_descriptor);
		return -1;
	}
	/* Make the file descriptor asynchronous (the manual page says only O_APPEND and O_NONBLOCK, will work with F_SETFL...) */
	if(fcntl(port_descriptor, F_SETFL, O_ASYNC) == -1) {
		printf("Could not open port asynchronously.");
		close(port_descriptor);
		return -1;
	}
	wait_flag = true;
	return port_descriptor;
}

int PortHandler::Close() {
	return (port_descriptor >= 0 ? close(port_descriptor) : -1);
}

int PortHandler::Configure() {
	if(port_descriptor >= 0 && &settings != NULL) {
		tcflush(port_descriptor, TCIFLUSH);
		return tcsetattr(port_descriptor,TCSANOW,&settings);
	}
	else return -1;
}

int PortHandler::Configure(termios new_settings) {
	memset(&settings, 0, sizeof(settings)); /* clear struct for new port settings */
	settings = new_settings;
	return this->Configure();
}

int PortHandler::Configure(tcflag_t cflag, tcflag_t iflag, tcflag_t oflag, tcflag_t lflag) {
	memset(&settings, 0, sizeof(settings)); /* clear struct for new port settings */

	/*
          BAUDRATE: Set bps rate. You could also use cfsetispeed and cfsetospeed.
          CRTSCTS : output hardware flow control (only used if the cable has
                    all necessary lines. See sect. 7 of Serial-HOWTO)
          CS8     : 8n1 (8bit,no parity,1 stopbit)
          CLOCAL  : local connection, no modem contol
          CREAD   : enable receiving characters
	 */
	settings.c_cflag = cflag; // B9600 | CS8 | CLOCAL | CREAD

	/*
          IGNPAR  : ignore bytes with parity errors
          ICRNL   : map CR to NL (otherwise a CR input on the other computer
                    will not terminate input)
          otherwise make device raw (no other input processing)
	 */
	settings.c_iflag = iflag; // IGNPAR | ICRNL

	/*
         Raw output.
	 */
	settings.c_oflag = oflag; //0;

	/*
          ICANON  : enable canonical input
          disable all echo functionality, and don't send signals to calling program
	 */
	settings.c_lflag = lflag; //ICANON;

	/*
          initialize all control characters
          default values can be found in /usr/include/termios.h, and are given
          in the comments, but we don't need them here
	 */
	settings.c_cc[VINTR]    = 0;     /* Ctrl-c */
	settings.c_cc[VQUIT]    = 0;     /* Ctrl-\ */
	settings.c_cc[VERASE]   = 0;     /* del */
	settings.c_cc[VKILL]    = 0;     /* @ */
	settings.c_cc[VEOF]     = 4;     /* Ctrl-d */
	settings.c_cc[VTIME]    = 0;     /* inter-character timer unused */
	settings.c_cc[VMIN]     = 1;     /* blocking read until 1 character arrives */
	settings.c_cc[VSWTC]    = 0;     /* '\0' */
	settings.c_cc[VSTART]   = 0;     /* Ctrl-q */
	settings.c_cc[VSTOP]    = 0;     /* Ctrl-s */
	settings.c_cc[VSUSP]    = 0;     /* Ctrl-z */
	settings.c_cc[VEOL]     = 0;     /* '\0' */
	settings.c_cc[VREPRINT] = 0;     /* Ctrl-r */
	settings.c_cc[VDISCARD] = 0;     /* Ctrl-u */
	settings.c_cc[VWERASE]  = 0;     /* Ctrl-w */
	settings.c_cc[VLNEXT]   = 0;     /* Ctrl-v */
	settings.c_cc[VEOL2]    = 0;     /* '\0' */

	return this->Configure();
}

PortHandler::~PortHandler() {
	// TODO Auto-generated destructor stub

	/* restore the old port settings */
	if(port_descriptor >= 0 && &old_settings != NULL) {
		tcsetattr(port_descriptor,TCSANOW,&old_settings);
	}
	this->Close();
}

/* signal handler. sets wait_flag to FALSE, to indicate above loop that characters have been received.*/
void PortHandler::signal_handler_IO (int status)
{
	printf("received SIGIO signal with status %i.\n", status);
	wait_flag = false;
}

int PortHandler::Read(char *buffer, int length) {
	if(port_descriptor >= 0) {
		int res = read(port_descriptor, buffer, length);
		buffer[res] = 0;
		if(res > 0 && buffer[res-1] == 10) {
			res--;
			buffer[res] = 0;
		}
		return res;
	}
	else return -1;
}

std::string PortHandler::ReadLine() {
	if(port_descriptor >= 0) {
		char buffer[255] = {0};
		int res = read(port_descriptor, buffer, sizeof(buffer)/sizeof(char));
		buffer[res] = 0;
		if(res > 0 && buffer[res-1] == 10) {
			res--;
			buffer[res] = 0;
		}
		std::string line(buffer);
		return line;
	}
	else return NULL;
}
