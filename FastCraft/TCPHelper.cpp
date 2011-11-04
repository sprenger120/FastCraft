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
#include "TCPHelper.h"
#include <Poco/ByteOrder.h>

string TCPHelper::readString16(StreamSocket& Connection) {
	short iLenght;
	char sBuffer[2];
	char * pBuff;
	string sOut("");


	//Read string lenght
	Connection.receiveBytes(sBuffer,2);
	iLenght = short(sBuffer[1]) | (short(sBuffer[0])<<8); //Convert to short

	if (iLenght<=0) {return "";}

	pBuff = new char[iLenght*2]; //Memory for string
	Connection.receiveBytes(pBuff,iLenght*2); //Recv

	for (int x = 1;x<=iLenght*2-1;x+=2) { 
		sOut.append(1,pBuff[x]);
	}

	delete [] pBuff; //Delete buffer

	return sOut;
}


int TCPHelper::readInt(StreamSocket& Connection) {
	char sBuffer[4];

	Connection.receiveBytes(sBuffer,4); //ReadData


	return int(sBuffer[3]) | (int(sBuffer[2])<<8) | (int(sBuffer[1])<<16) | (int(sBuffer[0])<<24);
}
/*
short TCPHelper::readShort(StreamSocket& Connection) {


}


char TCPHelper::readByte(StreamSocket& Connection) {


}
*/