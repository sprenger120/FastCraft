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
#include <Poco/Thread.h>
#include "FCRuntimeException.h"
#include <Poco/Path.h>
#include <Poco/File.h>
#include <Poco/Data/SQLite/Connector.h>
#include <vector>
#include "Constants.h"
#include "MinecraftServer.h"
//#include <vld.h>
//#include "EntityPlayer.h"
//#include "PlayerPool.h"
//#include "PlayerThread.h"
//#include "EntityPickup.h"
#if defined(_WIN32)
    #include <Windows.h>
    #include <DbgHelp.h>
	#include "NetworkOutRoot.h"
	#include "NetworkIn.h"
#include <sstream>
#endif


using std::cout;
using Poco::Thread;

int main(int argc, char *argv[]) {
	std::string BuildVer(BUILD_VERSION);
	

	cout<<"______        _   _____            __ _   \n";
	cout<<"|  ___|      | | /  __ \\          / _| |  \n";
	cout<<"| |_ __ _ ___| |_| /  \\/_ __ __ _| |_| |_ \n";
	cout<<"|  _/ _` / __| __| |   | '__/ _` |  _| __|\n";
	cout<<"| || (_| \\__ \\ |_| \\__/\\ | | (_| | | | |_ \n";
	cout<<"\\_| \\__,_|___/\\__|\\____/_|  \\__,_|_|  \\__|\n";
	cout<<"        Minecraft vServer Software\n";
	cout<<"Core version:"<<BUILD_VERSION<<(BuildVer.compare("") == 0 ? "" : "-")<<FC_VERSION<<" for Minecraft "<<FC_SUPPORTED_MINCRAFTVERSION<<"\n"<<std::endl;

	
	/*  Set up core components */
	cout<<"--- Init core components---\n";
	cout<<"SQLite...";
	Poco::Data::SQLite::Connector::registerConnector();
	cout<<" Done\n";

	Constants::init();
	
	#if defined(_WIN32)
		cout<<"SYM...";
		if (!SymInitialize(GetCurrentProcess(),NULL,true)) { 
			std::cout<<" Failed";
		}else{
			cout<<" Done\n";
		}
	#endif

	cout<<std::endl<<"\n";

	/* check server directory */
	Poco::Path pathRoot(argv[0]); 
	pathRoot.setFileName("");

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



	/* Load vServers */
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
			cout<<"--- Starting vServer: "<<pathTemp[pathTemp.depth()]<<" ---\n";
			pathTemp.pushDirectory(pathTemp[pathTemp.depth()]);
			pathTemp.setFileName("");
			pServer = new MinecraftServer(pathTemp[pathTemp.depth()-1],pathTemp,vUsedPorts);
		}catch(FCRuntimeException& ex) {
			cout<<"Unable to start server ("<<ex.getMessage()<<")\n"<<std::endl;
			continue;
		}
		vpServer.push_back(pServer);
		pathTemp.clear();
		
		if (x<vFileList.size()-1) {cout<<std::endl<<"\n";}
	}


	cout<<std::endl<<"\n--- Done ---\n";

	//Thread::sleep(5000);
	////EntityLiving e(Constants::get("/Entity/Alive/TypeID/ZombiePigman"),vpServer[0],vpServer[0]->getWorldByName("world"));
	////EntityPlayer e(vpServer[0],vpServer[0]->getWorldByName("world"),"testy");
	//EntityPickup e(vpServer[0],vpServer[0]->getWorldByName("world"),std::make_pair<short,char>(35,15));
	//e.Coordinates.X = 0;
	//e.Coordinates.Z = 0;
	//e.Coordinates.Y = 61;
	//
	//for (x=0;x<=5;x++) {
	//	for (int z=0;z<=5;z++){
	//		EntityPickup e(vpServer[0],vpServer[0]->getWorldByName("world"),std::make_pair<short,char>(1,0));
	//		e.Coordinates.X = double(x);
	//		e.Coordinates.Z = double(z);
	//		e.Coordinates.Y = 61;
	//		vpServer[0]->getPlayerPool()->getPlayerByName("Godofcode120",NULL)->spawnEntity(&e);
	//	}
	//}


	bool fSomethingRuns = false;	
	while(1) {
		#if defined(_WIN32)
			std::wstringstream wSS;
			wSS<<"FastCraft    Traffic I/O:"<<   double(NetworkIn::getReadTraffic() + NetworkOutRoot::getWriteTraffic()) / double(1024 * 1024)<<" MB";
			SetConsoleTitle(wSS.str().c_str());
		#endif


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
