/*
FastCraft - Minecraft SMP Server
Copyright (C) 2011  Michael Albrecht aka Sprenger120

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
*/

#ifndef _FASTCRAFTHEADER_TCPHELPER
#define _FASTCRAFTHEADER_TCPHELPER

#include <Poco/Net/StreamSocket.h>
#include <iostream>

using std::string;
using Poco::Net::StreamSocket;

class TCPHelper {
public:
	static string readString16(StreamSocket&);

	static int readInt(StreamSocket&);
	static short readShort(StreamSocket&);
	static char readByte(StreamSocket&);
	static bool readBool(StreamSocket&);
	static long long readInt64(StreamSocket&);
};


#endif