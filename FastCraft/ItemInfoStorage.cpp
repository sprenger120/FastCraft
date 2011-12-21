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

#include "ItemInfoStorage.h"
#include <Poco/Exception.h>
#include <Poco/String.h>
using Poco::RuntimeException;

ItemInfoStorage::ItemInfoStorage() {
}

ItemInfoStorage::~ItemInfoStorage() {
}

bool ItemInfoStorage::_fBasicItemsAdded = false;
vector<ItemInformation> ItemInfoStorage::_vItems(0);

void ItemInfoStorage::addBasicIDSet() {
	if (_fBasicItemsAdded) {
		return;
	}
	_fBasicItemsAdded=true;
	addItem(0,"Air",false,false,64,0);
	addItem(1,"Stone",false,false,64,0);
	addItem(2,"Grass",false,false,64,0);
	addItem(3,"Dirt",false,false,64,0);
	addItem(4,"Cobblestone",false,false,64,0);
	addItem(5,"Wooden_Plank",false,false,64,0);
	addItem(6,"Sapeling",false,false,64,0);
	addItem(7,"Bedrock",false,false,64,0);
	addItem(8,"Water",false,false,64,0);
	addItem(9,"Stationary_Water",false,false,64,0);
	addItem(10,"Lava",false,false,64,0);
	addItem(11,"Stationary_Lava",false,false,64,0);
	addItem(12,"Sand",false,false,64,0);
	addItem(13,"Gravel",false,false,64,0);
	addItem(14,"Gold_Ore",false,false,64,0);
	addItem(15,"Iron_Ore",false,false,64,0);
	addItem(16,"Coal_Ore",false,false,64,0);
	addItem(17,"Log",false,false,64,0);
	addItem(18,"Leaves",false,false,64,0);
	addItem(19,"Sponge",false,false,64,0);
	addItem(20,"Glass",false,false,64,0);
	addItem(328,"Minecart",false,false,4,0);
	addItem(329,"Saddle",false,false,1,0);
	addItem(330,"Iron_Door",false,false,1,0);
	addItem(331,"Redstone_Dust",false,false,64,0);
	addItem(332,"Snowball",false,false,16,0);
	addItem(333,"Boat",false,false,1,0);
	addItem(334,"Leather",false,false,64,0);
	addItem(335,"Milk_Bukkit",false,false,64,0);
	addItem(336,"Brick",false,false,64,0);
	addItem(337,"Clay",false,false,64,0);
	addItem(338,"Sugar_Cane",false,false,64,0);
	addItem(339,"Paper",false,false,64,0);
	addItem(340,"Book",false,false,64,0);
	addItem(341,"Slimeball",false,false,64,0);
	addItem(342,"Chest_Minecart",false,false,1,0);
	addItem(343,"Furnace_Minecart",false,false,1,0);
	addItem(344,"Egg",false,false,16,0);
	addItem(345,"Compass",false,false,64,0);
	addItem(346,"Fishing_Rod",true,false,1,65);
	addItem(347,"Clock",false,false,64,0);
	addItem(348,"Glowstone_Dust",false,false,64,0);
	addItem(349,"Fish",false,false,64,0);
	addItem(350,"Cooked_Fish",false,false,64,0);
	addItem(41,"Gold_Block",false,false,64,0);
	addItem(42,"Iron_Block",false,false,64,0);
	addItem(43,"Double_Slabs",false,false,64,0);
	addItem(44,"Slabs",false,false,64,0);
	addItem(45,"Bricks",false,false,64,0);
	addItem(46,"TNT",false,false,64,0);
	addItem(47,"Bookshelf",false,false,64,0);
	addItem(48,"Moss_Stone",false,false,64,0);
	addItem(49,"Obsidian",false,false,64,0);
	addItem(50,"Torch",false,false,64,0);
	addItem(51,"Fire",false,false,64,0);
	addItem(52,"Monster_Spawner",false,false,64,0);
	addItem(53,"Wooden_Stairs",false,false,64,0);
	addItem(54,"Chest",false,false,64,0);
	addItem(55,"Redstone_Wire",false,false,64,0);
	addItem(56,"Diamond_Ore",false,false,64,0);
	addItem(57,"Diamond_Block",false,false,64,0);
	addItem(58,"Workbench",false,false,64,0);
	addItem(59,"Wheat_Seeds",false,false,64,0);
	addItem(60,"Farmland",false,false,64,0);
	addItem(283,"Gold_Sword",true,true,1,33);
	addItem(284,"Gold_Shovel",true,true,1,33);
	addItem(285,"Gold_Pickaxe",true,true,1,33);
	addItem(286,"Gold_Axe",true,true,1,33);
	addItem(287,"String",false,false,64,0);
	addItem(288,"Feather",false,false,64,0);
	addItem(289,"Gunpowder",false,false,64,0);
	addItem(290,"Wooden_Hoe",true,false,1,60);
	addItem(291,"Stone_Hoe",true,false,1,132);
	addItem(292,"Iron_Hoe",true,false,1,251);
	addItem(293,"Diamond_Hoe",true,false,1,1562);
	addItem(294,"Gold_Hoe",true,false,1,33);
	addItem(295,"Seeds",false,false,64,0);
	addItem(296,"Wheat",false,false,64,0);
	addItem(297,"Bread",false,false,64,0);
	addItem(298,"Leather_Cap",true,true,1,0);
	addItem(299,"Leather_Tunic",true,true,1,0);
	addItem(300,"Leather_Pants",true,true,1,0);
	addItem(301,"Leather_Boots",true,true,1,0);
	addItem(302,"Chain_Helmet",true,true,1,0);
	addItem(303,"Chain_Chestplate",true,true,1,0);
	addItem(304,"Chain_Leggings",true,true,1,0);
	addItem(305,"Chain_Boots",true,true,1,0);
	addItem(306,"Iron_Helmet",true,true,1,0);
	addItem(307,"Iron_Chestplate",true,true,1,0);
	addItem(308,"Iron_Leggings",true,true,1,0);
	addItem(309,"Iron_Boots",true,true,1,0);
	addItem(21,"Lapis_Lazuli_Ore",false,false,64,0);
	addItem(22,"Lapis_Lazuli_Block",false,false,64,0);
	addItem(23,"Dispenser",false,false,64,0);
	addItem(24,"Sandstone",false,false,64,0);
	addItem(25,"Note_Block",false,false,64,0);
	addItem(27,"Powered_Rail",false,false,64,0);
	addItem(28,"Detector_Rail",false,false,64,0);
	addItem(29,"Sticky_Piston",false,false,64,0);
	addItem(30,"Cobweb",false,false,64,0);
	addItem(31,"Tall_Grass",false,false,64,0);
	addItem(32,"Dead_Bush",false,false,64,0);
	addItem(33,"Piston",false,false,64,0);
	addItem(35,"Wool",false,false,64,0);
	addItem(37,"Dandelion",false,false,64,0);
	addItem(38,"Rose",false,false,64,0);
	addItem(39,"Brown_Mushroom",false,false,64,0);
	addItem(40,"Red_Mushroom",false,false,64,0);
	addItem(256,"Iron_Shovel",true,true,1,251);
	addItem(257,"Iron_Pickaxe",true,true,1,251);
	addItem(258,"Iron_Axe",true,true,1,251);
	addItem(259,"Flint_and_Steel",true,false,1,65);
	addItem(260,"Red_Apple",false,false,64,0);
	addItem(261,"Bow",true,false,1,385);
	addItem(262,"Arrow",false,false,64,0);
	addItem(263,"Coal",false,false,64,0);
	addItem(264,"Diamond",false,false,64,0);
	addItem(265,"Iron_Ingot",false,false,64,0);
	addItem(266,"Gold_Ingot",false,false,64,0);
	addItem(267,"Iron_Sword",true,true,1,251);
	addItem(268,"Wooden_Sword",true,true,1,60);
	addItem(269,"Wooden_Shovel",true,true,1,60);
	addItem(270,"Wooden_Pickaxe",true,true,1,60);
	addItem(271,"Wooden_Axe",true,true,1,60);
	addItem(272,"Stone_Sword",true,true,1,132);
	addItem(273,"Stone_Shovel",true,true,1,132);
	addItem(274,"Stone_Pickaxe",true,true,1,132);
	addItem(275,"Stone_Axe",true,true,1,132);
	addItem(276,"Diamond_Sword",true,true,1,1562);
	addItem(277,"Diamond_Shovel",true,true,1,1562);
	addItem(278,"Diamond_Pickaxe",true,true,1,1562);
	addItem(279,"Diamond_Axe",true,true,1,1562);
	addItem(280,"Stick",false,false,64,0);
	addItem(281,"Bowl",false,false,64,0);
	addItem(282,"Mushroom_Stew",false,false,1,0);
	addItem(61,"Furnace",false,false,64,0);
	addItem(62,"Burning_Furnace",false,false,64,0);
	addItem(65,"Ladders",false,false,64,0);
	addItem(66,"Rails",false,false,64,0);
	addItem(67,"Cobblestone_Stairs",false,false,64,0);
	addItem(68,"Wall_Sign",false,false,64,0);
	addItem(69,"Lever",false,false,64,0);
	addItem(70,"Stone_Pressure_Plate",false,false,64,0);
	addItem(72,"Wooden_Pressure_Plate",false,false,64,0);
	addItem(73,"Redstone_Ore",false,false,64,0);
	addItem(75,"Redstone_Torch_Off",false,false,64,0);
	addItem(76,"Redstone_Torch_On",false,false,64,0);
	addItem(77,"Stone_Button",false,false,64,0);
	addItem(78,"Snow",false,false,64,0);
	addItem(79,"Ice",false,false,64,0);
	addItem(80,"Snow_Block",false,false,64,0);
	addItem(101,"Iron_Bars",false,false,64,0);
	addItem(102,"Glass_Pane",false,false,64,0);
	addItem(103,"Melon",false,false,64,0);
	addItem(106,"Vines",false,false,64,0);
	addItem(107,"Fence_Gate",false,false,64,0);
	addItem(108,"Brick_Stairs",false,false,64,0);
	addItem(109,"Stone_Brick_Stairs",false,false,64,0);
	addItem(110,"Mycelium",false,false,64,0);
	addItem(111,"Lily_Pad",false,false,64,0);
	addItem(112,"Nether_Brick",false,false,64,0);
	addItem(113,"Nether_Brick_Fence",false,false,64,0);
	addItem(114,"Nether_Brick_Stairs",false,false,64,0);
	addItem(115,"Nether_Wart_Plant",false,false,64,0);
	addItem(116,"Enchantment_Table",false,false,64,0);
	addItem(117,"Brewing_Stand",false,false,64,0);
	addItem(118,"Cauldron",false,false,64,0);
	addItem(121,"End_Stone",false,false,64,0);
	addItem(310,"Diamond_Helmet",true,true,1,0);
	addItem(311,"Diamond_Chestplate",true,true,1,0);
	addItem(312,"Diamond_Leggings",true,true,1,0);
	addItem(313,"Diamond_Boots",true,true,1,0);
	addItem(314,"Gold_Helmet",true,true,1,0);
	addItem(315,"Gold_Chestplate",true,true,1,0);
	addItem(316,"Gold_Leggings",true,true,1,0);
	addItem(317,"Gold_Boots",true,true,1,0);
	addItem(318,"Flint",false,false,64,0);
	addItem(319,"Raw_Porkchop",false,false,64,0);
	addItem(320,"Cooked_Porkchop",false,false,64,0);
	addItem(321,"Painting",true,false,64,0);
	addItem(322,"Golden_Apple",false,false,64,0);
	addItem(323,"Sign",true,false,1,0);
	addItem(325,"Bucket",false,false,1,0);
	addItem(326,"Water_Bucket",false,false,1,0);
	addItem(327,"Lava_Bucket",false,false,1,0);
	addItem(352,"Bone",false,false,64,0);
	addItem(353,"Sugar",false,false,64,0);
	addItem(354,"Cake",false,false,64,0);
	addItem(356,"Redstone_Repeater",true,false,64,0);
	addItem(357,"Cookie",false,false,64,0);
	addItem(358,"Map",false,false,64,0);
	addItem(359,"Shears",false,false,1,238);
	addItem(360,"Melon_Slice",false,false,64,0);
	addItem(361,"Pumkin_Seeds",false,false,64,0);
	addItem(362,"Melon_Seeds",false,false,64,0);
	addItem(363,"Raw_Beef",false,false,64,0);
	addItem(364,"Steak",false,false,64,0);
	addItem(365,"Raw_Chicken",false,false,64,0);
	addItem(366,"Cooked_Chicken",false,false,64,0);
	addItem(367,"Rotten_Flesh",false,false,64,0);
	addItem(368,"Ender_Pearl",false,false,64,0);
	addItem(369,"Blaze_Rod",false,false,64,0);
	addItem(370,"Ghast_Tear",false,false,1,0);
	addItem(371,"Gold_Nugget",false,false,64,0);
	addItem(372,"Nether_Wart",false,false,64,0);
	addItem(373,"Potions",false,false,64,0);
	addItem(374,"Glass_Bottle",false,false,64,0);
	addItem(375,"Spider_Eye",false,false,64,0);
	addItem(376,"Fermented_Spider_Eye",false,false,64,0);
	addItem(377,"Blaze_Powder",false,false,64,0);
	addItem(378,"Magma_Cream",false,false,64,0);
	addItem(381,"Eye_of_Ender",true,false,64,0);
	addItem(382,"Glistering_Melon",false,false,64,0);
	addItem(383,"Spawner_Egg",false,false,64,0);
}


bool ItemInfoStorage::isDamageable(short iId) {
	if (_vItems.size() > 0) {
		for(int x=0;x<=_vItems.size()-1;x++) {
			if (_vItems[x].ID == iId) {
				return _vItems[x].Damageable;
			}
		}	
	}
	throw RuntimeException("Unknown ItemID! Given:" + iId);
}

bool ItemInfoStorage::isEnchantable(short iId) {
	if (_vItems.size() > 0) {
		for(int x=0;x<=_vItems.size()-1;x++) {
			if (_vItems[x].ID == iId) {
				return _vItems[x].Enchantable;
			}
		}	
	}
	throw RuntimeException("Unknown ItemID! Given:" + iId);
}

char ItemInfoStorage::getMaxStackSize(short iId) {
	if (_vItems.size() > 0) {
		for(int x=0;x<=_vItems.size()-1;x++) {
			if (_vItems[x].ID == iId) {
				return _vItems[x].MaxStackSize;
			}
		}	
	}
	throw RuntimeException("Unknown ItemID! Given:" + iId);
}

short ItemInfoStorage::getMaxUsage(short iId) {
	if (_vItems.size() > 0) {
		for(int x=0;x<=_vItems.size()-1;x++) {
			if (_vItems[x].ID == iId) {
				return _vItems[x].MaxUsage;
			}
		}	
	}
	throw RuntimeException("Unknown ItemID! Given:" + iId);
}


string ItemInfoStorage::getName(short iId) {
	if (_vItems.size() > 0) {
		for(int x=0;x<=_vItems.size()-1;x++) {
			if (_vItems[x].ID == iId) {
				return _vItems[x].Name;
			}
		}	
	}
	throw RuntimeException("Unknown ItemID! Given:" + iId);
}

bool ItemInfoStorage::isRegistered(short iId) {
	if (_vItems.size() > 0) {
		for(int x=0;x<=_vItems.size()-1;x++) {
			if (_vItems[x].ID == iId) {
				return true;
			}
		}	
	}
	return false;
}

short ItemInfoStorage::getIDbyName(string sName) {
	if (_vItems.size() > 0) {
		for(int x=0;x<=_vItems.size()-1;x++) {
			
			if (Poco::icompare(_vItems[x].Name,sName) == 0) {
				return _vItems[x].ID;
			}
		}	
	}
	return 0;
}

void ItemInfoStorage::addItem(short iId,string sName,bool fDamageable,bool fEnchantable,char iMaxStackSize,short iMaxUsage) {
	if (iId < 0) {
		throw RuntimeException("Item ID invalid (n < 0)");
	}
	if (iMaxUsage < 0) {
		throw RuntimeException("MaxUsage invalid (n < 0) Affected ID:" + iId);
	}
	if (iMaxStackSize <= 0) {
		throw RuntimeException("MaxStackSize invalid (n <= 0) Affected ID:" + iId);
	}
	if (iId <= 255 && (iMaxUsage > 0 || fDamageable || fEnchantable)) {
		throw RuntimeException("Blocks haven't a damage bar! Affected ID:" + iId);
	}
	//Check availability of item id
	if (_vItems.size() > 0) {
		for(int x=0;x<=_vItems.size()-1;x++) {
			if (_vItems[x].ID == iId) {
				throw RuntimeException("Item/Block ID already in use! Affected Block:" + sName);
			}
		}
	}

	ItemInformation info;
	info.ID = iId;
	info.Name.assign(sName);
	info.Damageable = fDamageable;
	info.Enchantable = fEnchantable;
	info.MaxStackSize = iMaxStackSize;
	info.MaxUsage = iMaxUsage;

	_vItems.push_back(info);
}