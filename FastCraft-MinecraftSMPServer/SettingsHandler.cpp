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

#include "SettingsHandler.h"


SettingsHandler::SettingsHandler() {

}

SettingsHandler::~SettingsHandler() {

}


short SettingsHandler::getPort() {
	return 25565; 
}

int SettingsHandler::getMaxClients() {
	return 2;
}


std::string SettingsHandler::getSupportedMCVersion() {
	return "1.8.1";
}

std::string SettingsHandler::getFastCraftVersion() {
	return "0.0.1";
}

int SettingsHandler::getSupportedProtocolVersion() {
	return 17; //Minecraft 1.8
}