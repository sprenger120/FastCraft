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
#include "NBTTagByteArray.h"
#include "NBTConstants.h"
#include "NetworkOut.h"
#include <Poco/Exception.h>

NBTTagByteArray::NBTTagByteArray(string sName) :
NBTTagBase(sName,FC_NBT_TYPE_BYTEARRAY),
sData("")
{
}

NBTTagByteArray::~NBTTagByteArray() {
}

void NBTTagByteArray::write(string& rTarget,bool fMode,bool fHeaderless) {
	if (!fHeaderless) {
		rTarget.append<char>(1,FC_NBT_TYPE_BYTEARRAY); //Tag Type
		addHeaderlessString(rTarget,_sName);//Name
	} 
	addHeaderlessString(rTarget,sData); //Data
}

string& NBTTagByteArray::getDataRef() {
	return sData;
}

char NBTTagByteArray::operator[] (int index) {
	if (index < 0 || index > sData.length()) { throw Poco::RuntimeException("Index out of bound!"); }
	return sData.at(index);
}