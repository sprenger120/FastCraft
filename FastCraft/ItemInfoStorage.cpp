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
		int x,i;
		for (x=0;x<=vFiles.size()-1;x++) {
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

		cout<<"Postprocessing entries...";
		bool fhasSub = false;
		int index;

		if (!_vItems.empty()) {
			for (x = 0;x<=_vItems.size()-1;x++) {
				if (_vItems[x].hasSubItems) {continue;}

				for (i = 1;i<=15;i++) {
					index = search(_vItems,  std::make_pair(_vItems[x].ID,i));
					if (index != -1) {
						fhasSub = true;
						_vItems[index].hasSubItems = true;
					} 
				}

				if (fhasSub) {
					_vItems[x].hasSubItems = true;
					fhasSub=false;
				}
			}
		}

		if (!_vBlocks.empty()) {
			for (x = 0;x<=_vBlocks.size()-1;x++) {
				if (_vBlocks[x].hasSubBlocks) {continue;}

				for (i = 1;i<=15;i++) {
					index = search(_vBlocks,std::make_pair(_vBlocks[x].ID,i));
					if (index != -1) {
						fhasSub = true;
						_vBlocks[index].hasSubBlocks = true;
					} 
				}

				if (fhasSub) {
					_vBlocks[x].hasSubBlocks = true;
					fhasSub=false;
				}
			}
		}

		cout<<" Done.\n";
	}
	if (iLoadedDatabases==0) {
		cout<<"No databases found!"<<"\n";
	}
	cout<<"Done. (Loaded Entries:"<<ItemInfoStorage::getItemsInCache() + ItemInfoStorage::getBlocksInCache()<<")"<<"\n";
}

void ItemInfoStorage::loadSingleDatabase(Poco::Data::Session& rDB) {
	int iItemCount = 0,iBlockCount=0,x=0,i=0,index=0,iCount=0;
	ItemEntry IEntry;
	BlockEntry BEntry;
	ItemID ID;

	rDB<<"SELECT count(`ID`) FROM `Items`",into(iItemCount),now; //Fetch item count
	rDB<<"SELECT count(`ID`) FROM `Blocks`",into(iBlockCount),now; //Fetch block count


	BEntry.Thickness		= FC_IIS_DEFAULT_BLOCK_THICKNESS;
	BEntry.Height			= FC_IIS_DEFAULT_BLOCK_HEIGHT;
	BEntry.Stackable		= FC_IIS_DEFAULT_BLOCK_STACKABLE;
	BEntry.CanFloat			= FC_IIS_DEFAULT_BLOCK_CANFLOAT;
	BEntry.ConnectedItem	= FC_IIS_DEFAULT_BLOCK_CONNECTEDITEM;
	BEntry.BlastResistance  = 1.0F;
	BEntry.hasSubBlocks		= false;
	BEntry.noLoot			= false;
	BEntry.Placeable		= true;

	IEntry.Eatable			= FC_IIS_DEFAULT_ITEM_EATABLE;
	IEntry.ConnectedBlock	= FC_IIS_DEFAULT_ITEM_CONNECTEDBLOCK;
	IEntry.Weapon			= FC_IIS_DEFAULT_ITEM_WEAPON;
	IEntry.Damage			= FC_IIS_DEFAULT_ITEM_WEAPON_DAMAGE;
	IEntry.FoodValue		= FC_IIS_DEFAULT_ITEM_FOODVALUE;
	IEntry.hasSubItems		= false;

	/*
	* Load Item/Blocks
	*/
	cout<<"\t-Reading block IDs...\n";
	if (iBlockCount>0) {
		for (x=0;x<=iBlockCount-1;x++) {
			rDB<<"SELECT ID,SubID,Name,MaxStackSize,SelfLightLevel,Flammable,Solid,NeededTool,ToolLevel FROM Blocks LIMIT :x,1;",
				into(BEntry.ID),
				into(BEntry.SubID),
				into(BEntry.Name),
				into(BEntry.MaxStackSize),
				into(BEntry.SelfLightLevel),
				into(BEntry.Flammable),
				into(BEntry.Solid),
				into(BEntry.NeededTool),
				into(BEntry.ToolLevel),
				use(x),
				now;
			if (search(_vBlocks, std::make_pair(BEntry.ID,BEntry.SubID)) != -1) {
				cout<<"\t\tBlockID "<<int(BEntry.ID)<<":"<<int(BEntry.SubID)<<" already used\n";
				continue;
			}
			_vBlocks.push_back(BEntry);
		}
	}

	cout<<"\t-Reading item IDs...\n";
	if (iItemCount>0) {
		for (x=0;x<=iItemCount-1;x++) {
			rDB<<"SELECT ID,SubID,Name,Damageable,Enchantable,Durability,MaxStackSize FROM Items LIMIT :x,1;",
				into(IEntry.ID),
				into(IEntry.SubID),
				into(IEntry.Name),
				into(IEntry.Damageable),
				into(IEntry.Enchantable),
				into(IEntry.Durability),
				into(IEntry.MaxStackSize),
				use(x),
				now;
			if (search(_vItems, std::make_pair(IEntry.ID,IEntry.SubID)) != -1) {
				cout<<"\t\tItemID "<<IEntry.ID<<":"<<IEntry.SubID<<" already used\n";
				continue;
			}
			_vItems.push_back(IEntry);
		}
	}


	/*
	* Load special per block/item properties
	*/
	cout<<"\t-Loading per block/item properties...\n";


	//Connected Block
	cout<<"\t\t*Reading connected blocks...\n";
	rDB<<"SELECT count(`ID`) FROM `ConnectedBlock`",into(iCount),now;
	if (iCount > 0) {
		ItemID ConnectedBlock;

		for (x=0;x<=iCount-1;x++) {
			rDB<<"SELECT ID,SubID,ConnectedBlockID,ConnectedBlockSubID FROM ConnectedBlock LIMIT :x,1;",
				into(ID.first),
				into(ID.second),
				into(ConnectedBlock.first),
				into(ConnectedBlock.second),
				use(x),
				now;


			index = search(_vItems,ID);

			if (index==-1) {
				cout<<"\t\t\tItem "<<ID.first<<":"<<int(ID.second)<<" not found!\n";
			}else{
				_vItems[index].ConnectedBlock = ConnectedBlock;
			}
		}
	}
	iCount=0;


	//Connected Item
	cout<<"\t\t*Reading connected items...\n";
	rDB<<"SELECT count(`ID`) FROM `ConnectedItem`",into(iCount),now;
	if (iCount > 0) {
		ItemID ConnectedItem;

		for (x=0;x<=iCount-1;x++) {
			rDB<<"SELECT ID,SubID,ConnectedItemID,ConnectedItemSubID FROM ConnectedItem LIMIT :x,1;",
				into(ID.first),
				into(ID.second),
				into(ConnectedItem.first),
				into(ConnectedItem.second),
				use(x),
				now;

			index = search(_vBlocks,ID);

			if (index == -1) {
				cout<<"\t\t\tBlock "<<ID.first<<":"<<int(ID.second)<<" not found!\n";
			}else{
				if (ConnectedItem.first == 0) {_vBlocks[index].noLoot = true; }
				_vBlocks[index].ConnectedItem = ConnectedItem;
			}
		}
	}
	iCount=0;


	//Eatable
	cout<<"\t\t*Reading eatable flags...\n";
	rDB<<"SELECT count(`ID`) FROM `Eatable`",into(iCount),now; 
	if (iCount > 0) {
		char iFoodValue;
		for (x=0;x<=iCount-1;x++) {
			rDB<<"SELECT ID,SubID,FoodValue FROM Eatable LIMIT :x,1;",
				into(ID.first),
				into(ID.second),
				into(iFoodValue),
				use(x),
				now;

			index = search(_vItems,ID);

			if (index == -1) {
				cout<<"\t\t\tItem "<<ID.first<<":"<<int(ID.second)<<" not found!\n";
			}else{
				_vItems[index].Eatable = true;
				_vItems[index].FoodValue = iFoodValue;
			}
		}
	}
	iCount=0;


	//NoFloating
	cout<<"\t\t*Reading noFloating flags...\n";
	rDB<<"SELECT count(`ID`) FROM `NoFloating`",into(iCount),now;
	if (iCount > 0) {
		for (x=0;x<=iCount-1;x++) {
			rDB<<"SELECT ID,SubID FROM NoFloating LIMIT :x,1;",
				into(ID.first),
				into(ID.second),
				use(x),
				now;

			index = search(_vBlocks,ID);

			if (index == -1) {
				cout<<"\t\t\tBlock "<<ID.first<<":"<<int(ID.second)<<" not found!\n";
			}else{
				_vBlocks[index].CanFloat = false;
			}
		}
	}
	iCount=0;


	//Not stackable
	cout<<"\t\t*Reading notStackable flags...\n";
	rDB<<"SELECT count(`ID`) FROM `NotStackable`",into(iCount),now;
	if (iCount > 0) {
		for (x=0;x<=iCount-1;x++) {
			rDB<<"SELECT ID,SubID FROM NotStackable LIMIT :x,1;",
				into(ID.first),
				into(ID.second),
				use(x),
				now;

			index = search(_vBlocks,ID);

			if (index == -1) {
				cout<<"\t\t\tBlock "<<ID.first<<":"<<int(ID.second)<<" not found!\n";
			}else{
				_vBlocks[index].Stackable = false;
			}
		}
	}
	iCount=0;


	//Special scale
	cout<<"\t\t*Reading special scales...\n";
	rDB<<"SELECT count(`ID`) FROM `SpecialScale`",into(iCount),now;
	if (iCount > 0) {
		float Height,Thickness;

		for (x=0;x<=iCount-1;x++) {
			rDB<<"SELECT ID,SubID,Height,Thickness FROM SpecialScale LIMIT :x,1;",
				into(ID.first),
				into(ID.second),
				into(Height),
				into(Thickness),
				use(x),
				now;

			index = search(_vBlocks,ID);

			if (index == -1) {
				cout<<"\t\t\tBlock "<<ID.first<<":"<<int(ID.second)<<" not found!\n";
			}else{
				_vBlocks[index].Height = Height;
				_vBlocks[index].Thickness = Thickness;
			}
		}
	}
	iCount=0;


	//Weapons
	cout<<"\t\t*Reading weapon flags...\n";
	rDB<<"SELECT count(`ID`) FROM `Weapons`",into(iCount),now;
	if (iCount > 0) {
		char Damage;
		ID.second = 0;

		for (x=0;x<=iCount-1;x++) {
			rDB<<"SELECT ID,Damage FROM Weapons LIMIT :x,1;",
				into(ID.first),
				into(Damage),
				use(x),
				now;

			index = search(_vItems,ID);

			if (index == -1) {
				cout<<"\t\t\tItem "<<ID.first<<" not found!\n";
			}else{
				_vItems[index].Weapon = true;
				_vItems[index].Damage = Damage;
			}
		}
	}
	iCount=0;


	//Not placeable
	cout<<"\t\t*Reading notPlaceable flags...\n";
	rDB<<"SELECT count(`ID`) FROM `NotPlaceable`",into(iCount),now;
	if (iCount > 0) {
		for (x=0;x<=iCount-1;x++) {
			rDB<<"SELECT ID,SubID FROM NotPlaceable LIMIT :x,1;",
				into(ID.first),
				into(ID.second),
				use(x),
				now;

			index = search(_vBlocks,ID);

			if (index == -1) {
				cout<<"\t\t\tBlock "<<ID.first<<":"<<int(ID.second)<<" not found!\n";
			}else{
				_vBlocks[index].Placeable = false;
			}
		}
	}
	iCount=0;

	cout<<"\t-Checking data...\n";
	if (!_vBlocks.empty()) {
		for (x = _vBlocks.size() -1; x > 0; x--) {
			try {
				isValid(_vBlocks[x]);
			}catch(Poco::RuntimeException& ex) {
				cout<<"Block #"<<int(_vBlocks[x].ID)<<":"<<int(_vBlocks[x].SubID)<<" is invalid ("<<ex.message()<<")\n";
				_vBlocks.erase (_vBlocks.begin() + x);
			}
		}
	}

	if (!_vItems.empty()) {
		for (x = _vItems.size() -1; x > 0; x--) {
			try {
				isValid(_vItems[x]);
			}catch(Poco::RuntimeException& ex) {
				cout<<"Item #"<<_vItems[x].ID<<":"<<int(_vItems[x].SubID)<<" is invalid ("<<ex.message()<<")\n";
				_vItems.erase (_vItems.begin() + x);
			}
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
	if (search(_vBlocks, ID) == -1 && 
		search(_vItems, ID) == -1) {
		return false;
	}else{
		return true;
	}
}

bool ItemInfoStorage::isRegistered(short ID) {
	if (search(_vBlocks,  std::make_pair(ID,0)) == -1 && 
		search(_vItems,  std::make_pair(ID,0)) == -1) {
		return false;
	}else{
		return true;
	}
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

	int index = search(_vBlocks,ID);
	if (index == -1) {
		throw Poco::RuntimeException("Not found!");
	}

	return _vBlocks[index];	
}

BlockEntry ItemInfoStorage::getBlock(short iID) {
	if (_vBlocks.empty()) { throw Poco::RuntimeException("Not found!"); }
	if (!isBlock(iID)) { throw Poco::RuntimeException("Not a block!"); }

	int index = search(_vBlocks, std::make_pair(iID,0));
	if (index == -1) {
		throw Poco::RuntimeException("Not found!");
	}

	return _vBlocks[index];	
}

ItemEntry ItemInfoStorage::getItem(ItemID ID) {
	if (_vItems.empty()) { throw Poco::RuntimeException("Not found!"); }
	if (isBlock(ID)) { throw Poco::RuntimeException("Not a item!"); }

	int index = search(_vItems, ID);
	if (index == -1) {
		throw Poco::RuntimeException("Not found!");
	}

	return _vItems[index];	
}

ItemEntry ItemInfoStorage::getItem(short iID) {
	if (_vItems.empty()) { throw Poco::RuntimeException("Not found!"); }
	if (isBlock(iID)) { throw Poco::RuntimeException("Not a item!"); }

	int index = search(_vItems, std::make_pair(iID,0));
	if (index == -1) {
		throw Poco::RuntimeException("Not found!");
	}

	return _vItems[index];	
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

	if (Entry.MaxStackSize <= 0) {
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

	if (Entry.SubID > 15 || Entry.SubID < 0) {
		throw Poco::RuntimeException("Illegal SubID");
	}

	if (Entry.ID < 256) {
		throw Poco::RuntimeException("ID illegal or in block range");
	}

	if (Entry.Name.compare("") == 0) {
		throw Poco::RuntimeException("Name is empty");
	}

	if (Entry.Damage <= 0) {
		throw Poco::RuntimeException("Illegal damage");
	}

	if (Entry.FoodValue < 0) { 
		throw Poco::RuntimeException("Invalid food value");
	}

	if (!Entry.Eatable && Entry.FoodValue != 0) {
		throw Poco::RuntimeException("Non eatable things can't have food value");
	}

	if(Entry.ConnectedBlock.first != -1 && Entry.ConnectedBlock.second != -1) {
		if (search(_vBlocks,Entry.ConnectedBlock) == -1) {
			throw Poco::RuntimeException("Connected block doesn't exist");
		}
	}
}

void ItemInfoStorage::isValid(BlockEntry Entry) {
	if (Entry.SubID > 15 || Entry.SubID < 0) {
		throw Poco::RuntimeException("Illegal SubID");
	}

	if (Entry.Name.compare("") == 0) {
		throw Poco::RuntimeException("Name is empty");
	}

	if (Entry.MaxStackSize <= 0) {
		throw Poco::RuntimeException("Illegal stack size");
	}

	if (Entry.SelfLightLevel > 17 || Entry.SelfLightLevel < 0) {
		throw Poco::RuntimeException("Illegal SelfLightLevel");
	}


	if (Entry.Thickness < 0.0F || Entry.Thickness > 1.0F) {
		throw Poco::RuntimeException("Illegal thickness");
	}

	if (Entry.Height < 0.0F || Entry.Height > 1.0F) {
		throw Poco::RuntimeException("Illegal height");
	}

	if (Entry.noLoot) {
		if (Entry.ConnectedItem.first != 0) {
			throw Poco::RuntimeException("noLoot flag set, connected item != 0");
		}
	}else{
		if (Entry.ConnectedItem.first != -1 && Entry.ConnectedItem.second != -1) {
			if (search(_vItems,Entry.ConnectedItem) == -1) {
				throw Poco::RuntimeException("Connected item doesn't exist");
			}
		}
	}
}

int ItemInfoStorage::search(vector<BlockEntry>& vec,ItemID id) {
	if (vec.empty()) {return -1;}
	if (id.first > 255) {return -1;}
	if (id.second < 0 || id.second > 15) {return -1;}

	for (int x=0;x<=vec.size()-1;x++) {
		if (  vec[x].ID == id.first && vec[x].SubID == id.second) {
			return x;
		}
	}
	return -1;
}

int ItemInfoStorage::search(vector<ItemEntry>& vec,ItemID id) {
	if (vec.empty()) {return -1;}
	if (id.first < 256) {return -1;}
	if (id.second < 0 || id.second > 15) {return -1;}

	for (int x=0;x<=vec.size()-1;x++) {
		if (vec[x].ID == id.first && vec[x].SubID == id.second) {
			return x;
		}
	}
	return -1;
}