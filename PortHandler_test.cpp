/*
 * PortHandler_test.cpp
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

int main(void) {
	bool waiting = true;
	char buf[255] = {0};
	PortHandler ph("/dev/pts/3");
	ph.Open();
	ph.Configure(B9600 | CS8 | CLOCAL | CREAD, IGNPAR | ICRNL, 0, ICANON);

	while(waiting) {
		ph.Read(buf, sizeof(buf)/sizeof(char));
		//std::string buf = ph.ReadLine();
		//buf[res]=0;             /* set end of string, so we can printf */
		//printf(":%s:%d\n", buf, res -1);
		if (buf[0]=='z') waiting = false;

	}
	ph.Close();
	return 1;
}
