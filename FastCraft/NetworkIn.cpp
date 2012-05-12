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
#include "NetworkIn.h"
#include "FCRuntimeException.h"
#include <Poco/Net/NetException.h>


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
	} catch(FCRuntimeException& ex) {
		ex.rethrow();
	}

	return _sReadBuffer[0];
}

bool NetworkIn::readBool() {
	char iByte;

	try {
		iByte = readByte();
	} catch(FCRuntimeException& ex) {
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
	} catch(FCRuntimeException& ex) {
		ex.rethrow();
	}

	return (short(_sReadBuffer[0])<<8 & 0xFF00) | 
			(short(_sReadBuffer[1]) & 0x00FF);
}

int NetworkIn::readInt() {
	try {
		read(4);
	} catch(FCRuntimeException& ex) {
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
	} catch(FCRuntimeException& ex) {
		ex.rethrow();
	}

	return((((long long)_sReadBuffer[0])<<56  & 0xFF00000000000000) |
            (((long long)_sReadBuffer[1])<<48 & 0x00FF000000000000) |
			(((long long)_sReadBuffer[2])<<40 & 0x0000FF0000000000) |
			(((long long)_sReadBuffer[3])<<32 & 0x000000FF00000000) |
			(((long long)_sReadBuffer[4])<<24 & 0x00000000FF000000) |
			(((long long)_sReadBuffer[5])<<16 & 0x0000000000FF0000) |
			(((long long)_sReadBuffer[6])<<8  & 0x000000000000FF00) |
			((long long)_sReadBuffer[7]       & 0x00000000000000FF));
}

float NetworkIn::readFloat() {
	try {
		_ItF.i = readInt();
	} catch(FCRuntimeException& ex) {
		ex.rethrow();
	}

	return _ItF.d;
}


double NetworkIn::readDouble() {
	try {
		_ItD.i = readInt64();
	} catch(FCRuntimeException& ex) {
		ex.rethrow();
	}
	return _ItD.d;
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
	} catch(FCRuntimeException& ex) {
		ex.rethrow();
	}

	return sOutput;
}


void NetworkIn::read(int iLenght) {
	int iReadedLenght = 0;
	int iUnderflowCount=0;
	bool fUnderflow = false;

	if (iLenght <= 0 || iLenght > 8) { 
		throw FCRuntimeException("Invalid lenght");
	}

	while ( iReadedLenght < iLenght) {
		try {
			switch(fUnderflow) {
			case false:
				iReadedLenght = _rSocket.receiveBytes(_sReadBuffer,iLenght);
				break;
			case true:
				std::cout<<"underflow!!\n";
				iReadedLenght += _rSocket.receiveBytes(&_sReadBuffer[iReadedLenght], iLenght - iReadedLenght);
				break;
			}
		}catch(Poco::Net::ConnectionAbortedException) {
			throw FCRuntimeException("Connection aborted");
		}catch(Poco::Net::InvalidSocketException) {
			throw FCRuntimeException("Invalid socket");
		}catch(Poco::TimeoutException) {
			throw FCRuntimeException("Timeout");
		}catch(Poco::Net::ConnectionResetException) {
			throw FCRuntimeException("Connection reset");
		}catch(Poco::IOException) {
			throw FCRuntimeException("I/O Error");
		}
		if (iReadedLenght != iLenght) {
			iUnderflowCount++;
			if (iUnderflowCount > 10) {
				throw FCRuntimeException("TCP Underflow");
			}
			fUnderflow = true;
		}
	}
	_iReadTraffic += iLenght;
}

unsigned long long NetworkIn::getReadTraffic() {
	return _iReadTraffic;
}
