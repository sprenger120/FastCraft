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
#include "TextHandler.h"
#include <Poco\ByteOrder.h>
#include <string>


void TextHandler::Append(std::string & Input,signed long long iInt,bool fBigEndian) {
	char _sBuffer[8];
	if (fBigEndian) {iInt = Poco::ByteOrder::toBigEndian(iInt); }
	std::memcpy(_sBuffer,&iInt,8);
	Input.append(_sBuffer,8);
}

void TextHandler::Append(std::string & Input,short iInt,bool fBigEndian) {
	char _sBuffer[2];
	if (fBigEndian) {iInt = Poco::ByteOrder::toBigEndian(iInt); }
	std::memcpy(_sBuffer,&iInt,2);
	Input.append(_sBuffer,2);
}


void TextHandler::Append(std::string & Input,int iInt,bool fBigEndian) {
	char _sBuffer[4];
	if (fBigEndian) {iInt = Poco::ByteOrder::toBigEndian(iInt); }
	std::memcpy(_sBuffer,&iInt,4);
	Input.append(_sBuffer,4);
}

void TextHandler::Append(std::string & Input,double dVal,bool fBigEndian) {
	char _sBuffer[8];
	long long iTemp;
	if (fBigEndian) {
		std::memcpy(&iTemp,&dVal,8); //Copy to long long
		iTemp = Poco::ByteOrder::toBigEndian(iTemp);  //BigEndian
		std::memcpy(_sBuffer,&iTemp,8); //copy to buffer
	}else{
	std::memcpy(_sBuffer,&dVal,8);
	}
	Input.append(_sBuffer,8);
}

void TextHandler::Append(std::string & Input,float dVal,bool fBigEndian) {
	char _sBuffer[4];
	int iTemp;
	if (fBigEndian) {
		std::memcpy(&iTemp,&dVal,4); //Copy to long long
		iTemp = Poco::ByteOrder::toBigEndian(iTemp);  //BigEndian
		std::memcpy(_sBuffer,&iTemp,4); //copy to buffer
	}else{
	std::memcpy(_sBuffer,&dVal,4);
	}
	Input.append(_sBuffer,4);
}

void TextHandler::packString16(std::string & Output, std::string & Input) {
	short iDataLength;
	char sBuffer[2];
	iDataLength = Input.length();

	iDataLength = Poco::ByteOrder::toBigEndian(iDataLength);
	std::memcpy(sBuffer,&iDataLength,2);
	Output.append(sBuffer,2); //Length Field


	for (int x=0;x<=Input.length()-1;x++) {
		Output.append<int>(1,0);
		Output.append(Input,x,1);
	}	
}
