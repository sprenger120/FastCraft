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
#define BUILD_VERSION ""
#include <iostream>
#include <vld.h>
#include <Poco/Thread.h>
#include "FCRuntimeException.h"
#include <Poco/Path.h>
#include <Poco/File.h>
#include <Poco/Data/SQLite/Connector.h>
#include <vector>
#include "Constants.h"
#include "MinecraftServer.h"

#if defined(_WIN32)
    #include <Windows.h>
    #include <DbgHelp.h>
#endif


using std::cout;
using Poco::Thread;

int main(int argc, char *argv[]) {
	std::string BuildVer(BUILD_VERSION);
	
	cout<<"FastCraft Minecraft Server"<<"\n";
	cout<<"Version "<<BUILD_VERSION<<(BuildVer.compare("") == 0 ? "" : "-")<<FC_VERSION<<" for Minecraft "<<FC_SUPPORTED_MINCRAFTVERSION<<"\n"<<std::endl;

	Poco::Path pathRoot(argv[0]); 
	pathRoot.setFileName(""); //Remove filename

	pathRoot.pushDirectory("Server");
	Poco::File fileRootDirectory(pathRoot.toString());
	if (!fileRootDirectory.exists()) {
		try {
			fileRootDirectory.createDirectories();
		}catch(Poco::FileException& ex) {
			cout<<"Unable to create server directory ("<<ex.message()<<")"<<std::endl;
		}
	}else{
		if ((!fileRootDirectory.canRead()) || (!fileRootDirectory.canWrite())) {
			cout<<"Unable to read or write FastCraft root directory"<<std::endl;
			Thread::sleep(3000);
			return 0;
		}
	}


	Poco::Data::SQLite::Connector::registerConnector(); //Startup sqlite engine
	Constants::init(); //load constants
	
	#if defined(_WIN32)
		if (!SymInitialize(GetCurrentProcess(),NULL,true)) { std::cout<<"Unable to init Sym\n";}
	#endif


	std::vector<MinecraftServer*> vpServer(0);
	std::vector<Poco::File>		  vFileList(0);

	fileRootDirectory.list(vFileList);
	if (vFileList.empty()) {
		cout<<"No server configurations found!"<<std::endl;
		Thread::sleep(3000);
		return 0;
	}

	MinecraftServer* pServer;
	Poco::Path pathTemp;
	vector<unsigned short> vUsedPorts(0);
	int x;

	//Start all servers
	for (x=0;x<=vFileList.size()-1;x++) {
		if (!vFileList[x].isDirectory()) {continue;} //Skip files

		if(!pathTemp.tryParse(vFileList[x].path())) {
			cout<<"Illegal path!\n"<<std::endl;
			continue;
		}

		try {
			cout<<"Starting "<<pathTemp[pathTemp.depth()]<<"\n";
			pathTemp.pushDirectory(pathTemp[pathTemp.depth()]);
			pathTemp.setFileName("");
			pServer = new MinecraftServer(pathTemp[pathTemp.depth()-1],pathTemp,vUsedPorts);
		}catch(FCRuntimeException& ex) {
			cout<<"Unable to start server ("<<ex.getMessage()<<")\n"<<std::endl;
			continue;
		}
		vpServer.push_back(pServer);
		pathTemp.clear();
	}

	cout<<"Loading done!\n";

	bool fSomethingRuns = false;	
	while(1) {
		Thread::sleep(100);

		//Check if there is at least one server that runs
		fSomethingRuns=false;
		if (!vpServer.empty()) {
			for (x=vpServer.size()-1;x>=0;x--) {
				if (vpServer[x]->isRunning()) {fSomethingRuns=true;}
				if (vpServer[x]->isMarkedForShutdown()) {
					delete vpServer[x];
					vpServer.erase(vpServer.begin()+x);
				}
			}
		}else {
			break;
		}
		if (!fSomethingRuns) {break;}
	}

	Poco::Data::SQLite::Connector::unregisterConnector();
	return 1;
}
