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
#include "NBTTagShort.h"
#include "NBTConstants.h"
#include "NetworkOut.h"

NBTTagShort::NBTTagShort(string sName) :
NBTTagBase(sName,FC_NBT_TYPE_SHORT)
{
	iData = 0;
}

NBTTagShort::~NBTTagShort() {
}

void NBTTagShort::write(string& rTarget,bool fMode,bool fHeaderless) {
	if (!fHeaderless) {
		rTarget.append<char>(1,FC_NBT_TYPE_SHORT); //Tag Type
		addHeaderlessString(rTarget,_sName); //Name
	} 
	NetworkOut::addShort(rTarget,iData);
}

short& NBTTagShort::getDataRef() {
	return iData;
}