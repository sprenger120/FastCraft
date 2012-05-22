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

#include "ItemInformationProvider.h"
#include "FCRuntimeException.h"
#include <Poco/String.h>
#include <Poco/Path.h>
#include <Poco/File.h>
#include "Constants.h"

using std::cout;
using std::string;
using Poco::Data::into;
using Poco::Data::now;
using Poco::Data::use;

ItemInformationProvider::ItemInformationProvider(Poco::Path& rPath) : 
_vItems(0),
	_vBlocks(0)
{
	Poco::Data::Session DB("SQLite", rPath.toString());

	int iItemCount = 0,iBlockCount=0,x=0,i=0,index=0,iCount=0,Speed=0,Spread=0;
	bool fFound;
	ItemEntry IEntry;
	BlockEntry BEntry;
	ItemID ID;

	DB<<"SELECT count(`ID`) FROM `Items`",into(iItemCount),now; //Fetch item count
	DB<<"SELECT count(`ID`) FROM `Blocks`",into(iBlockCount),now; //Fetch block count

	BEntry.Thickness		= 1.0F;
	BEntry.Height			= 1.0F;
	BEntry.Stackable		= true; /* You can place a block of same type above or under */
	BEntry.CanFloat			= true;
	BEntry.ConnectedItem	= FC_EMPTYITEMID; /* if this field is set to another value as -1, the given ITEM will pop off on breaking*/
	BEntry.BlastResistance  = 1.0F;
	BEntry.hasSubBlocks		= false;
	BEntry.noLoot			= false;
	BEntry.Placeable		= true;
	BEntry.Fluid			= false;
	BEntry.Spread			= 0;
	BEntry.Speed			= 0;

	IEntry.Eatable			= false;
	IEntry.ConnectedBlock	= FC_EMPTYITEMID; /* if this field is set to another value as -1, the given BLOCK will placed*/
	IEntry.Weapon			= false;
	IEntry.Damage			= 1;
	IEntry.FoodValue		= 0;
	IEntry.hasSubItems		= false;

	/*
	* Load Item/Blocks
	*/
	if (iBlockCount>0) {
		for (x=0;x<=iBlockCount-1;x++) {
			DB<<"SELECT ID,SubID,Name,MaxStackSize,SelfLightLevel,Flammable,Solid,NeededTool,ToolLevel FROM Blocks LIMIT :x,1;",
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

			fFound=false;
			if (!_vBlocks.empty()) {
				for(int i=0;i<=_vBlocks.size()-1;i++) {
					if (_vBlocks[i].ID == BEntry.ID && _vBlocks[i].SubID == BEntry.SubID) {
						cout<<"\tBlock ID "<<BEntry.ID<<":"<<int(BEntry.SubID)<<" is already in use!\n";
						fFound=true;
						break;
					}
				}
			}
			if (!fFound) {_vBlocks.push_back(BEntry);}
		}
	}

	if (iItemCount>0) {
		for (x=0;x<=iItemCount-1;x++) {
			DB<<"SELECT ID,SubID,Name,Damageable,Enchantable,Durability,MaxStackSize FROM Items LIMIT :x,1;",
				into(IEntry.ID),
				into(IEntry.SubID),
				into(IEntry.Name),
				into(IEntry.Damageable),
				into(IEntry.Enchantable),
				into(IEntry.Durability),
				into(IEntry.MaxStackSize),
				use(x),
				now;
			fFound=false;
			if (!_vItems.empty()) {
				for(int i=0;i<=_vItems.size()-1;i++) {
					if (_vItems[i].ID == IEntry.ID && _vItems[i].SubID == IEntry.SubID) {
						cout<<"\tItem ID "<<BEntry.ID<<":"<<int(BEntry.SubID)<<" is already in use!\n";
						fFound=true;
						break;
					}
				}
			}
			if (!fFound) {_vItems.push_back(IEntry);}
		}
	}


	/*
	* Load special per block/item properties
	*/

	//Connected Block
	DB<<"SELECT count(`ID`) FROM `ConnectedBlock`",into(iCount),now;
	if (iCount > 0) {
		ItemID ConnectedBlock;

		for (x=0;x<=iCount-1;x++) {
			DB<<"SELECT ID,SubID,ConnectedBlockID,ConnectedBlockSubID FROM ConnectedBlock LIMIT :x,1;",
				into(ID.first),
				into(ID.second),
				into(ConnectedBlock.first),
				into(ConnectedBlock.second),
				use(x),
				now;

			index = search(_vItems,ID);
			if (index==-1) {
				cout<<"\tItem "<<ID.first<<":"<<int(ID.second)<<" not found!\n";
			}else{
				_vItems[index].ConnectedBlock = ConnectedBlock;
			}
		}
	}
	iCount=0;


	//Connected Item
	DB<<"SELECT count(`ID`) FROM `ConnectedItem`",into(iCount),now;
	if (iCount > 0) {
		ItemID ConnectedItem;

		for (x=0;x<=iCount-1;x++) {
			DB<<"SELECT ID,SubID,ConnectedItemID,ConnectedItemSubID FROM ConnectedItem LIMIT :x,1;",
				into(ID.first),
				into(ID.second),
				into(ConnectedItem.first),
				into(ConnectedItem.second),
				use(x),
				now;

			index = search(_vBlocks,ID);

			if (index == -1) {
				cout<<"\tBlock "<<ID.first<<":"<<int(ID.second)<<" not found!\n";
			}else{
				if (ConnectedItem.first == 0) {_vBlocks[index].noLoot = true; }
				_vBlocks[index].ConnectedItem = ConnectedItem;
			}
		}
	}
	iCount=0;


	//Eatable
	DB<<"SELECT count(`ID`) FROM `Eatable`",into(iCount),now; 
	if (iCount > 0) {
		char iFoodValue;
		for (x=0;x<=iCount-1;x++) {
			DB<<"SELECT ID,SubID,FoodValue FROM Eatable LIMIT :x,1;",
				into(ID.first),
				into(ID.second),
				into(iFoodValue),
				use(x),
				now;

			index = search(_vItems,ID);

			if (index == -1) {
				cout<<"\tItem "<<ID.first<<":"<<int(ID.second)<<" not found!\n";
			}else{
				_vItems[index].Eatable = true;
				_vItems[index].FoodValue = iFoodValue;
			}
		}
	}
	iCount=0;


	//NoFloating
	DB<<"SELECT count(`ID`) FROM `NoFloating`",into(iCount),now;
	if (iCount > 0) {
		for (x=0;x<=iCount-1;x++) {
			DB<<"SELECT ID,SubID FROM NoFloating LIMIT :x,1;",
				into(ID.first),
				into(ID.second),
				use(x),
				now;

			index = search(_vBlocks,ID);

			if (index == -1) {
				cout<<"\tBlock "<<ID.first<<":"<<int(ID.second)<<" not found!\n";
			}else{
				_vBlocks[index].CanFloat = false;
			}
		}
	}
	iCount=0;


	//Not stackable
	DB<<"SELECT count(`ID`) FROM `NotStackable`",into(iCount),now;
	if (iCount > 0) {
		for (x=0;x<=iCount-1;x++) {
			DB<<"SELECT ID,SubID FROM NotStackable LIMIT :x,1;",
				into(ID.first),
				into(ID.second),
				use(x),
				now;

			index = search(_vBlocks,ID);

			if (index == -1) {
				cout<<"\tBlock "<<ID.first<<":"<<int(ID.second)<<" not found!\n";
			}else{
				_vBlocks[index].Stackable = false;
			}
		}
	}
	iCount=0;


	//Special scale
	DB<<"SELECT count(`ID`) FROM `SpecialScale`",into(iCount),now;
	if (iCount > 0) {
		float Height,Thickness;

		for (x=0;x<=iCount-1;x++) {
			DB<<"SELECT ID,SubID,Height,Thickness FROM SpecialScale LIMIT :x,1;",
				into(ID.first),
				into(ID.second),
				into(Height),
				into(Thickness),
				use(x),
				now;

			index = search(_vBlocks,ID);

			if (index == -1) {
				cout<<"\tBlock "<<ID.first<<":"<<int(ID.second)<<" not found!\n";
			}else{
				_vBlocks[index].Height = Height;
				_vBlocks[index].Thickness = Thickness;
			}
		}
	}
	iCount=0;


	//Weapons
	DB<<"SELECT count(`ID`) FROM `Weapons`",into(iCount),now;
	if (iCount > 0) {
		char Damage;
		ID.second = 0;

		for (x=0;x<=iCount-1;x++) {
			DB<<"SELECT ID,Damage FROM Weapons LIMIT :x,1;",
				into(ID.first),
				into(Damage),
				use(x),
				now;

			index = search(_vItems,ID);

			if (index == -1) {
				cout<<"\tItem "<<ID.first<<" not found!\n";
			}else{
				_vItems[index].Weapon = true;
				_vItems[index].Damage = Damage;
			}
		}
	}
	iCount=0;


	//Not placeable
	DB<<"SELECT count(`ID`) FROM `NotPlaceable`",into(iCount),now;
	if (iCount > 0) {
		for (x=0;x<=iCount-1;x++) {
			DB<<"SELECT ID,SubID FROM NotPlaceable LIMIT :x,1;",
				into(ID.first),
				into(ID.second),
				use(x),
				now;

			index = search(_vBlocks,ID);

			if (index == -1) {
				cout<<"\tBlock "<<ID.first<<":"<<int(ID.second)<<" not found!\n";
			}else{
				_vBlocks[index].Placeable = false;
			}
		}
	}
	iCount=0;


	//Fluid 
	DB<<"SELECT count(`ID`) FROM `Fluid`",into(iCount),now;
	if (iCount > 0) {
		for (x=0;x<=iCount-1;x++) {
			DB<<"SELECT ID,SubID,Spread,Speed FROM Fluid LIMIT :x,1;",
				into(ID.first),
				into(ID.second),
				into(Spread),
				into(Speed),
				use(x),
				now;

			index = search(_vBlocks,ID);

			if (index == -1) {
				cout<<"\tBlock "<<ID.first<<":"<<int(ID.second)<<" not found!\n";
			}else{
				_vBlocks[index].Fluid = true;
				_vBlocks[index].Spread = (char)Spread;
				_vBlocks[index].Speed  = (char)Speed;
			}
		}
	}
	iCount=0;

	cout<<"Postprocessing information...\n";
	bool fBlocksDone = false;
	try {
		if (!_vBlocks.empty()) {
			for (x = _vBlocks.size() -1; x > 0; x--) {
				isValid(_vBlocks[x]);
			}
		}
		fBlocksDone=true;

		if (!_vItems.empty()) {
			for (x = _vItems.size() -1; x > 0; x--) {
				isValid(_vItems[x]);
			}
		}
	}catch(FCRuntimeException) {
		if (!fBlocksDone) {
			cout<<"Block #"<<int(_vBlocks[x].ID)<<":"<<int(_vBlocks[x].SubID)<<" is invalid\n";
		}else{
			cout<<"Item #"<<_vItems[x].ID<<":"<<int(_vItems[x].SubID)<<" is invalid\n";
		}
		cout<<"Clearing loaded blocks/items...\n";
		_vBlocks.clear();
		_vItems.clear();
		return;
	}

	/* Setting the hasSubItems Flag */
	bool fhasSub = false;
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
	DB.close();
}

ItemInformationProvider::~ItemInformationProvider() {
	_vItems.clear();
	_vBlocks.clear();
}

string ItemInformationProvider::getName(ItemID ID) {
	int x;

	//Loop through blocks
	if (!_vBlocks.empty() && isBlock(ID)) {
		for (x=0;x<=_vBlocks.size()-1;x++) {
			if (_vBlocks[x].ID == ID.first) {
				if(_vBlocks[x].hasSubBlocks && _vBlocks[x].SubID != ID.second) {continue;}
				return _vBlocks[x].Name;
			}
		}
	}

	//Loop through items
	if (!_vItems.empty() && !isBlock(ID)) {
		for (x=0;x<=_vItems.size()-1;x++) {
			if (_vItems[x].ID == ID.first) {
				if(_vItems[x].hasSubItems && _vItems[x].SubID != ID.second) {continue;}
				return _vItems[x].Name;
			}
		}
	}
	throw FCRuntimeException("Not found!");
}


string ItemInformationProvider::getName(short ID) {
	int x;

	//Loop through blocks
	if (!_vBlocks.empty() && isBlock(ID)) {
		for (x=0;x<=_vBlocks.size()-1;x++) {
			if (_vBlocks[x].ID == ID) {
				if(_vBlocks[x].hasSubBlocks && _vBlocks[x].SubID != 0){continue;}
				return _vBlocks[x].Name;
			}
		}
	}

	//Loop through items
	if (!_vItems.empty() && !isBlock(ID)) {
		for (x=0;x<=_vItems.size()-1;x++) {
			if (_vItems[x].ID == ID) {
				if(_vItems[x].hasSubItems && _vItems[x].SubID != 0){continue;}
				return _vItems[x].Name;
			}
		}
	}

	throw FCRuntimeException("Not found!");
}


bool ItemInformationProvider::isRegistered(ItemID ID) {
	if (search(_vBlocks, ID) == -1 && 
		search(_vItems, ID) == -1) {
			return false;
	}else{
		return true;
	}
}

bool ItemInformationProvider::isRegistered(short ID) {
	if (search(_vBlocks,  std::make_pair(ID,0)) == -1 && 
		search(_vItems,  std::make_pair(ID,0)) == -1) {
			return false;
	}else{
		return true;
	}
}

ItemID ItemInformationProvider::getIDbyName(string Name) {
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

	throw FCRuntimeException("Not found!");
}

bool ItemInformationProvider::isBlock(ItemID ID) {
	if (!_vBlocks.empty()) {
		for(int x=0;x<=_vBlocks.size()-1;x++) {
			if (_vBlocks[x].ID == ID.first) {
				if (_vBlocks[x].hasSubBlocks && _vBlocks[x].SubID != ID.second) {continue;}
				return true;
			}
		}
	}
	return false;
}

bool ItemInformationProvider::isBlock(short iID) {
	if (!_vBlocks.empty()) {
		for(int x=0;x<=_vBlocks.size()-1;x++) {
			if (_vBlocks[x].ID == iID) {
				if (_vBlocks[x].hasSubBlocks && _vBlocks[x].SubID != 0) {continue;}
				return true;
			}
		}
	}
	return false;
}


BlockEntry* ItemInformationProvider::getBlock(ItemID ID) {
	if (_vBlocks.empty()) { throw FCRuntimeException("Not found!"); }
	if (!isBlock(ID)) { throw FCRuntimeException("Not a block!"); }

	int index = search(_vBlocks,ID);
	if (index == -1) {
		throw FCRuntimeException("Not found!");
	}

	return &_vBlocks[index];	
}

BlockEntry* ItemInformationProvider::getBlock(short iID) {
	if (_vBlocks.empty()) { throw FCRuntimeException("Not found!"); }
	if (!isBlock(iID)) { throw FCRuntimeException("Not a block!"); }

	int index = search(_vBlocks, std::make_pair(iID,0));
	if (index == -1) {
		throw FCRuntimeException("Not found!");
	}

	return &_vBlocks[index];	
}

ItemEntry* ItemInformationProvider::getItem(ItemID ID) {
	if (_vItems.empty()) { throw FCRuntimeException("Not found!"); }
	if (isBlock(ID)) { throw FCRuntimeException("Not a item!"); }

	int index = search(_vItems, ID);
	if (index == -1) {
		throw FCRuntimeException("Not found!");
	}

	return &_vItems[index];	
}

ItemEntry* ItemInformationProvider::getItem(short iID) {
	if (_vItems.empty()) { throw FCRuntimeException("Not found!"); }
	if (isBlock(iID)) { throw FCRuntimeException("Not a item!"); }

	int index = search(_vItems, std::make_pair(iID,0));
	if (index == -1) {
		throw FCRuntimeException("Not found!");
	}

	return &_vItems[index];	
}

int ItemInformationProvider::getItemsInCache() {
	return _vItems.size();
}

int ItemInformationProvider::getBlocksInCache() {
	return _vBlocks.size();
}

void ItemInformationProvider::isValid(ItemEntry Entry) {
	if (Entry.Enchantable && !Entry.Damageable) {
		throw FCRuntimeException("Enchantable but undestroyable");
	}

	if (Entry.Durability < -1 || Entry.Durability == 0) {
		throw FCRuntimeException("Illegal durability");
	}

	if (Entry.Damageable && Entry.Durability==-1) {
		throw FCRuntimeException("Destroyable but no durability");
	}

	if (Entry.MaxStackSize <= 0) {
		throw FCRuntimeException("Illegal stack size");
	}

	if (Entry.Damageable && Entry.MaxStackSize != 1) {
		throw FCRuntimeException("Destroyable but MaxStackSize > 1");
	}

	if (Entry.Damageable && Entry.Eatable) {
		throw FCRuntimeException("Cou can't make tools eatable");
	}

	if (Entry.Weapon && !Entry.Damageable) {
		throw FCRuntimeException("Indestructible weapon");
	}

	if (Entry.SubID > 15 || Entry.SubID < 0) {
		throw FCRuntimeException("Illegal SubID");
	}

	if (Entry.ID > 4096 || Entry.ID < 0) {
		throw FCRuntimeException("Illegal ID");
	}

	if (Entry.Name.compare("") == 0) {
		throw FCRuntimeException("Name is empty");
	}

	if (Entry.Damage <= 0) {
		throw FCRuntimeException("Illegal damage");
	}

	if (Entry.FoodValue < 0) { 
		throw FCRuntimeException("Invalid food value");
	}

	if (!Entry.Eatable && Entry.FoodValue != 0) {
		throw FCRuntimeException("Non eatable things can't have food value");
	}

	if(Entry.ConnectedBlock.first != -1 && Entry.ConnectedBlock.second != -1) {
		if (search(_vBlocks,Entry.ConnectedBlock) == -1) {
			throw FCRuntimeException("Connected block doesn't exist");
		}
	}
}

void ItemInformationProvider::isValid(BlockEntry Entry) {
	if (Entry.SubID > 15 || Entry.SubID < 0) {
		throw FCRuntimeException("Illegal SubID");
	}

	if (Entry.Name.compare("") == 0) {
		throw FCRuntimeException("Name is empty");
	}

	if (Entry.MaxStackSize <= 0) {
		throw FCRuntimeException("Illegal stack size");
	}

	if (Entry.SelfLightLevel > 17 || Entry.SelfLightLevel < 0) {
		throw FCRuntimeException("Illegal SelfLightLevel");
	}

	if (Entry.Thickness < 0.0F || Entry.Thickness > 1.0F) {
		throw FCRuntimeException("Illegal thickness");
	}

	if (Entry.ID > 4096 || Entry.ID < 0) {
		throw FCRuntimeException("Illegal ID");
	}

	if (Entry.Height < 0.0F || Entry.Height > 1.0F) {
		throw FCRuntimeException("Illegal height");
	}

	if (Entry.Fluid) {
		if(Entry.Speed  < 0) { throw FCRuntimeException("Speed is below 0");}
		if(Entry.Spread < 0) {throw FCRuntimeException("Spread is below 0");}
	}

	if (Entry.noLoot) {
		if (Entry.ConnectedItem.first != 0) {
			throw FCRuntimeException("noLoot flag set, connected item not null 0");
		}
	}else{
		if (Entry.ConnectedItem.first != -1 && Entry.ConnectedItem.second != -1) {
			if (search(_vItems,Entry.ConnectedItem) == -1) {
				throw FCRuntimeException("Connected item doesn't exist");
			}
		}
	}
}

int ItemInformationProvider::search(vector<BlockEntry>& vec,ItemID id) {
	if (vec.empty()) {return -1;}
	if (id.first > 255) {return -1;}
	if (id.second < 0 || id.second > 15) {return -1;}

	for (int x=0;x<=vec.size()-1;x++) {
		if (vec[x].ID == id.first) {
			if(vec[x].hasSubBlocks && vec[x].SubID != id.second) {continue;} 
			return x;
		}
	}
	return -1;
}

int ItemInformationProvider::search(vector<ItemEntry>& vec,ItemID id) {
	if (vec.empty()) {return -1;}
	if (id.first < 256) {return -1;}
	if (id.second < 0 || id.second > 15) {return -1;}

	for (int x=0;x<=vec.size()-1;x++) {
		if (vec[x].ID == id.first) {
			if(vec[x].hasSubItems && vec[x].SubID != id.second) {continue;} 
			return x;
		}
	}
	return -1;
}
