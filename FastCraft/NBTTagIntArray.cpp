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
#include "NBTTagIntArray.h"
#include "FCRuntimeException.h"
#include "NBTConstants.h"
#include "NetworkOut.h"

NBTTagIntArray::NBTTagIntArray(string sName) : 
	NBTTagBase(sName,FC_NBT_TYPE_INTARRAY)
{
}

void NBTTagIntArray::write(string& rStr,char iType,bool fHeaderless)  {
	string sTemp("");
	string& rTarget = ( iType == FC_NBT_IO_RAW ? rStr : sTemp);
	int iSize = _vpElements.size();

	if (!fHeaderless) {
		rTarget.append(1,FC_NBT_TYPE_INTARRAY);
		addHeaderlessString(rTarget,_sName); //Name
	} 
	NetworkOut::addInt(rTarget,_vpElements.size());

	if (iSize > 0) {
		for (int i = 0; i<= iSize-1;i++) {
			NetworkOut::addInt(rTarget,_vpElements[i]);
		}
	}

	if(iType != FC_NBT_IO_RAW) {
		try {
			NBTTagBase::compress(rTarget,iType); 
		}catch(FCRuntimeException& ex) {
			ex.rethrow();
		}
		rStr.assign(rTarget);
	}
}

int& NBTTagIntArray::operator[] (int index) {
	int iSize = _vpElements.size();
	if (iSize == 0 || index < 0 || iSize-1 < index) { throw FCRuntimeException("Invalid index"); }
	return _vpElements[index];
}

int NBTTagIntArray::size() {
	return _vpElements.size();
}

bool NBTTagIntArray::isEmpty() { 
	return	_vpElements.empty();
}

void NBTTagIntArray::erase(int index) {
	int iSize = _vpElements.size();
	if (iSize == 0 || index < 0 || iSize-1 < index) { throw FCRuntimeException("Invalid index"); }

	_vpElements.erase(_vpElements.begin()+index);
}

void NBTTagIntArray::addSubElement(int i) {
	_vpElements.push_back(i);
}