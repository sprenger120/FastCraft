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
#include "MinecraftServer.h"
#include <Poco/String.h>
#include <Poco/File.h>
#include <fstream>
#include "FCRuntimeException.h"
#include <Poco/DOM/DOMParser.h>
#include <Poco/SAX/InputSource.h>
#include <Poco/XML/XMLException.h>
#include <Poco/SAX/SAXException.h>
#include <Poco/NumberParser.h>
#include "Constants.h"
#include "ItemInformationProvider.h"
#include "World.h"
#include "PlayerPool.h"
#include "AcceptThread.h"
#include <ctime>
#include <Poco/ScopedLock.h>
using std::cout;

MinecraftServer::MinecraftServer(string sName,Poco::Path sRootPath,vector<unsigned short>& rvUsedPorts) :
_sServerName(""),
	_sServerDescription		("FastCraft Minecraft server"),
	_sMOTD					("§dWelcome to FastCraft 0.0.2 Alpha server."),
	_sMainMapName			("main"),
	_sServerFullMessage     ("Server is full!"),
	_vsAdditionalWorlds		(0),
	_vpWorlds				(0),
	ServerThreadBase		("Minecraft Server"),
	_iInGameTime			(0)
{
	_iPort						= 25565;
	_iPlayerSlotCount			= 16;
	_iServerMode				= Constants::get("/ServerMode/Survival") == 0 ? false : true;
	_iDifficulty				= Constants::get("/Difficulty/Easy");
	_fOnlineMode				= true;
	_fWhitelist					= false;
	_fAllowNeather				= false;
	_fLoadEnd					= true;
	_fSpawnPeacefulMobs			= true;
	_fSpawnHostileMobs			= true;
	_fPVP						= true;
	_fAllowFlyMod				= false;
	_dMaxFlyingSpeed			= 1.0;
	_pAcceptThread				= NULL;
	_pPlayerPool				= NULL;
	_pItemInformationProvider	= NULL;
	_fMarkedForShutdown			= false;

	_clockCreation.start();
	_iID = 100;

	//Check name
	if (sName.compare("") == 0) {throw FCRuntimeException("Illegal name");}

	
	{
		//Read config
		cout<<"Reading configuration...\n";
		
		sRootPath.setFileName("settings.xml");
		Poco::File ckeckExistanceFile(sRootPath);
		
		if (!ckeckExistanceFile.exists()) {
			cout<<"Settings file doesn't exist. Starting with standart configuration.";
		}else if(!(ckeckExistanceFile.canRead() && ckeckExistanceFile.canWrite())){
			cout<<"Unable to read/write settings file. Starting with standart configuration.\n";
		}else{
			switch (readConfiguration(sRootPath)) {
			case FC_VSERVERERROR_INACTIVE:
				throw FCRuntimeException("Inactive");
			case FC_VSERVERERROR_ERROR:
				throw FCRuntimeException("Unable to read configuration");
			}
		}
	}

	//Check port availability
	if (!rvUsedPorts.empty()) {
		for(int x = 0; x<= rvUsedPorts.size()-1;x++) {
			if (rvUsedPorts[x] == _iPort) {throw FCRuntimeException("Port already in use!");}
		}
	}
		

	{
		//Reading items.db
		cout<<"Reading item database...\n";
		sRootPath.setFileName("items.db");
		Poco::File ckeckExistanceFile(sRootPath);


		if (!ckeckExistanceFile.exists()) {
			cout<<"Item database doesn't exist. Aborting server startup!";
			return;
		}
		if(!(ckeckExistanceFile.canRead() && ckeckExistanceFile.canWrite())){
			cout<<"Unable to read/write item database. Aborting server startup!\n";
			return;
		}
		_pItemInformationProvider = new ItemInformationProvider(sRootPath);
	}
	

	//load words
	cout<<"Loading worlds...\n";
	_vpWorlds.push_back(new World(_sMainMapName,0,this));
	if (!_vsAdditionalWorlds.empty()) {
		for(int x=0;x<=_vsAdditionalWorlds.size()-1;x++) {
			_vpWorlds.push_back(new World(_vsAdditionalWorlds[x],0,this));
		}
	}


	//start playerpool
	cout<<"Init player factory...\n";
	_pPlayerPool = new PlayerPool(this);


	//start accept thread
	try{
		cout<<"Init network...\n";
		_pAcceptThread = new AcceptThread(this);
		rvUsedPorts.push_back(_iPort);
	}catch(FCRuntimeException& ex) { 
		cout<<"Unable to start server: "<< ex.getMessage()<<"\n"<<std::endl;
		return;
	}

	startThread(this);
	cout<<sName<<" started successfully! (Startup time:"
		<<_clockCreation.elapsed()/1000000<<"s "
		<<(_clockCreation.elapsed()/1000)%1000<<"ms"
		<<" ;  Port:"<<_iPort<<")\n"
		<<std::endl;
}

MinecraftServer::~MinecraftServer() {
	if (isRunning()) {cout<<"Shutting down server "<<_sServerName<<"...\n";}
	if (_pAcceptThread != NULL)	{delete _pAcceptThread;}
	if (_pPlayerPool != NULL)	{delete _pPlayerPool;}
	if (_pItemInformationProvider != NULL)	{delete _pItemInformationProvider;}
	if (!_vpWorlds.empty()) {
		for(int x=0;x<=_vpWorlds.size()-1;x++) {
			delete _vpWorlds[x];
		}
	}

	killThread();
}


void MinecraftServer::run() {
	_iThreadStatus = FC_THREADSTATUS_RUNNING;

	/*Poco::Stopwatch _TickClock;
	_TickClock.start();*/


	while(_iThreadStatus==FC_THREADSTATUS_RUNNING)  {
		Poco::Thread::sleep(50);
		_iInGameTime += 1;
	}

	_iThreadStatus = FC_THREADSTATUS_DEAD;
}

char MinecraftServer::readConfiguration(Poco::Path& path) {
	std::ifstream Input(path.toString());

	if (!Input.good()) {
		cout<<"***Error: Configuration path invalid!"<<"\n";
		return FC_VSERVERERROR_ERROR;
	}

	Poco::XML::InputSource ISource(Input);
	Poco::XML::DOMParser parser;
	//parser.setEncoding("ascii");
	Poco::AutoPtr<Poco::XML::Document> pDoc;

	try {
		pDoc = parser.parse(&ISource);
	}catch(Poco::XML::SAXException& ex) {
		cout<<"***Error while parsing settings: "<<ex.message()<<"\n";
		return FC_VSERVERERROR_ERROR;
	}


	/*
	* Parsing settings
	*/
	int iData;
	if (!parseNodeInt(pDoc,"/settings/DocVer",iData,FC_SUPPORTED_DOCVER,FC_SUPPORTED_DOCVER)) {
		cout<<"***Invalid settings DocVer\n";
		return FC_VSERVERERROR_ERROR;
	}else{
		string sTemp("");

		if (parseNodeInt(pDoc,"/settings/Server/Active",iData,0,1)) {
			if (iData==0) {return FC_VSERVERERROR_INACTIVE;}
		}else{
			return FC_VSERVERERROR_ERROR;
		}

		if(parseNodeInt(pDoc,"/settings/Network/Port",iData,0,65535)) {_iPort = short(iData);} 
		if(parseNodeInt(pDoc,"/settings/Network/Slots",iData,1,255)) {_iPlayerSlotCount = short(iData);}
		parseNodeString(pDoc,"/settings/Server/Description",_sServerDescription);
		parseNodeString(pDoc,"/settings/Server/motd",_sMOTD);
		parseNodeString(pDoc,"/settings/Server/ServerFullMessage",_sServerFullMessage);
		if(parseNodeInt(pDoc,"/settings/Server/OnlineMode",iData,0,1)) {_fOnlineMode = Int2Bool(iData);}
		if(parseNodeInt(pDoc,"/settings/Server/Whitelist",iData,0,1)) {_fWhitelist = Int2Bool(iData);}
		if(parseNodeInt(pDoc,"/settings/Server/ServerMode",iData,0,1)) {_iServerMode = Int2Bool(iData);}
		if(parseNodeInt(pDoc,"/settings/Server/Difficulty",iData,0,1)) {_iDifficulty = (char)iData;}
		if(parseNodeInt(pDoc,"/settings/Map/AllowNeather",iData,0,1)) {_fAllowNeather = Int2Bool(iData);}
		parseNodeString(pDoc,"/settings/Map/RootMap",_sMainMapName);
		if(parseNodeInt(pDoc,"/settings/Map/loadEnd",iData,0,1)) {_fLoadEnd = Int2Bool(iData);}
		if(parseNodeString(pDoc,"/settings/Map/AdditionalWorlds",sTemp)) {
			if (!sTemp.empty()) {
				string sPart("");
				for (int x=0;x<=sTemp.size()-1;x++) {
					if (sTemp[x] == ',') {
						if(sPart.compare("") != 0) {
							_vsAdditionalWorlds.push_back(sPart);
							sPart.clear();
							continue;
						}
					}else{
						sPart.append(1,sTemp[x]);
					}
				}
				if(sPart.compare("") != 0) {_vsAdditionalWorlds.push_back(sPart);}
			}
		}
		if(parseNodeInt(pDoc,"/settings/Spawning/Peaceful",iData,0,1)) {_fSpawnPeacefulMobs = Int2Bool(iData);}
		if(parseNodeInt(pDoc,"/settings/Spawning/Hostile",iData,0,1)) {_fSpawnHostileMobs = Int2Bool(iData);}
		if(parseNodeInt(pDoc,"/settings/Player/AllowPVP",iData,0,1)) {_fPVP = Int2Bool(iData);}
		//if(parseNodeInt(pDoc,"/settings/Player/FlyMod/AllowFlying",iData,0,1)) {_fAllowFlyMod = Int2Bool(iData);}
		//parseNodeDouble(pDoc,"/settings/Player/FlyMod/MaxFlySpeed",_dMaxFlyingSpeed,1.0,10.0);
		if (!_fOnlineMode) {	
			cout<<"***Config WARNING: Server runs in unsafe mode. Hackers can connect without verification!"<<"\n";  	
			cout<<"***Solution: Set OnlineMode in your settings.xml to true."<<"\n";
		}
	}
	return FC_VSERVERERROR_OK;
}

unsigned short MinecraftServer::getPort() {
	return _iPort; 
}

unsigned short MinecraftServer::getPlayerSlotCount() {
	return _iPlayerSlotCount;
}

string MinecraftServer::getServerDescription() {
	return _sServerDescription;
}

bool MinecraftServer::getServerMode() {
	return _iServerMode;
}

char MinecraftServer::getDifficulty() {
	return _iDifficulty;
}

bool MinecraftServer::isOnlineModeActivated() {
	return _fOnlineMode;
}

bool MinecraftServer::isWhitelistActivated() {
	return _fWhitelist;
}

string MinecraftServer::getServerMOTD() {
	return _sMOTD;
}

//bool MinecraftServer::isFlyModAllowed() {
//	return _fAllowFlyMod;
//}
//
//double MinecraftServer::getMaxFlySpeed() {
//	return _dMaxFlyingSpeed;
//}

bool MinecraftServer::isNeatherAllowed() {
	return _fAllowNeather;
}

string MinecraftServer::getMainWorldName() {
	return _sMainMapName;
}

bool MinecraftServer::isPeacefulAnimalSpawningActivated() {
	return _fSpawnPeacefulMobs;
}

bool MinecraftServer::isHostileAnimalSpawningActivated() {
	return _fSpawnHostileMobs;
}


bool MinecraftServer::isPVPActivated() {
	return _fPVP;
}

bool MinecraftServer::parseNodeInt(Poco::AutoPtr<Poco::XML::Document> pDoc,string sPath,int& rData,int iMin,int iMax) { 
	Poco::XML::Node* pNode;

	pNode = pDoc->getNodeByPath(sPath);

	if(pNode==NULL) {
		cout<<"***Error: Node "<<sPath<<" was not found!"<<"\n";
		return false;
	}else{
		string sData("");
		int iData;

		sData = pNode->innerText();
		try {
			iData = Poco::NumberParser::parseUnsigned(sData);
		}catch(Poco::SyntaxException) {
			cout<<"***Error: Unable to parse data from node "<<sPath<<"\n";
			return false;
		}

		if (iData < iMin || iData > iMax) {
			cout<<"***Error: Value of "<<sPath<<" is invalid!\n";
			return false;
		}
		rData = iData;
		return true;
	}
}

bool MinecraftServer::parseNodeDouble(Poco::AutoPtr<Poco::XML::Document> pDoc,string sPath,double& rData,double dMin,double dMax) { 
	Poco::XML::Node* pNode;

	pNode = pDoc->getNodeByPath(sPath);

	if(pNode==NULL) {
		cout<<"***Error: Node "<<sPath<<" was not found!"<<"\n";
		return false;
	}else{
		string sData("");
		double dData;

		sData = pNode->innerText();
		try {
			dData = Poco::NumberParser::parseFloat(sData);
		}catch(Poco::SyntaxException) {
			cout<<"***Error: Unable to parse data from node "<<sPath<<"\n";
			return false;
		}

		if (dData < dMin || dData > dMax) {
			cout<<"***Error: Value of "<<sPath<<" is invalid!\n";
			return false;
		}
		rData = dData;
		return true;
	}
}

bool MinecraftServer::parseNodeString(Poco::AutoPtr<Poco::XML::Document> pDoc,string sPath,string& rTarget) { 
	Poco::XML::Node* pNode;
	pNode = pDoc->getNodeByPath(sPath);

	if (pNode == NULL) {
		cout<<"***Error: Node "<<sPath<<" was not found!"<<"\n";
		return false;
	}else{
		rTarget.assign(pNode->innerText());
		return true;
	}
}

bool MinecraftServer::Int2Bool(int& r) {
	if(r){
		return true;
	}else{
		return false;
	}
}

string MinecraftServer::getServerFullMessage() {
	return _sServerFullMessage;
}

ItemInformationProvider* MinecraftServer::getItemInfoProvider() {
	return _pItemInformationProvider;
}

PlayerPool* MinecraftServer::getPlayerPool() {
	return _pPlayerPool;
}

World* MinecraftServer::getWorldByName(string sName) {
	if (!_vpWorlds.empty()) {
		for (int x=0;x<=_vpWorlds.size()-1;x++) {
			if (_vpWorlds[x]->getName().compare(sName) == 0) {
				return _vpWorlds[x];
			}
		}
	}

	return NULL;
}


Tick MinecraftServer::getInGameTime() {
	return _iInGameTime;
}

Tick MinecraftServer::getTimestamp() {
	return _clockCreation.elapsed() / 1000;
}

int MinecraftServer::generateID(){
	Poco::ScopedLock<Poco::Mutex> scopeLock(_idMutex);
	_iID++;
	return _iID;
}

void MinecraftServer::shutdown() {
	_fMarkedForShutdown = true;
}

bool MinecraftServer::isMarkedForShutdown() {
	return _fMarkedForShutdown;
}
