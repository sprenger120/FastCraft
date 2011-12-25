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

#ifndef _FASTCRAFTHEADER_NETWORKIN
#define _FASTCRAFTHEADER_NETWORKIN
#include <Poco/Net/StreamSocket.h>
#include <string>

using Poco::Net::StreamSocket;
using std::string;

class NetworkIn {
private:
	StreamSocket & _rSocket;
	char _sReadBuffer[8];
	static unsigned long long _iReadTraffic;
public:
	/*
	* Constructor

	Parameter:
	@1 : Reference to a StreamSocket
	*/
	NetworkIn(StreamSocket&);
	
	/*
	* Destructor
	*/
	~NetworkIn();

	/*
	* Read functions
	* Will throw Poco::RuntimeException if connection get aborted
	*/
	char readByte(); 
	bool readBool();
	short readShort();
	int readInt();
	long long readInt64();
	float readFloat();
	double readDouble();
	string readString();


	/*
	* Returns byte count that have been readed so far
	*/
	static unsigned long long getReadTraffic();
private:
	void read(int);
};
#endif