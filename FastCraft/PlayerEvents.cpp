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
#include "PlayerEvents.h"
#include "Constants.h"
#include <Poco/Exception.h>
#include <iostream>
#include "PlayerThread.h"
#include "PlayerPool.h"

/*
* Con/Destructors
*/

PlayerChatEvent::PlayerChatEvent(PlayerThread* pThread,string Message,EntityCoordinates Coordinates) :
PlayerEventBase(pThread),
_sMessage(Message),
_Coordinates(Coordinates)
{
	if (_sMessage.length() > 119) {
		_sMessage.resize(119);
	}
}

PlayerChatEvent::~PlayerChatEvent(){
}


ChatEvent::ChatEvent(string Message) : 
PlayerEventBase(NULL,true),
_sMessage(Message)
{
	if (_sMessage.length() > 119) {
		_sMessage.resize(119);
	}
}

ChatEvent::~ChatEvent(){
}




PlayerJoinEvent::PlayerJoinEvent(PlayerThread* pThread) : 
PlayerEventBase(pThread)
{
}

PlayerJoinEvent::~PlayerJoinEvent(){
}


PlayerDisconnectEvent::PlayerDisconnectEvent(PlayerThread* pThread,int iEID,string sUsername) :
PlayerEventBase(pThread),
_iEntityID(iEID),
_sName(sUsername)
{
	if(_iEntityID <= 0) {
		std::cout<<"PlayerDisconnectEvent::PlayerDisconnectEvent lower or less than zero\n";
	}
}

PlayerDisconnectEvent::~PlayerDisconnectEvent(){
}




PlayerAnimationEvent::PlayerAnimationEvent(PlayerThread* pThread,char iAnimID) : 
PlayerEventBase(pThread),
_iAnimationID(iAnimID)
{
	if (_iAnimationID != 0 &&
		_iAnimationID != 1 &&
		_iAnimationID != 3){
			std::cout<<"PlayerAnimationEvent::PlayerAnimationEvent only AnimID 0,1,3 will send by notchain clients\n";
		}
}

PlayerAnimationEvent::~PlayerAnimationEvent(){
}


PlayerUpdateFlagsEvent::PlayerUpdateFlagsEvent(PlayerThread* pThread,EntityFlags Flags) :
PlayerEventBase(pThread),
_Flags(Flags)
{
}

PlayerUpdateFlagsEvent::~PlayerUpdateFlagsEvent(){
}

PlayerMoveEvent::PlayerMoveEvent(PlayerThread* pThread,EntityCoordinates newCoordinates) : 
PlayerEventBase(pThread),
_newCoordinates(newCoordinates)
{
}

PlayerMoveEvent::~PlayerMoveEvent(){
}



PlayerChangeHeldEvent::PlayerChangeHeldEvent(PlayerThread* pThread,ItemID Item,short iSlot) :
PlayerEventBase(pThread),
_Item(Item),
_iSlot(iSlot),
_fIgnore(false)
{
	if (iSlot < 0 || iSlot > 4) {
		std::cout<<"PlayerChangeHeldEvent::PlayerChangeHeldEvent invalid SlotID!\n";
		_fIgnore=true;
	}
	if (Item.second < 0 || Item.second > 16) {
		std::cout<<"PlayerChangeHeldEvent::PlayerChangeHeldEvent invalid metadata entry!\n";
		_fIgnore=true;
	}
	if(iSlot>0 && Item.first < 255) {
		std::cout<<"PlayerChangeHeldEvent::PlayerChangeHeldEvent item expected, block given\n";
		_fIgnore=true;
	}
}

PlayerChangeHeldEvent::~PlayerChangeHeldEvent(){
}



PlayerSetBlockEvent::PlayerSetBlockEvent(PlayerThread* pThread,BlockCoordinates Coordinates,ItemID Item) :
PlayerEventBase(pThread),
_Coordinates(Coordinates),
_Item(Item),
_fIgnore(false)
{
	if (Item.second < 0 || Item.second > 16) {
		std::cout<<"PlayerSetBlockEvent::PlayerSetBlockEvent invalid metadata entry!\n";
		_fIgnore=true;
	}
	if(Item.first > 255) {
		std::cout<<"PlayerSetBlockEvent::PlayerSetBlockEvent block expected, item given\n";
		_fIgnore=true;
	}
}

PlayerSetBlockEvent::~PlayerSetBlockEvent(){
}

/*
* Execute implementations
*/

void PlayerChatEvent::Execute(vector<PlayerThread*>& rvPlayers,PlayerPool* pPlayerPool) {
	if (rvPlayers.empty()) {return;}
	for (int x=0;x<=rvPlayers.size()-1;x++) {
		if (rvPlayers[x]->isAssigned() && rvPlayers[x]->isSpawned()) {
			rvPlayers[x]->insertChat(_sMessage);
		}
	}
}

void ChatEvent::Execute(vector<PlayerThread*>& rvPlayers,PlayerPool* pPlayerPool) {
}

void PlayerJoinEvent::Execute(vector<PlayerThread*>& rvPlayers,PlayerPool* pPlayerPool) {
}

void PlayerDisconnectEvent::Execute(vector<PlayerThread*>& rvPlayers,PlayerPool* pPlayerPool) {
}

void PlayerAnimationEvent::Execute(vector<PlayerThread*>& rvPlayers,PlayerPool* pPlayerPool) {
}

void PlayerUpdateFlagsEvent::Execute(vector<PlayerThread*>& rvPlayers,PlayerPool* pPlayerPool) {
}

void PlayerMoveEvent::Execute(vector<PlayerThread*>& rvPlayers,PlayerPool* pPlayerPool) {
}

void PlayerChangeHeldEvent::Execute(vector<PlayerThread*>& rvPlayers,PlayerPool* pPlayerPool) {
}

void PlayerSetBlockEvent::Execute(vector<PlayerThread*>& rvPlayers,PlayerPool* pPlayerPool) {
}