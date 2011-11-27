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

#ifndef _FASTCRAFTHEADER_NETWORKIO
#define _FASTCRAFTHEADER_NETWORKIO
#include <Poco/Net/StreamSocket.h>
#include <string>
#include <queue>
#include "Structs.h"

using Poco::Net::StreamSocket;
using std::string;

//This class sets the streamsocket automaticly to blocking
class NetworkIO {
private:
	StreamSocket _Connection;
	string _sBuffer;
	char _charBuffer[4096];
	char _sEndianBuffer[8];

	QueueJob _Job;

	std::queue<QueueJob>* _pSendQueue;

	bool _fConnected;
	const int _iTimeout;

	static unsigned long long _iReadTraffic;
	static unsigned long long _iWriteTraffic;
public:
	NetworkIO(std::queue<QueueJob>*); //Init NetworkIO without connection
	~NetworkIO();

	//Write part
	void addByte(char);
	void addBool(bool);
	void addShort(short);
	void addInt(int);
	void addInt64(long long);
	void addFloat(float);
	void addDouble(double);
	void addString(string);

	//Read part
	char readByte(); 
	bool readBool();
	short readShort();
	int readInt();
	long long readInt64();
	float readFloat();
	double readDouble();
	string readString();
	void read(int); 

	void Flush(int = FC_JOB_NO); 

	//Connection adding / closing
	bool isConnected();
	void closeConnection();
	void newConnection(StreamSocket&);

	//Traffic
	static unsigned long long getReadTraffic();
	static unsigned long long getWriteTraffic();
	static unsigned long long getIOTraffic();

	//Special
	string& Str(); //Returns a reference of the internal buffer
private:
	bool exceptionSaveReading(int); //Reads data from tcp stream, checks all thrown exceptions, increase readtraffic
}; 
#endif