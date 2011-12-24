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
#include <cstring>

using Poco::RuntimeException;

NetworkIO::NetworkIO(ThreadSafeQueue<string>& r,StreamSocket& rSock) : 
	_rSocket(rSock),
	_sBuffer(""),
	_iTimeout(10), //10 seconds
	_rQueue(r)
{
	_fLocked = false;
}

NetworkIO::~NetworkIO() {
	_sBuffer.clear();
}

unsigned long long NetworkIO::_iReadTraffic  = 0;
unsigned long long NetworkIO::_iWriteTraffic = 0;


void NetworkIO::addByte(unsigned char Byte) {
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

void NetworkIO::addInt64(signed long long iInt) {
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
	long long int iBuff;

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

	return (short(_charBuffer[0])<<8 & 0xFF00) | 
			(short(_charBuffer[1]) & 0x00FF);
}

int NetworkIO::readInt() {
	if (!exceptionSaveReading(4)) {
		throw RuntimeException("Connection aborted");
	}

	return (int(_charBuffer[0])<<24 & 0xFF000000 )| 
		(int(_charBuffer[1])<<16 & 0x00FF0000 )|
		(int(_charBuffer[2])<<8 & 0x0000FF00 )|
		(int(_charBuffer[3]) & 0x000000FF );
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
	int iUnderflowCount=0;
	bool fUnderflow = false;

	if (iLenght <= 0 || iLenght > 4096) { 
		return false;
	}

	while ( iReadedLenght < iLenght) {
		try {
			switch(fUnderflow) {
			case false:
				iReadedLenght = _rSocket.receiveBytes(_charBuffer,iLenght);
				break;
			case true:
				iReadedLenght += _rSocket.receiveBytes(&_charBuffer[iReadedLenght], iLenght - iReadedLenght);
				break;
			}
		}catch(Poco::Net::ConnectionAbortedException) {
			std::cout<<"NetworkIO error!"<<"\n";
			return false;
		}catch(Poco::Net::InvalidSocketException) {
			std::cout<<"NetworkIO error!"<<"\n";
			return false;
		}catch(Poco::TimeoutException) {
			std::cout<<"NetworkIO error!"<<"\n";
			return false;
		}catch(Poco::Net::ConnectionResetException) {
			std::cout<<"NetworkIO error!"<<"\n";
			return false;
		}catch(Poco::IOException) {
			std::cout<<"NetworkIO error!"<<"\n";
			return false;
		}
		if (iReadedLenght != iLenght) {
			iUnderflowCount++;
			if (iUnderflowCount > 10) {
				std::cout<<"error! u"<<"\n";
				return false;
			}
			fUnderflow = true;
		}
	}

	_iReadTraffic += iLenght;
	return true;
}

void NetworkIO::Flush() {
	_rQueue.push(_sBuffer);

	_iWriteTraffic += _sBuffer.length();
	_sBuffer.clear();
}

unsigned long long NetworkIO::getReadTraffic() {
	return _iReadTraffic;
}

unsigned long long NetworkIO::getWriteTraffic() {
	return _iWriteTraffic;
}

unsigned long long NetworkIO::getIOTraffic() {
	return _iReadTraffic + _iWriteTraffic;
}

string& NetworkIO::Str() {
	return _sBuffer;
}

void NetworkIO::packString(string& rString,string Input) {
	short iLenght = Input.size();
	
	rString.append<char>(1, char(iLenght>>8)); 
	rString.append<char>(1, char(iLenght));
	
	for(int x = 0;x<=Input.length()-1;x++) {
		rString.append<char>(1,0);
		rString.append(1,Input.at(x));
	}
}

void NetworkIO::Lock() {
	while(_fLocked) {
	}
	
	_fLocked = true;
}

void NetworkIO::UnLock() {
	_fLocked = false;
}




void NetworkIO::addByte(string& rBuff,char Byte) {
	rBuff.append<char>(1,Byte);
}

void NetworkIO::addByte(string& rBuff,unsigned char Byte) {
	rBuff.append<char>(1,Byte);
}

void NetworkIO::addBool(string& rBuff,bool Bool) {
	if (Bool) {
		rBuff.append<char>(1,1);
	}else{
		rBuff.append<char>(1,0);
	}
}

void NetworkIO::addShort(string& rBuff,short Short) {
	rBuff.append<char>(1, char(Short>>8)); 
	rBuff.append<char>(1, char(Short));
}


void NetworkIO::addInt(string& rBuff,int iInt) {
	char EndianBuffer[4];
	iInt = Poco::ByteOrder::toBigEndian(iInt);
	memcpy(EndianBuffer,&iInt,4);
	rBuff.append(EndianBuffer,4);
}

void NetworkIO::addInt64(string& rBuff,long long iInt) {
	char EndianBuffer[8];
	iInt = Poco::ByteOrder::toBigEndian(iInt);
	memcpy(EndianBuffer,&iInt,8);
	rBuff.append(EndianBuffer,8);
}

void NetworkIO::addFloat(string& rBuff,float dVal) {
	int iBuff;
	char EndianBuffer[4];

	memcpy(&iBuff,&dVal,4);
	iBuff = Poco::ByteOrder::toBigEndian(iBuff);
	memcpy(EndianBuffer,&iBuff,4);
	rBuff.append(EndianBuffer,4);
}


void NetworkIO::addDouble(string& rBuff,double dVal) {
	long long iBuff;
	char EndianBuffer[8];

	memcpy(&iBuff,&dVal,8); //copy double to an int64
	iBuff = Poco::ByteOrder::toBigEndian(iBuff); //switch endian
	memcpy(EndianBuffer,&iBuff,8);//copy to endian buffer
	rBuff.append(EndianBuffer,8);//append
}

void NetworkIO::addString(string& rBuff,string sString) {
	short iDataLength;

	iDataLength = sString.length();
	addShort(rBuff,iDataLength);

	if ( sString.length() == 0) {return;}

	for (int x=0;x<=sString.length()-1;x++) {
		rBuff.append<char>(1,0);
		rBuff.append(sString,x,1);
	}	
}
