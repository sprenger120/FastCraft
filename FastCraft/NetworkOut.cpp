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
#include "NetworkOut.h"
#include "NetworkOutRoot.h"
#include "Constants.h"
#include <Poco/ByteOrder.h>
#include <cstring>
#include "FCException.h"
#include "PlayerThread.h"
//using CryptoPP::StringSink;

NetworkOut::NetworkOut(NetworkOutRoot* p) 
{
	_pMaster = p;
//	_cfbEncryptor = NULL;
	_pNetworkBuffer = new string("");
}

NetworkOut::~NetworkOut() {
	delete _pNetworkBuffer;
}

void NetworkOut::addByte(string& rBuff,char Byte) {
	rBuff.append(1,Byte);
}

void NetworkOut::addByte(string& rBuff,unsigned char Byte) {
	rBuff.append(1,Byte);
}

void NetworkOut::addBool(string& rBuff,bool Bool) {
	if (Bool) {
		rBuff.append(1,1);
	}else{
		rBuff.append(1,0);
	}
}

void NetworkOut::addShort(string& rBuff,short Short) {
	rBuff.append(1, char(Short>>8)); 
	rBuff.append(1, char(Short));
}


void NetworkOut::addInt(string& rBuff,int iInt) {
	char EndianBuffer[4];
	iInt = Poco::ByteOrder::toBigEndian(iInt);
	memcpy(EndianBuffer,&iInt,4);
	rBuff.append(EndianBuffer,4);
}

void NetworkOut::addInt64(string& rBuff,long long iInt) {
	char EndianBuffer[8];
	iInt = Poco::ByteOrder::toBigEndian(Poco::Int64(iInt));
	memcpy(EndianBuffer,&iInt,8);
	rBuff.append(EndianBuffer,8);
}

void NetworkOut::addFloat(string& rBuff,float dVal) {
	int iBuff;
	char EndianBuffer[4];
	memcpy(&iBuff,&dVal,4);
	iBuff = Poco::ByteOrder::toBigEndian(iBuff);
	memcpy(EndianBuffer,&iBuff,4);
	rBuff.append(EndianBuffer,4);
}


void NetworkOut::addDouble(string& rBuff,double dVal) {
	long long iBuff;
	char EndianBuffer[8];

	memcpy(&iBuff,&dVal,8); //copy double to an int64
	iBuff = Poco::ByteOrder::toBigEndian(Poco::Int64(iBuff)); //switch endian
	memcpy(EndianBuffer,&iBuff,8);//copy to endian buffer
	rBuff.append(EndianBuffer,8);//append
}

void NetworkOut::addString(string& rBuff,string& sString) {
	short iDataLength;

	iDataLength = sString.length();
	addShort(rBuff,iDataLength);

	if ( sString.length() == 0) {return;}

	for (int x=0;x<=sString.length()-1;x++) {
		rBuff.append(1,0);
		rBuff.append(sString,x,1);
	}	
}

void NetworkOut::addByte(unsigned char Byte) {
	_pNetworkBuffer->append(1,Byte);
}

void NetworkOut::addBool(bool Bool) {
	if (Bool) {
		_pNetworkBuffer->append(1,1);
	}else{
		_pNetworkBuffer->append(1,0);
	}
}

void NetworkOut::addShort(short Short) {
	_pNetworkBuffer->append(1, char(Short>>8)); 
	_pNetworkBuffer->append(1, char(Short));
}


void NetworkOut::addInt(int iInt) {
	iInt = Poco::ByteOrder::toBigEndian(iInt);
	memcpy(_sEndianBuffer,&iInt,4);
	_pNetworkBuffer->append(_sEndianBuffer,4);
}

void NetworkOut::addInt64(long long iInt) {
	iInt = Poco::ByteOrder::toBigEndian(Poco::Int64(iInt));
	memcpy(_sEndianBuffer,&iInt,8);
	_pNetworkBuffer->append(_sEndianBuffer,8);
}

void NetworkOut::addFloat(float dVal) {
	_ItF.d = dVal;
	_ItF.i = Poco::ByteOrder::toBigEndian(_ItF.i);
	memcpy(_sEndianBuffer,&_ItF.i,4);
	_pNetworkBuffer->append(_sEndianBuffer,4);
}


void NetworkOut::addDouble(double dVal) {
	_ItD.d = dVal;
	_ItD.i = Poco::ByteOrder::toBigEndian((Poco::Int64)_ItD.i); //switch endian
	memcpy(_sEndianBuffer,&_ItD.i,8);//copy to endian buffer
	_pNetworkBuffer->append(_sEndianBuffer,8);//append
}

void NetworkOut::addString(string& sString) {
	short iDataLength;
	iDataLength = sString.length();
	addShort(iDataLength);

	if (sString.empty()) {return;}

	for (int x=0;x<=sString.length()-1;x++) {
		_pNetworkBuffer->append(1,0);
		_pNetworkBuffer->append(1,sString[x]);
	}
}

string& NetworkOut::getStr() {
	return *_pNetworkBuffer;
}

void NetworkOut::Finalize(char iType) {
	if (iType != FC_QUEUE_LOW && iType != FC_QUEUE_HIGH) {throw FCException("Unknown queue type");}
	if (_pNetworkBuffer->empty()) {return;}
	if (!_pMaster->_pPlayer->isAssigned()) {throw FCException("Unable to encode data",false);}
	
	try {
		_pMaster->Add(iType,_pNetworkBuffer);
		_pNetworkBuffer = new string("");
	}catch(FCException &ex) {
		ex.rethrow();
	}
}

void NetworkOut::addByteArray(string& rTarget,std::pair<char*,short>& rData) {
	if (rData.first == NULL) {throw FCException("Nullpointer");}
	addShort(rTarget,rData.second);
	rTarget.append(rData.first,rData.second);	
}


void NetworkOut::addByteArray(std::pair<char*,short>& rData) {
	try {
		addByteArray(*_pNetworkBuffer,rData);
	}catch(FCException& ex) { 
		ex.rethrow();
	}
}
