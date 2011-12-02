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

#include <iostream>
#include <sstream>

#include <Poco/Thread.h>
#include <Poco/Exception.h>
#include "AcceptThread.h"
#include "SettingsHandler.h"
#include "PlayerThread.h"
#include "NetworkIO.h"
#include <Poco/StreamCopier.h>

using std::cout;
using std::stringstream;
using Poco::Thread;

int main() {
	SettingsHandler* pSettingHandler;

	cout<<"--- FAST CRAFT v. "<<SettingsHandler::getFastCraftVersion()<<" for Minecraft "<<SettingsHandler::getSupportedMCVersion()<<" ---"<<"\n";

	try {
		pSettingHandler = new SettingsHandler; //Load configuration
	} catch (Poco::RuntimeException) {
		Thread::sleep(3000);
		return 1;
	}
	cout<<"Running on *:"<<pSettingHandler->getPort()<<" with "<<pSettingHandler->getMaxClients()<<" player slots"<<std::endl;


	//Start Network Thread
	AcceptThread AcceptThread(pSettingHandler);
	Thread threadAcceptThread("AcceptThread");
	threadAcceptThread.start(AcceptThread);

	cout<<"Ready."<<"\n"<<std::endl;

	stringstream StrStrm;
	while (1) {		
		StrStrm.clear();

		StrStrm<<"IO:"
			   <<NetworkIO::getIOTraffic() / 1024 / 1024
			   <<" MB | Player: "
			   <<PlayerThread::getConnectedPlayers()
			   <<"/"
			   <<pSettingHandler->getMaxClients()
			   <<" | Time: "
			   <<ServerTime::getTimeFormated()
			   <<"\n";
		

		Poco::StreamCopier::copyStream(StrStrm,cout);
		Thread::sleep(1000);
	}

	delete pSettingHandler;
	return 1;
}