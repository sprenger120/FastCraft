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

/*
* This class reads the server and map configuration. It also provide accessors for given server properties.
* If the configuration file doesn't exists it will be created.
* Syntax errors will be shown.
*/

#ifndef _FASTCRAFTHEADER_SETTINGSNHANDLER
#define _FASTCRAFTHEADER_SETTINGSNHANDLER
#include <iostream>

using std::string;

class SettingsHandler {
private:
	//Network
	int _iPort;
	int _iMaxClients;
	
	//Server Info
	string _sServerDescription;
	int _iServerMode;
	char _iDifficulty;
	bool _fOnlineMode;
	bool _fWhitelist;
	
	//Map Info
	unsigned char _iWorldHeight;
	bool _fAllowNeather;
	string _sMainMapName;

	//Spawning
	bool _fSpawnPeacefulAnimals;
	bool _fSpawnHostileAnimals;

	//Player
	int _iViewDistance;
	bool _fPVP;
public:
	//De- / constructor 
	SettingsHandler(); //Read configuration
	~SettingsHandler(); //Save configuration

	//Property Accessors
		//Network
		short getPort(); //Returns port that FastCraft binds
		int getMaxClients(); //Returns maximal allowed parallel connected clients
		
		//Version
		static string getSupportedMCVersion(); //Returns accepted Minecraft Version (Text)
		static string getFastCraftVersion(); //Returns actual FastCraft Version (Text)
		static int getSupportedProtocolVersion(); //Returns actual supported minecraft protocol version 

		//Server Info
		string getServerDescription();
		int getServerMode(); //false for survival, true for creative
		char getDifficulty();
		bool isOnlineModeActivated(); //returns true if name verification is required
		bool isWhitelistActivated();

		//Map Info
		long long getMapSeed();
		unsigned char getWorldHeight();
		bool isNeatherAllowed(); 
		string getMainWorldName();

		//Spawning
		bool isPeacefulAnimalSpawningActivated();
		bool isHostileAnimalSpawningActivated();

		//Player
		int getViewDistance();
		bool isPVPActivated();
};

#endif