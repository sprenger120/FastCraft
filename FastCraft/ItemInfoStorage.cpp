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

#include "ItemInfoStorage.h"
#include <Poco/Exception.h>
#include <Poco/String.h>
#include <Poco/Path.h>
#include <Poco/File.h>
using Poco::RuntimeException;
using std::cout;
using std::string;
using Poco::Data::into;
using Poco::Data::now;
using Poco::Data::use;

ItemInfoStorage::ItemInfoStorage(){
}

ItemInfoStorage::~ItemInfoStorage() {
}

vector<ItemEntry> ItemInfoStorage::_vItems(0);
vector<BlockEntry>ItemInfoStorage::_vBlocks(0);
Poco::Path ItemInfoStorage::_workingDirectory;
bool ItemInfoStorage::_fDatabasesLoaded(false);

void ItemInfoStorage::loadDatabases(Poco::Path Path) {
	if (_fDatabasesLoaded) {
		cout<<"You cannot call ItemInfoStorage::loadDatabases more than one"<<"\n";
		refreshCache();
		return;
	}

	_workingDirectory = Path;
	_fDatabasesLoaded = true;


	refreshCache();
}

void ItemInfoStorage::refreshCache() {
	if (!_fDatabasesLoaded) {
		cout<<"ItemInfoStorage::refreshCache no path specified"<<"\n";
		throw Poco::RuntimeException("no path specified");
	}
	Poco::File workDir(_workingDirectory.toString());
	vector<string> vFiles;
	int iLoadedDatabases=0;

	cout<<"Loading item databases... "<<"\n";

	//Clear vectors
	_vItems.clear();
	_vBlocks.clear();

	//List files
	workDir.list(vFiles);

	if (!vFiles.empty()) {
		for (int x=0;x<=vFiles.size()-1;x++) {
			{
				_workingDirectory.setFileName(vFiles[x]);

				Poco::File tempFile(_workingDirectory.toString());
				if (!tempFile.exists() || !tempFile.canRead() || _workingDirectory.getExtension().compare("db") != 0) {
					cout<<"Skipping: "<<_workingDirectory.getFileName()<<"\n";
					continue;
				}

				Poco::Data::Session DB("SQLite", _workingDirectory.toString());
				cout<<"Loading: "<<vFiles[x]<<"\n";
				loadSingleDatabase(DB);

				DB.close();

				iLoadedDatabases++;
			}
		}
	}
	if (iLoadedDatabases==0) {
		cout<<"No databases found!"<<"\n";
	}
	cout<<"Done. (Loaded Entries:"<<ItemInfoStorage::getItemsInCache() + ItemInfoStorage::getBlocksInCache()<<")"<<"\n";
}

void ItemInfoStorage::loadSingleDatabase(Poco::Data::Session& rDB) {
	int iItemCount = 0,iBlockCount=0,x;
	ItemEntry IEntry;
	BlockEntry BEntry;

	rDB<<"SELECT count(`ID`) FROM `Items`",into(iItemCount),now; //Fetch item count
	rDB<<"SELECT count(`ID`) FROM `Blocks`",into(iBlockCount),now; //Fetch block count

	if (iBlockCount>0) {
		for (x=0;x<=iBlockCount-1;x++) {
			rDB<<"SELECT ID,SubID,Name,MaxStackSize,SelfLightLevel,Flammable,Solid,NeededTool,ToolLevel,Thickness,Height,Stackable,NeedWallOrFloorConnection FROM Blocks LIMIT :x,1;",
				into(BEntry.ID),
				into(BEntry.SubID),
				into(BEntry.Name),
				into(BEntry.MaxStackSize),
				into(BEntry.SelfLightLevel),
				into(BEntry.Flammable),
				into(BEntry.Solid),
				into(BEntry.NeededTool),
				into(BEntry.ToolLevel),
				into(BEntry.Thickness),
				into(BEntry.Height),
				into(BEntry.Stackable),
				into(BEntry.NeedWallOrFloorConnection),
				use(x),
				now;

			try {
				isValid(BEntry);
			}catch(Poco::RuntimeException& ex) {
				cout<<"Block #"<<int(BEntry.ID)<<":"<<int(BEntry.SubID)<<" is invalid ("<<ex.message()<<")"<<"\n";
				continue;
			}
			_vBlocks.push_back(BEntry);
		}
	}

	if (iItemCount>0) {
		for (x=0;x<=iItemCount-1;x++) {
			rDB<<"SELECT ID,SubID,Name,Damageable,Enchantable,Durability,MaxStackSize,Eatable,ConnectedBlock,Weapon FROM Items LIMIT :x,1;",
				into(IEntry.ID),
				into(IEntry.SubID),
				into(IEntry.Name),
				into(IEntry.Damageable),
				into(IEntry.Enchantable),
				into(IEntry.Durability),
				into(IEntry.MaxStackSize),
				into(IEntry.Eatable),
				into(IEntry.ConnectedBlock),
				into(IEntry.Weapon),
				use(x),
				now;

			try {
				isValid(IEntry);
			}catch(Poco::RuntimeException& ex) {
				cout<<"Item #"<<IEntry.ID<<":"<<int(IEntry.SubID)<<" is invalid ("<<ex.message()<<")"<<"\n";
				continue;
			}
			_vItems.push_back(IEntry);
		}
	}
}

string ItemInfoStorage::getName(ItemID ID) {
	int x;

	//Loop through blocks
	if (!_vBlocks.empty() && isBlock(ID)) {
		for (x=0;x<=_vBlocks.size()-1;x++) {
			if (_vBlocks[x].ID == ID.first && _vBlocks[x].SubID == ID.second) {
				return _vBlocks[x].Name;
			}
		}
	}

	//Loop through items
	if (!_vItems.empty() && !isBlock(ID)) {
		for (x=0;x<=_vItems.size()-1;x++) {
			if (_vItems[x].ID == ID.first && _vItems[x].SubID == ID.second) {
				return _vItems[x].Name;;
			}
		}
	}
	throw Poco::RuntimeException("Not found!");
}


string ItemInfoStorage::getName(short ID) {
	int x;

	//Loop through blocks
	if (!_vBlocks.empty() && isBlock(ID)) {
		for (x=0;x<=_vBlocks.size()-1;x++) {
			if (_vBlocks[x].ID == ID && _vBlocks[x].SubID == 0) {
				return _vBlocks[x].Name;
			}
		}
	}

	//Loop through items
	if (!_vItems.empty() && !isBlock(ID)) {
		for (x=0;x<=_vItems.size()-1;x++) {
			if (_vItems[x].ID == ID && _vItems[x].SubID == 0) {
				return _vItems[x].Name;
			}
		}
	}

	throw Poco::RuntimeException("Not found!");
}


bool ItemInfoStorage::isRegistered(ItemID ID) {
	int x;

	//Loop through blocks
	if (!_vBlocks.empty() && isBlock(ID)) {
		for (x=0;x<=_vBlocks.size()-1;x++) {
			if (_vBlocks[x].ID == ID.first && _vBlocks[x].SubID == ID.second) {
				return true;
			}
		}
	}

	//Loop through items
	if (!_vItems.empty() && !isBlock(ID)) {
		for (x=0;x<=_vItems.size()-1;x++) {
			if (_vItems[x].ID == ID.first && _vItems[x].SubID == ID.second) {
				return true;
			}
		}
	}

	return false;
}

bool ItemInfoStorage::isRegistered(short ID) {
	int x;

	//Loop through blocks
	if (!_vBlocks.empty() && isBlock(ID)) {
		for (x=0;x<=_vBlocks.size()-1;x++) {
			if (_vBlocks[x].ID == ID  && _vBlocks[x].SubID == 0) {
				return true;
			}
		}
	}

	//Loop through items
	if (!_vItems.empty() && !isBlock(ID)) {
		for (x=0;x<=_vItems.size()-1;x++) {
			if (_vItems[x].ID == ID && _vItems[x].SubID == 0) {
				return true;
			}
		}
	}

	return false;
}

ItemID ItemInfoStorage::getIDbyName(string Name) {
	ItemID id;
	int x;

	//Loop through blocks
	if (!_vBlocks.empty()) {
		for (x=0;x<=_vBlocks.size()-1;x++) {
			if (Poco::icompare(_vBlocks[x].Name,Name) == 0) {
				id.first = _vBlocks[x].ID;
				id.second = _vBlocks[x].SubID;
				return id;
			}
		}
	}

	//Loop through items
	if (!_vItems.empty()) {
		for (x=0;x<=_vItems.size()-1;x++) {
			if (Poco::icompare(_vItems[x].Name,Name) == 0) {
				id.first = _vItems[x].ID;
				id.second = _vItems[x].SubID;
				return id;
			}
		}
	}

	throw Poco::RuntimeException("Not found!");
}

bool ItemInfoStorage::isBlock(ItemID ID) {
	if (ID.first >= 0 && ID.first <= 255) {
		return true;
	}else{
		return false;
	}
}

bool ItemInfoStorage::isBlock(short iID) {
	if (iID >= 0 && iID <= 255) {
		return true;
	}else{
		return false;
	}
}


BlockEntry ItemInfoStorage::getBlock(ItemID ID) {
	if (_vBlocks.empty()) { throw Poco::RuntimeException("Not found!"); }
	if (!isBlock(ID)) { throw Poco::RuntimeException("Not a block!"); }

	for (int x=0;x<=_vBlocks.size()-1;x++) {
		if (_vBlocks[x].ID == ID.first && _vBlocks[x].SubID == ID.second) {
			return _vBlocks[x];
		}
	}

	throw Poco::RuntimeException("Not found!");
}

BlockEntry ItemInfoStorage::getBlock(short iID) {
	if (_vBlocks.empty()) { throw Poco::RuntimeException("Not found!"); }
	if (!isBlock(iID)) { throw Poco::RuntimeException("Not a block!"); }

	for (int x=0;x<=_vBlocks.size()-1;x++) {
		if (_vBlocks[x].ID == iID && _vBlocks[x].SubID == 0) {
			return _vBlocks[x];
		}
	}
	throw Poco::RuntimeException("Not found!");
}

ItemEntry ItemInfoStorage::getItem(ItemID ID) {
	if (_vItems.empty()) { throw Poco::RuntimeException("Not found!"); }
	if (isBlock(ID)) { throw Poco::RuntimeException("Not a item!"); }

	for (int x=0;x<=_vItems.size()-1;x++) {
		if (_vItems[x].ID == ID.first && _vItems[x].SubID == ID.second) {
			return _vItems[x];
		}
	}
	throw Poco::RuntimeException("Not found!");
}

ItemEntry ItemInfoStorage::getItem(short iID) {
	if (_vItems.empty()) { throw Poco::RuntimeException("Not found!"); }
	if (isBlock(iID)) { throw Poco::RuntimeException("Not a item!"); }

	for (int x=0;x<=_vItems.size()-1;x++) {
		if (_vItems[x].ID == iID && _vItems[x].SubID == 0) {
			return _vItems[x];
		}
	}
	throw Poco::RuntimeException("Not found!");
}

bool ItemInfoStorage::isDamageable(ItemID ID) {
	if (isBlock(ID))  {return false;}
	bool f;
	try {
		f = getItem(ID).Damageable;
	}catch(Poco::RuntimeException& ex) {
		ex.rethrow();
	}
	return f;
}

bool ItemInfoStorage::isDamageable(short ID) {
	if (isBlock(ID))  {return false;}
	bool f;
	try {
		f = getItem(ID).Damageable;
	}catch(Poco::RuntimeException& ex) {
		ex.rethrow();
	}
	return f;
}

bool ItemInfoStorage::isEnchantable(ItemID ID) {
	if (isBlock(ID))  {return false;}
	bool f;
	try {
		f = getItem(ID).Enchantable;
	}catch(Poco::RuntimeException& ex) {
		ex.rethrow();
	}
	return f;
}

bool ItemInfoStorage::isEnchantable(short ID) {
	if (isBlock(ID))  {return false;}
	bool f;
	try {
		f = getItem(ID).Enchantable;
	}catch(Poco::RuntimeException& ex) {
		ex.rethrow();
	}
	return f;
}

bool ItemInfoStorage::isFlammable(ItemID ID) {
	if (!isBlock(ID))  {return true;}
	bool f;
	try {
		f = getBlock(ID).Flammable;
	}catch(Poco::RuntimeException& ex) {
		ex.rethrow();
	}
	return f;
}

bool ItemInfoStorage::isFlammable(short ID) {
	if (!isBlock(ID))  {return true;}
	bool f;
	try {
		f = getBlock(ID).Flammable;
	}catch(Poco::RuntimeException& ex) {
		ex.rethrow();
	}
	return f;
}

bool ItemInfoStorage::isSolid(ItemID ID) {
	if (!isBlock(ID))  {return false;}
	bool f;
	try {
		f = getBlock(ID).Solid;
	}catch(Poco::RuntimeException& ex) {
		ex.rethrow();
	}
	return f;
}

bool ItemInfoStorage::isSolid(short ID) {
	if (!isBlock(ID))  {return false;}
	bool f;
	try {
		f = getBlock(ID).Solid;
	}catch(Poco::RuntimeException& ex) {
		ex.rethrow();
	}
	return f;
}

char ItemInfoStorage::getMaxStackSize(ItemID ID) {
	char i;
	switch(isBlock(ID.first)) {
	case true:
		try {
			i = getBlock(ID).MaxStackSize;
		}catch(Poco::RuntimeException& ex) {
			ex.rethrow();
		}
		break;
	case false:
		try {
			i =  getItem(ID).MaxStackSize;
		}catch(Poco::RuntimeException& ex) {
			ex.rethrow();
		}
	}
	return i;
}

char ItemInfoStorage::getMaxStackSize(short ID) {
	char i;
	switch(isBlock(ID)) {
	case true:
		try {
			i =  getBlock(ID).MaxStackSize;
		}catch(Poco::RuntimeException& ex) {
			ex.rethrow();
		}
		break;
	case false:
		try {
			i =  getItem(ID).MaxStackSize;
		}catch(Poco::RuntimeException& ex) {
			ex.rethrow();
		}
	}
	return i;
}

short ItemInfoStorage::getDurability(ItemID ID) {
	short i;
	if (isBlock(ID.first))  {return -1;}
	try {
		i =  getItem(ID).Durability;
	}catch(Poco::RuntimeException& ex) {
		ex.rethrow();
	}
	return i;
}

short ItemInfoStorage::getDurability(short ID) {
	if (isBlock(ID))  {return -1;}
	short i;
	try {
		i =  getItem(ID).Durability;
	}catch(Poco::RuntimeException& ex) {
		ex.rethrow();
	}
	return i;
}

int ItemInfoStorage::getItemsInCache() {
	return _vItems.size();
}

int ItemInfoStorage::getBlocksInCache() {
	return _vBlocks.size();
}

void ItemInfoStorage::isValid(ItemEntry Entry) {
	if (Entry.Enchantable && !Entry.Damageable) {
		throw Poco::RuntimeException("Enchantable but undestroyable");
	}

	if (Entry.Durability < -1 || Entry.Durability == 0) {
		throw Poco::RuntimeException("Illegal durability");
	}

	if (Entry.Damageable && Entry.Durability==-1) {
		throw Poco::RuntimeException("Destroyable but no durability");
	}

	if (Entry.MaxStackSize <= 0 || Entry.MaxStackSize > 127) {
		throw Poco::RuntimeException("Illegal stack size");
	}

	if (Entry.Damageable && Entry.MaxStackSize != 1) {
		throw Poco::RuntimeException("Destroyable but MaxStackSize > 1");
	}

	if (Entry.Damageable && Entry.Eatable) {
		throw Poco::RuntimeException("Cou can't make tools eatable");
	}

	if (Entry.Weapon && !Entry.Damageable) {
		throw Poco::RuntimeException("Indestructible weapon");
	}

	if (Entry.SubID > 17 || Entry.SubID < 0) {
		throw Poco::RuntimeException("Illegal SubID");
	}

	if (Entry.ID < 256) {
		throw Poco::RuntimeException("ID illegal or in block range");
	}

	if (Entry.Name.compare("") == 0) {
		throw Poco::RuntimeException("Name is empty");
	}


	int x,i;
	if (!_vItems.empty()) {
		bool found = false;

		//Check ConnectedBlock existance
		if (Entry.ConnectedBlock != 0) {
			if (!_vBlocks.empty()) {
				for (x=0;x<=_vBlocks.size()-1;x++) {
					if (_vBlocks[x].ID == Entry.ConnectedBlock) {
						found=true;
						break;
					}
				}
			}
			if (!found){
				throw Poco::RuntimeException("Connected Block doesn't exist");
			}
		}
	

		//Check ID & name availability
		for (x=0;x<=_vItems.size()-1;x++) {
			if (_vItems[x].ID == Entry.ID) { //ID is already taken.. lets check SubID
				for (i=0;i<=_vItems.size()-1;i++) {
					if (_vItems[i].ID == Entry.ID && _vItems[i].SubID == Entry.SubID) {
						throw Poco::RuntimeException("ID already taken");
					}
				}
			}
			if ( Poco::icompare(_vItems[x].Name,Entry.Name) == 0) {
				throw Poco::RuntimeException("Name already taken");
			}
		}
	}
}

void ItemInfoStorage::isValid(BlockEntry Entry) {
	if (Entry.SubID > 17 || Entry.SubID < 0) {
		throw Poco::RuntimeException("Illegal SubID");
	}

	if (Entry.ID > 255) {
		throw Poco::RuntimeException("ID illegal or in item range");
	}

	if (Entry.Name.compare("") == 0) {
		throw Poco::RuntimeException("Name is empty");
	}

	if (Entry.MaxStackSize <= 0 || Entry.MaxStackSize > 127) {
		throw Poco::RuntimeException("Illegal stack size");
	}

	if (Entry.SelfLightLevel > 17 || Entry.SelfLightLevel < 0) {
		throw Poco::RuntimeException("Illegal SelfLightLevel");
	}

	//ToDo: tool check

	if (Entry.Thickness < 0.0F || Entry.Thickness > 1.0F) {
		throw Poco::RuntimeException("Illegal thickness");
	}

	if (Entry.Height < 0.0F || Entry.Height > 1.0F) {
		throw Poco::RuntimeException("Illegal height");
	}

	//Check ID & name availability
	int x,i;
	if (!_vItems.empty()) {
		for (x=0;x<=_vItems.size()-1;x++) {
			if (_vItems[x].ID == Entry.ID) { //ID is already taken.. lets check SubID
				for (i=0;i<=_vItems.size()-1;i++) {
					if (_vItems[i].ID == Entry.ID && _vItems[i].SubID == Entry.SubID) {
						throw Poco::RuntimeException("ID already taken");
					}
				}
			}
			if ( Poco::icompare(_vItems[x].Name,Entry.Name) == 0) {
				throw Poco::RuntimeException("Name already taken");
			}
		}
	}
}