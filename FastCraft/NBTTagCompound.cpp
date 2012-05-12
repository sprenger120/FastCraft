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
#include "FCRuntimeException.h"

NBTTagCompound::NBTTagCompound(string sName) :
	NBTTagBase(sName,FC_NBT_TYPE_COMPOUND),
	_vpElements(0)
{
}

NBTTagCompound::~NBTTagCompound(){
	if (!_vpElements.empty()) {
		for (int x=0;x<=_vpElements.size()-1;x++) {
			delete _vpElements[x];
		}
	}
	_vpElements.clear();
}

void NBTTagCompound::addSubElement(NBTTagBase* pNew) {
	if(pNew->getName().compare("") == 0) {throw FCRuntimeException("Name is empty");}
	if (getElementIndex(pNew->getName()) != -1) {throw FCRuntimeException("Name already taken!");}
	_vpElements.push_back(pNew);
}

void NBTTagCompound::removeSubElement(string sName) {
	int index = getElementIndex(sName);
	if (index == -1) {throw FCRuntimeException("Not found!");}
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

NBTTagBase* NBTTagCompound::search(string sPath, char iType) {
	if (iType < 1 || iType > 10) {throw FCRuntimeException("Invalid tag type"); }
	if (sPath.empty() || sPath[0] != '/') { throw FCRuntimeException("Invalid Path"); }
	
	vector<string> aPathElements(0);
	string sTemp("");
	int iStartSlash=0,iEndSlash;

	//Prepare path
	while (sPath[sPath.size()-1] == '/' && sPath[sPath.size()-2] == '/') {sPath.resize(sPath.size()-1);} //Remove double slashes from end
	if (sPath[sPath.size()-1] != '/') {sPath.push_back('/');} //add a slash if there aren't one at the end

	//Split path 
	while(1) {
		//Search end slash
		for(iEndSlash = iStartSlash+1;iEndSlash<=sPath.size()-1;iEndSlash++) {
			if (sPath[iEndSlash] == '/') {  
				break;
			}
		}
		sTemp.assign(sPath,iStartSlash+1,iEndSlash-iStartSlash-1);
		aPathElements.push_back(sTemp);

		if (iEndSlash == sPath.size() -1) {break;}
		iStartSlash = iEndSlash;
	}

	int iVecIndex = 0;
	NBTTagBase*			pLastElement = NULL;
	NBTTagCompound*		pLastCompound = this;
	
	while (1) {
		pLastElement = pLastCompound->getElementByName(aPathElements[iVecIndex]);
		if (pLastElement==NULL) {throw FCRuntimeException("Not found");}


		if (iVecIndex == aPathElements.size()-1) {
			if (pLastElement->getTagType() != iType) {throw FCRuntimeException("Not found");}
			break;
		}else{
			if (pLastElement->getTagType() != FC_NBT_TYPE_COMPOUND) {throw FCRuntimeException("Not found");}
			pLastCompound = (NBTTagCompound*)pLastElement;
			iVecIndex++;
		}
	}
	return pLastElement;
}


NBTTagBase* NBTTagCompound::getElementByName(string sName) {
	if (_vpElements.empty()) { return NULL;}
	for (int x=0;x<=_vpElements.size()-1;x++) {
		if (_vpElements[x]->getName().compare(sName) == 0) {return  _vpElements[x];}
	}
	return NULL;
}
