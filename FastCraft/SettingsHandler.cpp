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

#include "SettingsHandler.h"
#include "Constants.h"
#include <string>
#include <Poco/Path.h>
#include <fstream>
#include <Poco/AutoPtr.h>
#include <Poco/Exception.h>
#include <Poco/DOM/DOMParser.h>
#include <Poco/SAX/InputSource.h>
#include <Poco/XML/XMLException.h>
#include <Poco/SAX/SAXException.h>
#include <Poco/NumberParser.h>

using std::cout;
using std::endl;
using Poco::Path;
using Poco::AutoPtr;

//Init static variables
int SettingsHandler::_iPort						= 25565;
int SettingsHandler::_iPlayerSlots				= 16;

string SettingsHandler::_sServerDescription		("FastCraft Minecraft server");
string SettingsHandler::_sMOTD					("§dWelcome to FastCraft 0.0.2 Alpha server.");
int SettingsHandler::_iServerMode				= FC_SERVMODE_SURVIVAL;
char SettingsHandler::_iDifficulty				= FC_DIFFICULTY_EASY;	
bool SettingsHandler:: _fOnlineMode				= true;
bool SettingsHandler::_fWhitelist				= false;

int SettingsHandler::_iWorldHeight				= 128;
bool SettingsHandler::_fAllowNeather			= false;
string SettingsHandler::_sMainMapName			("main");

bool SettingsHandler::_fSpawnPeacefulMobs		= true;
bool SettingsHandler::_fSpawnHostileMobs		= true;


int SettingsHandler::_iViewDistance				= 10;
bool SettingsHandler::_fPVP						= true;
bool SettingsHandler::_fAllowFlyMod				= false;
double SettingsHandler::_dMaxSpeed				= 7.5;


SettingsHandler::SettingsHandler(){
}

SettingsHandler::~SettingsHandler() {
}

void SettingsHandler::readConfiguration(Poco::Path path) {
	std::ifstream Input(path.toString());

	if (!Input.good()) {
		cout<<"***Error: Configuration path invalid!"<<"\n";
		cout<<"Starting with standart configuration!"<<std::endl;
		return;
	}


	Poco::XML::InputSource ISource(Input);
	Poco::XML::DOMParser parser;
	Poco::AutoPtr<Poco::XML::Document> pDoc;
	int iData;

	try {
		pDoc = parser.parse(&ISource);
	}catch(Poco::XML::XMLException& ex) {
		cout<<"***Error while parsing settings: "<<ex.message()<<"\n";
		cout<<"Starting with standart configuration!"<<std::endl;
		return;
	}


	/*
	* Parsing settings
	*/

	//Network
	iData = parseNodeInt(pDoc,"/settings/Network/Port");
	_iPort = (iData == -1 || (iData<0 || iData > 65535) ? _iPort : iData);

	iData = parseNodeInt(pDoc,"/settings/Network/Slots");
	_iPlayerSlots = (iData == -1 || (iData < 0 || iData > 255) ? _iPlayerSlots : iData);


	//Server
	parseNodeString(pDoc,"/settings/Server/Description",_sServerDescription,"Server description not found!");
	parseNodeString(pDoc,"/settings/Server/motd",_sMOTD,"MOTD not defined!");

	iData = parseNodeInt(pDoc,"/settings/Server/OnlineMode");
	_fOnlineMode = (iData == -1 || iData != 0 ? true : false); //Activate if node doesnt exists

	iData = parseNodeInt(pDoc,"/settings/Server/Whitelist"); 
	_fWhitelist = (iData == -1 || iData == 0 ? false : true); //Deactivate if node doesnt exists

	iData = parseNodeInt(pDoc,"/settings/Server/ServerMode");
	_iServerMode = (iData == -1 || iData == 0 ? FC_SERVMODE_SURVIVAL : FC_SERVMODE_CREATIVE);

	iData = parseNodeInt(pDoc,"/settings/Server/Difficulty");
	_iDifficulty = (iData == -1 || (iData < FC_DIFFICULTY_PEACEFUL || iData > FC_DIFFICULTY_HARD) ? FC_DIFFICULTY_EASY : iData);


	//Map
	iData = parseNodeInt(pDoc,"/settings/Map/Height");
	_iWorldHeight = (iData == -1 || (iData < 10 || iData > 255) ? _iWorldHeight : iData);

	iData = parseNodeInt(pDoc,"/settings/Map/AllowNeather");
	_fAllowNeather = (iData == -1 || iData != 0 ? true : false);

	iData = parseNodeInt(pDoc,"/settings/Map/ViewDistance");
	_iViewDistance = (iData == -1 || (iData < 5 || iData > 25) ? 10 : iData);

	parseNodeString(pDoc,"/settings/Map/RootMap",_sMainMapName,"world");


	//Spawning
	iData = parseNodeInt(pDoc,"/settings/Spawning/Peaceful");
	_fSpawnPeacefulMobs = (iData == -1 || iData != 0 ? true : false);

	iData = parseNodeInt(pDoc,"/settings/Spawning/Hostile");
	_fSpawnHostileMobs = (iData == -1 || iData != 0 ? true : false);


	//Player
	iData = parseNodeInt(pDoc,"/settings/Player/AllowPVP");
	_fPVP = (iData == -1 || iData != 0 ? true : false);

	iData = parseNodeInt(pDoc,"/settings/Player/FlyMod/Allow");
	_fAllowFlyMod = (iData == -1 || iData == 0 ? false : true);

	double dData;
	dData = parseNodeDouble(pDoc,"/settings/Player/FlyMod/MaxSpeed");
	_dMaxSpeed = (dData == -1.0 || dData < 7.5 ? 7.5 : dData);


	if (!_fOnlineMode) {	
		cout<<"***Config WARNING: Server runs in unsafe mode. Hackers can connect without verification!"<<"\n";  	
		cout<<"***Solution: Set OnlineMode in your fastcraft.properties to true."<<"\n";
	}

	return;
}


short SettingsHandler::getPort() {
	return short(_iPort); 
}

int SettingsHandler::getPlayerSlotCount() {
	return _iPlayerSlots;
}

string SettingsHandler::getSupportedMCVersion() {
	return "1.1.0";
}

string SettingsHandler::getFastCraftVersion() {
	return "0.0.2";
}

int SettingsHandler::getSupportedProtocolVersion() {
	return 23;
}

string SettingsHandler::getServerDescription() {
	return _sServerDescription;
}

int SettingsHandler::getServerMode() {
	return _iServerMode;
}

char SettingsHandler::getDifficulty() {
	return _iDifficulty;
}

bool SettingsHandler::isOnlineModeActivated() {
	return _fOnlineMode;
}

bool SettingsHandler::isWhitelistActivated() {
	return _fWhitelist;
}

string SettingsHandler::getServerMOTD() {
	return _sMOTD;
}

bool SettingsHandler::isFlyModAllowed() {
	return _fAllowFlyMod;
}

double SettingsHandler::getMaxMovementSpeed() {
	return _dMaxSpeed;
}

unsigned char SettingsHandler::getWorldHeight() {
	return (unsigned char)_iWorldHeight;
}

bool SettingsHandler::isNeatherAllowed() {
	return _fAllowNeather;
}

string SettingsHandler::getMainWorldName() {
	return _sMainMapName;
}

bool SettingsHandler::isPeacefulAnimalSpawningActivated() {
	return _fSpawnPeacefulMobs;
}

bool SettingsHandler::isHostileAnimalSpawningActivated() {
	return _fSpawnHostileMobs;
}

int SettingsHandler::getViewDistance() {
	return _iViewDistance;
}

bool SettingsHandler::isPVPActivated() {
	return _fPVP;
}

int SettingsHandler::parseNodeInt(Poco::AutoPtr<Poco::XML::Document> pDoc,string sPath) { 
	Poco::XML::Node* pNode;

	pNode = pDoc->getNodeByPath(sPath);

	if(pNode==NULL) {
		cout<<"***Error: Node "<<sPath<<" was not found!"<<"\n";
		return -1;
	}else{
		string sData("");
		int iData;

		sData = pNode->innerText();
		try {
			iData = Poco::NumberParser::parseUnsigned(sData);
		}catch(Poco::SyntaxException) {
			cout<<"***Error: Unable to parse data from node "<<sPath<<"\n";
			return -1;
		}
		return iData;
	}
}

double SettingsHandler::parseNodeDouble(Poco::AutoPtr<Poco::XML::Document> pDoc,string sPath) { 
	Poco::XML::Node* pNode;

	pNode = pDoc->getNodeByPath(sPath);

	if(pNode==NULL) {
		cout<<"***Error: Node "<<sPath<<" was not found!"<<"\n";
		return -1.0;
	}else{
		string sData("");
		double dData;

		sData = pNode->innerText();
		try {
			dData = Poco::NumberParser::parseFloat(sData);
		}catch(Poco::SyntaxException) {
			cout<<"***Error: Unable to parse data from node "<<sPath<<"\n";
			return -1.0;
		}
		return dData;
	}
}

void  SettingsHandler::parseNodeString(Poco::AutoPtr<Poco::XML::Document> pDoc,string sPath,string& rTarget,string sFailString) { 
	Poco::XML::Node* pNode;
	pNode = pDoc->getNodeByPath(sPath);

	if (pNode == NULL) {
		cout<<"***Error: Node "<<sPath<<" was not found!"<<"\n";
		rTarget.assign(sFailString);
	}else{
		rTarget.assign(pNode->innerText());
	}
}