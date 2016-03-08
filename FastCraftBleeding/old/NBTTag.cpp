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
#include "NBTTag.h"
#include "FCRuntimeException.h"

NBTTag::NBTTag(char iType,string sName) {
	if (iType < 1 || iType > 11) {throw FCRuntimeException("Illegal tag type");}
	_sName.assign(sName);
	_iTagType = iType;
}

NBTTag::~NBTTag() {
}

string NBTTag::getName() {
	return _sName;
}

char NBTTag::getTagType() { 
	return _iTagType;
}