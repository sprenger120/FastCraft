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
#include "NBTTagList.h"
#include "NBTConstants.h"
#include "NetworkOut.h"
#include <Poco/Exception.h>

NBTTagList::NBTTagList(string sName,char iListType) : 
NBTTagBase(sName,FC_NBT_TYPE_LIST),
	_iElementType(iListType),
	_vpElements(0)
{
	if (iListType < 1 || iListType > 10) {throw Poco::RuntimeException("Invalid tag type"); }
}

NBTTagList::~NBTTagList(){
	if (!_vpElements.empty()) {
		for (int x=0;x<=_vpElements.size()-1;x++) {
			delete _vpElements[x];
		}
	}
	_vpElements.clear();
}

void NBTTagList::write(string& rTarget,bool fMode,bool fHeaderless) {
	int iSize = _vpElements.size();

	if (!fHeaderless) {
		rTarget.append<char>(1,FC_NBT_TYPE_LIST);
		addHeaderlessString(rTarget,_sName); //Name
	} 
	rTarget.append<char>(1,_iElementType);
	NetworkOut::addInt(rTarget,_vpElements.size());

	if (iSize > 0) {
		for (int i = 0; i<= iSize-1;i++) {
			_vpElements[i]->write(rTarget,FC_NBT_OUTPUT_RAW,FC_NBT_FLAG_HEADERLESS);
		}
	}
}



NBTTagBase* NBTTagList::operator[] (int index) {
	int iSize = _vpElements.size();
	if (iSize == 0 || index < 0 || iSize-1 < index) { throw Poco::RuntimeException("Invalid index"); }
	return _vpElements[index];
}

int NBTTagList::size() {
	return _vpElements.size();
}

bool NBTTagList::isEmpty() { 
	return	_vpElements.empty();
}

void NBTTagList::erase(int index) {
	int iSize = _vpElements.size();
	if (iSize == 0 || index < 0 || iSize-1 < index) { throw Poco::RuntimeException("Invalid index"); }

	_vpElements.erase(_vpElements.begin()+index);
}

void NBTTagList::addSubElement(NBTTagBase* pElement) {
	if(pElement->getTagType() != _iElementType) {
		throw Poco::RuntimeException("Type not supported");
	}
	_vpElements.push_back(pElement);
}