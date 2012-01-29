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

/*
* This class reads the server and map configuration. It also provide accessors for given server properties.
* If the configuration file doesn't exists it will be created.
* Syntax errors will be shown.
*/

#ifndef _FASTCRAFTHEADER_SETTINGSNHANDLER
#define _FASTCRAFTHEADER_SETTINGSNHANDLER
#include <iostream>
#include <Poco/Path.h>
#include <Poco/AutoPtr.h>
#include <Poco/DOM/Document.h>

using std::string;

class SettingsHandler {
private:
	//Network
	static int _iPort;
	static int _iPlayerSlots;
	
	//Server Info
	static string _sServerDescription;
	static string _sMOTD;
	static int _iServerMode;
	static char _iDifficulty;
	static bool _fOnlineMode;
	static bool _fWhitelist;
	
	//Map Info
	static int _iWorldHeight;
	static bool _fAllowNeather;
	static string _sMainMapName;

	//Spawning
	static bool _fSpawnPeacefulMobs;
	static bool _fSpawnHostileMobs;

	//Player
	static int _iViewDistance;
	static bool _fPVP;
	static bool _fAllowFlyMod;
	static double _dMaxSpeed;
public:
	/*
	* Loads configuration
	*/
	static void readConfiguration(Poco::Path);


	//Property Accessors
		//Network
		static short getPort(); //Returns port that FastCraft binds
		static int getPlayerSlotCount(); //Returns maximal allowed parallel connected clients
		
		//Version
		static string getSupportedMCVersion(); //Returns accepted Minecraft Version (Text)
		static string getFastCraftVersion(); //Returns actual FastCraft Version (Text)
		static int getSupportedProtocolVersion(); //Returns actual supported minecraft protocol version 

		//Server Info
		static string getServerDescription();
		static string getServerMOTD();
		static int getServerMode(); //false for survival, true for creative
		static char getDifficulty();
		static bool isOnlineModeActivated(); //returns true if name verification is required
		static bool isWhitelistActivated();

		//Map Info
		static unsigned char getWorldHeight();
		static bool isNeatherAllowed(); 
		static string getMainWorldName();

		//Spawning
		static bool isPeacefulAnimalSpawningActivated();
		static bool isHostileAnimalSpawningActivated();

		//Player
		static int getViewDistance();
		static bool isPVPActivated();
		static bool isFlyModAllowed();
		static double getMaxMovementSpeed();
private:
	SettingsHandler();
	~SettingsHandler();
	static int parseNodeInt(Poco::AutoPtr<Poco::XML::Document>,string);
	static double parseNodeDouble(Poco::AutoPtr<Poco::XML::Document>,string);
	static void parseNodeString(Poco::AutoPtr<Poco::XML::Document>,string,string&,string);
};

#endif