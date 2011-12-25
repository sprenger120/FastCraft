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
#include "NetworkIn.h"
#include <Poco/Exception.h>
#include <Poco/Net/NetException.h>
#include <Poco/ByteOrder.h>
#include <cstring>
using Poco::RuntimeException;

NetworkIn::NetworkIn(StreamSocket& r) :
_rSocket(r)
{
}

unsigned long long NetworkIn::_iReadTraffic = 0;


NetworkIn::~NetworkIn() {
}

char NetworkIn::readByte() {
	try {
		read(1);
	} catch(Poco::RuntimeException& ex) {
		ex.rethrow();
	}

	return _sReadBuffer[0];
}

bool NetworkIn::readBool() {
	char iByte;

	try {
		iByte = readByte();
	} catch(Poco::RuntimeException& ex) {
		ex.rethrow();
	}


	if (iByte == 0) {
		return false;
	}else{
		return true;
	}
}

short NetworkIn::readShort() {
	try {
		read(2);
	} catch(Poco::RuntimeException& ex) {
		ex.rethrow();
	}

	return (short(_sReadBuffer[0])<<8 & 0xFF00) | 
			(short(_sReadBuffer[1]) & 0x00FF);
}

int NetworkIn::readInt() {
	try {
		read(4);
	} catch(Poco::RuntimeException& ex) {
		ex.rethrow();
	}

	return (int(_sReadBuffer[0])<<24 & 0xFF000000 )| 
		(int(_sReadBuffer[1])<<16 & 0x00FF0000 )|
		(int(_sReadBuffer[2])<<8 & 0x0000FF00 )|
		(int(_sReadBuffer[3]) & 0x000000FF );
}


long long NetworkIn::readInt64() {
	try {
		read(8);
	} catch(Poco::RuntimeException& ex) {
		ex.rethrow();
	}

	long long iVal;
	memcpy(&iVal,_sReadBuffer,8);

	return Poco::ByteOrder::fromBigEndian(Poco::Int64(iVal));
}

float NetworkIn::readFloat() {
	int iInt;

	try {
		iInt = readInt();
	} catch(Poco::RuntimeException& ex) {
		ex.rethrow();
	}

	float dVal;

	memcpy(&dVal,&iInt,4);

	return dVal;
}


double NetworkIn::readDouble() {
	Poco::Int64 iInt;

	try {
		iInt = readInt64();
	} catch(Poco::RuntimeException& ex) {
		ex.rethrow();
	}

	double dVal;
	memcpy(&dVal,&iInt,8);
	return dVal;
}

string NetworkIn::readString() {
	string sOutput("");
	short iDataLenght;

	try {
		iDataLenght = readShort();

		for(int x = 0;x<=iDataLenght-1;x++) {
			readByte();
			sOutput.append(1,readByte());
		}
	} catch(Poco::RuntimeException& ex) {
		ex.rethrow();
	}

	return sOutput;
}


void NetworkIn::read(int iLenght) {
	int iReadedLenght = 0;
	int iUnderflowCount=0;
	bool fUnderflow = false;

	if (iLenght <= 0 || iLenght > 8) { 
		throw Poco::RuntimeException("Invalid lenght");
	}

	while ( iReadedLenght < iLenght) {
		try {
			switch(fUnderflow) {
			case false:
				iReadedLenght = _rSocket.receiveBytes(_sReadBuffer,iLenght);
				break;
			case true:
				iReadedLenght += _rSocket.receiveBytes(&_sReadBuffer[iReadedLenght], iLenght - iReadedLenght);
				break;
			}
		}catch(Poco::Net::ConnectionAbortedException) {
			throw Poco::RuntimeException("Connection aborted");
		}catch(Poco::Net::InvalidSocketException) {
			throw Poco::RuntimeException("Invalid socket");
		}catch(Poco::TimeoutException) {
			throw Poco::RuntimeException("Timeout");
		}catch(Poco::Net::ConnectionResetException) {
			throw Poco::RuntimeException("Connection reset");
		}catch(Poco::IOException) {
			throw Poco::RuntimeException("I/O Error");
		}
		if (iReadedLenght != iLenght) {
			iUnderflowCount++;
			if (iUnderflowCount > 10) {
				throw Poco::RuntimeException("Connection too slow");
			}
			fUnderflow = true;
		}
	}

	_iReadTraffic += iLenght;
}

unsigned long long NetworkIn::getReadTraffic() {
	return _iReadTraffic;
}