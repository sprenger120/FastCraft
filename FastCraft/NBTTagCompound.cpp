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
#include "NBTTagCompound.h"
#include "NBTConstants.h"
#include <Poco/Exception.h>

NBTTagCompound::NBTTagCompound(string sName) :
	NBTTagBase(sName,FC_NBT_TYPE_COMPOUND),
	_vpElements(0)
{
}

NBTTagCompound::~NBTTagCompound(){
	_vpElements.clear();
}

void NBTTagCompound::addSubElement(NBTTagBase* pNew) {
	if(pNew->getName().compare("") == 0) {throw Poco::RuntimeException("Name is empty");}
	if (getElementIndex(pNew->getName()) != -1) {throw Poco::RuntimeException("Name already taken!");}
	_vpElements.push_back(pNew);
}

void NBTTagCompound::removeSubElement(string sName) {
	int index = getElementIndex(sName);
	if (index == -1) {throw Poco::RuntimeException("Not found!");}
	_vpElements.erase(_vpElements.begin()+index);
}

void NBTTagCompound::write(string& rStr,bool fMode,bool fHeaderless) {
	string sTemp("");
	string& rTarget = ( fMode == FC_NBT_OUTPUT_RAW ? rStr : sTemp);
	
	if (!fHeaderless) {
		rTarget.append<char>(1,FC_NBT_TYPE_COMPOUND);
		addHeaderlessString(rTarget,_sName);//Name 
	} 

	if (!_vpElements.empty()) {
		for (int x=0;x<=_vpElements.size()-1;x++){
			_vpElements[x]->write(rTarget,FC_NBT_OUTPUT_RAW);
		}
	}

	rTarget.append<char>(1,0);
	if(fMode == FC_NBT_OUTPUT_GZIP) { 
		NBTTagBase::compress(rTarget); 
		rStr.assign(rTarget);
	}
}

int NBTTagCompound::getElementIndex(string sName) {
	if (_vpElements.empty()) {return -1;}

	for (int x=0;x<=_vpElements.size()-1;x++) {
		if (_vpElements[x]->getName().compare(sName) == 0) {return x;}
	}
	return -1;
}