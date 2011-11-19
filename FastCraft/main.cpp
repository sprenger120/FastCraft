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
#include <string>

#include <Poco/Thread.h>
#include <Poco/Exception.h>
#include "NetworkHandler.h"
#include "SettingsHandler.h"
#include "PlayerThread.h"
#include "EntityProvider.h"
#include "ChunkProvider.h"
#include "NetworkIO.h"
#include "PlayerThread.h"
#include <Poco/NumberFormatter.h>

using std::cout;
using std::string;
using std::wstring;
using std::endl;
using Poco::Thread;

std::wstring s2ws(const std::string& s)
{
	//http://codereview.stackexchange.com/questions/419/converting-between-stdwstring-and-stdstring
    int len;
    int slength = (int)s.length() + 1;
    len = MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, 0, 0); 
    wchar_t* buf = new wchar_t[len];
    MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, buf, len);
    std::wstring r(buf);
    delete[] buf;
    return r;
}


int main() {
	SettingsHandler* pSettingHandler;

	cout<<"--- FAST CRAFT v. "<<SettingsHandler::getFastCraftVersion()<<" for Minecraft "<<SettingsHandler::getSupportedMCVersion()<<" ---"<<"\n";

	try {
		pSettingHandler = new SettingsHandler; //Load configuration
	} catch (Poco::RuntimeException) {
		Thread::sleep(3000);
		return 1;
	}
	cout<<"Running on *:"<<pSettingHandler->getPort()<<" with "<<pSettingHandler->getMaxClients()<<" player slots"<<endl;


	//Start Network Thread
	NetworkHandler NetworkHandler(pSettingHandler);
	Thread threadNetworkHandler("NetworkHandler");
	threadNetworkHandler.start(NetworkHandler);

	cout<<"Ready."<<"\n"<<endl;

	std::wstring sConsole(L"");
	string sTemp("");

	while (1) {		
		sTemp.clear();
		Poco::NumberFormatter::append(sTemp,double(NetworkIO::getIOTraffic()) / 1024.0 / 1024.0,4);
		sTemp.append(" MB | Player: ");
		Poco::NumberFormatter::append(sTemp,PlayerThread::getConnectedPlayers());
		sTemp.append("/");
		Poco::NumberFormatter::append(sTemp,pSettingHandler->getMaxClients());

		sConsole.assign(L"Fastcraft | IO: " + s2ws(sTemp));
		SetConsoleTitle(sConsole.c_str());

		Thread::sleep(1000);
	}

	delete pSettingHandler;

	return 1;
}