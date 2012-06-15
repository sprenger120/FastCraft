/*
FastCraft - Minecraft SMP Server
Copyright (C) 2011 - 2012 Michael Albrecht aka Sprenger120

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
*/

#ifndef _FASTCRAFTHEADER_NETWORKIN
#define _FASTCRAFTHEADER_NETWORKIN
#include <Poco/Net/StreamSocket.h>
#include <string>
#include "Structs.h"

using Poco::Net::StreamSocket;
using std::string;
class MinecraftServer;

class NetworkIn {
private:
	IntToFloat _ItF;
	Int64ToDouble _ItD;
	char _sReadBuffer[8];

	StreamSocket & _rSocket;
	MinecraftServer* _pMCServer;
public:
	/*
	* Constructor

	Parameter:
	@1 : Reference to a StreamSocket
	@2 : a MinecraftServer instance
	*/
	NetworkIn(StreamSocket&,MinecraftServer*);
	

	/*
	* Read functions
	* Will throw FCRuntimeException if connection get aborted
	*/
	char readByte(); 
	bool readBool();
	short readShort();
	int readInt();
	long long readInt64();
	float readFloat();
	double readDouble();
	string readString();
private:
	void read(int);
};
#endif
