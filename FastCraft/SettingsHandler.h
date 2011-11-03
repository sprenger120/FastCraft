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

#ifndef _FASTCRAFTHEADER_SETTINGSNHANDLER
#define _FASTCRAFTHEADER_SETTINGSNHANDLER
#include <iostream>
using std::string;

class SettingsHandler {
private:
	//Network
	short _iPort;
	int _iMaxClients;
	
	//Version
	string _sSupportedMCVersion;
	string _sFastcraftVersion;
	int _iSupportedProtocolVersion;

	//Server Info
	string _sServerDescription;
public:
	//De- / constructor 
	SettingsHandler(); //Read configuration
	~SettingsHandler(); //Save configuration

	//Property Accessors

		//Network
		short getPort(); //Returns port that FastCraft binds
		int getMaxClients(); //Returns maximal allowed parallel connected clients
		
		//Version
		string getSupportedMCVersion(); //Returns accepted Minecraft Version (Text)
		string getFastCraftVersion(); //Returns actual FastCraft Version (Text)

		int getSupportedProtocolVersion(); //Returns actual supported minecraft protocol version

		//Server Info
		string getServerDescription();
};


#endif