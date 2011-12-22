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
#include "ThreadSafeQueue.h"

using Poco::Net::StreamSocket;
using std::string;


class NetworkIO {
private:
	StreamSocket& _rSocket;
	string _sBuffer;
	char _charBuffer[4096];
	char _sEndianBuffer[8];

	ThreadSafeQueue<string>& _rQueue;

	bool _fLocked;
	const int _iTimeout;

	static unsigned long long _iReadTraffic;
	static unsigned long long _iWriteTraffic;
public:
	NetworkIO(ThreadSafeQueue<string>&,StreamSocket&); //Init NetworkIO without connection
	~NetworkIO();

	//Static members>
	static void addByte(string&,char);
	static void addByte(string&,unsigned char);
	static void addBool(string&,bool);
	static void addShort(string&,short);
	static void addInt(string&,int);
	static void addInt64(string&,long long);
	static void addFloat(string&,float);
	static void addDouble(string&,double);
	static void addString(string&,string);


	//Write part
	void addByte(unsigned char);
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

	void Flush(); 


	//Lock & unlock
	void Lock();
	void UnLock();

	//StringPacking
	static void packString(string&,string);

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