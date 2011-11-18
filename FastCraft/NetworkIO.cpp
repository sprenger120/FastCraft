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

#include "NetworkIO.h"
#include <iostream>
#include <Poco/Net/NetException.h>
#include <Poco/Timespan.h>
#include <Poco/Exception.h>
#include <iostream>

using Poco::RuntimeException;
using std::memcpy;

NetworkIO::NetworkIO() : 
_Connection(),
	_sBuffer(""),
	_fConnected(false),
	_iTimeout(10) //10 seconds
{
}

NetworkIO::NetworkIO(StreamSocket& Sock) :
_Connection(Sock),
	_sBuffer(""),
	_fConnected(false),
	_iTimeout(10) //10 seconds
{
	_Connection.setReceiveTimeout( Poco::Timespan( 1000 * 5000) );
	_Connection.setBlocking(true);
}

NetworkIO::~NetworkIO() {
	if (_fConnected) {
		_Connection.close();
	}
	_sBuffer.clear();
}

int NetworkIO::_iReadTraffic  = 0;
int NetworkIO::_iWriteTraffic = 0;


void NetworkIO::addByte(char Byte) {
	_sBuffer.append<char>(1,Byte);
	_iWriteTraffic++;
}

void NetworkIO::addBool(bool Bool) {
	if (Bool) {
		_sBuffer.append<char>(1,1);
	}else{
		_sBuffer.append<char>(1,0);
	}
	_iWriteTraffic++;
}

void NetworkIO::addShort(short Short) {
	_sBuffer.append<char>(1, char(Short>>8)); 
	_sBuffer.append<char>(1, char(Short));
	_iWriteTraffic += 2;
}


void NetworkIO::addInt(int iInt) {
	_sBuffer.append<char>(1, char( iInt>>24)); 
	_sBuffer.append<char>(1, char( iInt>>16)); 
	_sBuffer.append<char>(1, char( iInt>>8)); 
	_sBuffer.append<char>(1, char( iInt ));  
	_iWriteTraffic += 4;
}

void NetworkIO::addInt64(long long iInt) {
	_sBuffer.append<char>(1, char( iInt>>54)); 
	_sBuffer.append<char>(1, char( iInt>>48)); 
	_sBuffer.append<char>(1, char( iInt>>40)); 
	_sBuffer.append<char>(1, char( iInt>>32)); 
	_sBuffer.append<char>(1, char( iInt>>24)); 
	_sBuffer.append<char>(1, char( iInt>>16)); 
	_sBuffer.append<char>(1, char( iInt>>8)); 
	_sBuffer.append<char>(1, char( iInt)); 
	_iWriteTraffic += 8;
}

void NetworkIO::addFloat(float dVal) {
	int iBuff;

	std::memcpy(&iBuff,&dVal,4);

	_sBuffer.append<char>(1, char( iBuff>>24)); 
	_sBuffer.append<char>(1, char( iBuff>>16)); 
	_sBuffer.append<char>(1, char( iBuff>>8));
	_sBuffer.append<char>(1, char( iBuff ));  

	_iWriteTraffic += 4;
}


void NetworkIO::addDouble(double dVal) {
	long long iBuff;

	std::memcpy(&iBuff,&dVal,8);

	_sBuffer.append<char>(1, char( iBuff>>54)); 
	_sBuffer.append<char>(1, char( iBuff>>48)); 
	_sBuffer.append<char>(1, char( iBuff>>40)); 
	_sBuffer.append<char>(1, char( iBuff>>32)); 
	_sBuffer.append<char>(1, char( iBuff>>24)); 
	_sBuffer.append<char>(1, char( iBuff>>16)); 
	_sBuffer.append<char>(1, char( iBuff>>8)); 
	_sBuffer.append<char>(1, char( iBuff)); 
	_iWriteTraffic += 8;
}

void NetworkIO::addString(string sString) {
	short iDataLength;

	iDataLength = sString.length();
	addShort(iDataLength);

	if ( sString.length() == 0) {return;}

	for (int x=0;x<=sString.length()-1;x++) {
		_sBuffer.append<char>(1,0);
		_sBuffer.append(sString,x,1);
	}	

	_iWriteTraffic += sString.length() * 2;
}

char NetworkIO::readByte() {
	if (!exceptionSaveReading(1)) {
		throw RuntimeException("Connection aborted");
	}

	return _charBuffer[0];
}

bool NetworkIO::readBool() {
	char iByte;
	
	try {
		iByte = readByte();
	}catch(RuntimeException) {
		throw RuntimeException("Connection aborted");
	}
	
	
	if (iByte == 0) {
		return false;
	}else{
		return true;
	}
}

short NetworkIO::readShort() {
	if (!exceptionSaveReading(2)) {
		throw RuntimeException("Connection aborted");
	}

	return short(_charBuffer[0])<<8 | 
		   short(_charBuffer[1]);
}

int NetworkIO::readInt() {
	if (!exceptionSaveReading(4)) {
		throw RuntimeException("Connection aborted");
	}

	return int(_charBuffer[0])<<24 | 
		   int(_charBuffer[1])<<16 |
		   int(_charBuffer[2])<<8 |
		   int(_charBuffer[3]);
}


long long NetworkIO::readInt64() {
	if (!exceptionSaveReading(8)) {
		throw RuntimeException("Connection aborted");
	}

	return long long(_charBuffer[0])<<54 | 
		   long long(_charBuffer[1])<<48 |
		   long long(_charBuffer[2])<<40 |
		   long long(_charBuffer[3])<<32 |
		   long long(_charBuffer[4])<<24 |
		   long long(_charBuffer[5])<<16 |
		   long long(_charBuffer[6])<<8  |
		   long long(_charBuffer[7]);
}

float NetworkIO::readFloat() {
	int iInt;
	
	try {
		iInt = readInt();
	}catch(RuntimeException) {
		throw RuntimeException("Connection aborted");
	}
	
	float dVal;
	memcpy(&dVal,&iInt,4);
	
	return dVal;
}


double NetworkIO::readDouble() {
	long long iInt;
	
	try {
		iInt = readInt64();
	}catch(RuntimeException) {
		throw RuntimeException("Connection aborted");
	}
	
	double dVal;
	memcpy(&dVal,&iInt,8);
	
	return dVal;
}

string NetworkIO::readString() {
	string sOutput("");
	short iDataLenght;

	try {
		iDataLenght = readShort();
		
		for(int x = 0;x<=iDataLenght-1;x++) {
			readByte();
			sOutput.append(1,readByte());
		}
	}catch(RuntimeException) {
		throw RuntimeException("Connection aborted");
	}

	return sOutput;
}

bool NetworkIO::exceptionSaveReading(int iLenght) {
	int iReadedLenght;
	try {
		iReadedLenght = _Connection.receiveBytes(_charBuffer,iLenght);
	}catch(Poco::Net::ConnectionAbortedException) {
		return false;
	}catch(Poco::Net::InvalidSocketException) {
		return false;
	}catch(Poco::TimeoutException) {
		return false;
	}

	if (iReadedLenght != iLenght) { 
		return false;
	}

	_iReadTraffic += iLenght;

	return true;
}

void NetworkIO::Flush() {
	_Connection.sendBytes(_sBuffer.c_str(),_sBuffer.length());
	_sBuffer.clear();
}

bool NetworkIO::isConnected() {
	return _fConnected;
}

void NetworkIO::closeConnection() {
	_Connection.close();
	_fConnected = false;
}

void NetworkIO::newConnection(StreamSocket& Sock) {
	_Connection = StreamSocket(Sock);
	_fConnected = true;
	_Connection.setReceiveTimeout( Poco::Timespan( 1000 * 5000) );
	_Connection.setBlocking(true);
}

int NetworkIO::getReadTraffic() {
	return _iReadTraffic;
}

int NetworkIO::getWriteTraffic() {
	return _iWriteTraffic;
}

int NetworkIO::getIOTraffic() {
	return _iReadTraffic + _iWriteTraffic;
}