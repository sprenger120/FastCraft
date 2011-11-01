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

#include <Poco\Thread.h>
#include <Poco\ThreadPool.h>
#include <Poco\Net\ServerSocket.h>

#include "NetworkHandler.h"
#include "SettingsHandler.h"

using std::cout;
using std::string;
using std::endl;
int main() {
	SettingsHandler Settings;
	
	//Informations
	cout<<"--- FAST CRAFT v. "+ Settings.getFastCraftVersion() + " for Minecraft " + Settings.getSupportedMCVersion() + " ---"<<"\n";
	cout<<"Running on *:";
	cout<<Settings.getPort();
	cout<<" with ";
	cout<<Settings.getMaxClients();
	cout<<" slots"<<endl;




	

	return 1;
}