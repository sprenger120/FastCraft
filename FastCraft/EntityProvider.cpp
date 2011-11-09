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
#include "EntityProvider.h"

EntityProvider::EntityProvider():
_vEntitys(0)
{
	_iCount=0;
}

EntityProvider::~EntityProvider() {
}

int EntityProvider::Add(char iType) {
	int iIndex = _vEntitys.size();
	_iCount++;

	if (iIndex > 0) {
		for(unsigned int x=0;x <= _vEntitys.size()-1;x++) {
			if (_vEntitys[x].EntityID == 0) { //Free place
				_vEntitys[x].EntityID = _iCount;
				_vEntitys[x].Type = iType;
				return _iCount;
			}
		}
	}

	_vEntitys.resize(iIndex + 1);

	_vEntitys[iIndex].EntityID = _iCount;
	_vEntitys[iIndex].Type = iType;
	return _iCount;
}

void EntityProvider::Remove(int iEntityID) {
	if (_vEntitys.size() > 0) {
		for(unsigned int x=0;x <= _vEntitys.size()-1;x++) {
			if (_vEntitys[x].EntityID == iEntityID) {
				_vEntitys[x].EntityID = 0;
				_vEntitys[x].Type = 0;
			}
		}
	}
}

char EntityProvider::getType(int iEntityID) {
	if (_vEntitys.size() > 0) {
		for(unsigned int x=0;x <= _vEntitys.size()-1;x++) {
			if (_vEntitys[x].EntityID == iEntityID) {
				return _vEntitys[x].Type;
			}
		}
		return 0;
	}else{
		return 0;
	}
}