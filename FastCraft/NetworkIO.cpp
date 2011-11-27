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
#include <Poco/ByteOrder.h>
#include <iostream>

using Poco::RuntimeException;
using std::memcpy;

NetworkIO::NetworkIO(std::queue<QueueJob>* p) : 
_Connection(),
	_sBuffer(""),
	_fConnected(false),
	_iTimeout(10), //10 seconds
	_pSendQueue(p)
{
}

NetworkIO::~NetworkIO() {
	if (_fConnected) {
		_Connection.close();
	}
	_sBuffer.clear();
}

unsigned long long NetworkIO::_iReadTraffic  = 0;
unsigned long long NetworkIO::_iWriteTraffic = 0;


void NetworkIO::addByte(char Byte) {
	_sBuffer.append<char>(1,Byte);
}

void NetworkIO::addBool(bool Bool) {
	if (Bool) {
		_sBuffer.append<char>(1,1);
	}else{
		_sBuffer.append<char>(1,0);
	}
}

void NetworkIO::addShort(short Short) {
	_sBuffer.append<char>(1, char(Short>>8)); 
	_sBuffer.append<char>(1, char(Short));
}


void NetworkIO::addInt(int iInt) {
	iInt = Poco::ByteOrder::toBigEndian(iInt);
	memcpy(_sEndianBuffer,&iInt,4);
	_sBuffer.append(_sEndianBuffer,4);
}

void NetworkIO::addInt64(long long iInt) {
	iInt = Poco::ByteOrder::toBigEndian(iInt);
	memcpy(_sEndianBuffer,&iInt,8);
	_sBuffer.append(_sEndianBuffer,8);
}

void NetworkIO::addFloat(float dVal) {
	int iBuff;

	memcpy(&iBuff,&dVal,4);
	iBuff = Poco::ByteOrder::toBigEndian(iBuff);
	memcpy(_sEndianBuffer,&iBuff,4);
	_sBuffer.append(_sEndianBuffer,4);
}


void NetworkIO::addDouble(double dVal) {
	long long iBuff;

	memcpy(&iBuff,&dVal,8); //copy double to an int64
	iBuff = Poco::ByteOrder::toBigEndian(iBuff); //switch endian
	memcpy(_sEndianBuffer,&iBuff,8);//copy to endian buffer
	_sBuffer.append(_sEndianBuffer,8);//append
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

	long long iVal;
	memcpy(&iVal,_charBuffer,8);

	return Poco::ByteOrder::fromBigEndian(iVal);
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

void NetworkIO::read(int iLenght) {
	if (!exceptionSaveReading(iLenght)) {
		throw RuntimeException("Connection aborted");
	}
}

bool NetworkIO::exceptionSaveReading(int iLenght) {
	int iReadedLenght = 0;
	bool fUnderflow = false;

	if (iLenght == 0) { 
		std::cout<<"NETWORKIO: recv lenght zero"<<"\n";
		return true;
	}

	if (iLenght < 0) { 
		std::cout<<"NETWORKIO: recv lenght lower than zero"<<"\n";
		return false;
	}

	if (iLenght > 4096) {
		return false;
	}


	while ( iReadedLenght < iLenght) {
		try {
			switch(fUnderflow) {
			case false:
				iReadedLenght = _Connection.receiveBytes(_charBuffer,iLenght);
				break;
			case true:
				iReadedLenght += _Connection.receiveBytes(&_charBuffer[iReadedLenght], iLenght - iReadedLenght);
				break;
			}
		}catch(Poco::Net::ConnectionAbortedException) {
			return false;
		}catch(Poco::Net::InvalidSocketException) {
			return false;
		}catch(Poco::TimeoutException) {
			std::cout<<"NETWORKIO: timeout"<<"\n";
			return false;
		}
		if (iReadedLenght != iLenght) {
			std::cout<<"underflow: r:"<<iReadedLenght<<" l:"<<iLenght<<"\n";
			fUnderflow = true;
		}
	}

	if (fUnderflow) {
		std::cout<<"solved"<<"\n";
		std::cout<<"r:"<<iReadedLenght<<" l:"<<iLenght<<"\n";
	}

	_iReadTraffic += iLenght;
	return true;
}

void NetworkIO::Flush(int iSpecial) {
	_Job.Data.assign(_sBuffer);
	_Job.Special = iSpecial;
	_pSendQueue->push(_Job);


	_iWriteTraffic += _sBuffer.length();
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

unsigned long long NetworkIO::getReadTraffic() {
	return _iReadTraffic;
}

unsigned long long NetworkIO::getWriteTraffic() {
	return _iWriteTraffic;
}

unsigned long long NetworkIO::getIOTraffic() {
	if (_iReadTraffic < 0 || _iWriteTraffic < 0) {
		std::cout<<"";
	}
	return _iReadTraffic + _iWriteTraffic;
}

string& NetworkIO::Str() {
	return _sBuffer;
}