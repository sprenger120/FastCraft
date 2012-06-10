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
#include "FCRuntimeException.h"
#include <iostream>
#include "PlayerThread.h"
#include "PlayerPool.h"
#include "EntityPlayer.h"
#include "MathHelper.h"
#include "ChunkMath.h"
#include "FCRuntimeException.h"
#include "World.h"

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
	if(_iEntityID <= 100) {
		throw FCRuntimeException("invalid EID");
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
		throw FCRuntimeException("invalid SlotID");
		_fIgnore=true;
	}
	if (Item.second < 0 || Item.second > 15) {
		throw FCRuntimeException("invalid ID");
		_fIgnore=true;
	}
	
	if (!_pSourcePlayer->getMinecraftServer()->getItemInfoProvider()->isRegistered(Item)) {
		throw FCRuntimeException("Item not registered");
		_fIgnore=true;
	}

}

PlayerChangeHeldEvent::~PlayerChangeHeldEvent(){
}



PlayerSetBlockEvent::PlayerSetBlockEvent(BlockCoordinates Coordinates,ItemID Item,World* pWorld) :
PlayerEventBase(NULL,true),
	_Coordinates(Coordinates),
	_Item(Item)
{
	_pWorld = pWorld;
}

PlayerSetBlockEvent::~PlayerSetBlockEvent(){
}

/*
* Execute implementations
*/

void PlayerChatEvent::Execute(vector<PlayerThread*>& rvPlayers,PlayerPool* pPlayerPool) {
	if (rvPlayers.empty()) {return;}
	for (int x=0;x<=rvPlayers.size()-1;x++) {
		if (!(rvPlayers[x]->isAssigned() && rvPlayers[x]->isSpawned())) {continue;}

		rvPlayers[x]->insertChat(_sMessage);
	}
}

void ChatEvent::Execute(vector<PlayerThread*>& rvPlayers,PlayerPool* pPlayerPool) {
	if (rvPlayers.empty()) {return;}
	for (int x=0;x<=rvPlayers.size()-1;x++) {
		if (!(rvPlayers[x]->isAssigned() && rvPlayers[x]->isSpawned())) {continue;}

		rvPlayers[x]->insertChat(_sMessage);
	}
}

void PlayerJoinEvent::Execute(vector<PlayerThread*>& rvPlayers,PlayerPool* pPlayerPool) {
	if (rvPlayers.empty()) {return;}

	//Write join message to chat
	pPlayerPool->sendMessageToAll( _pSourcePlayer->getUsername() + " joined game" );
	EntityPlayer NewPlayer(_pSourcePlayer);

	for (int x=0;x<=rvPlayers.size()-1;x++) {
		if (!(rvPlayers[x]->isAssigned() && rvPlayers[x]->isSpawned())) {continue;} /* Not spawned */
		if (rvPlayers[x] == _pSourcePlayer) {continue;} /* dont spawn yourself to yourself */

		rvPlayers[x]->PlayerInfoList(true,_pSourcePlayer->getUsername()); /* spawn name to playerlist */

		if (MathHelper::distance2D(rvPlayers[x]->getCoordinates(),_pSourcePlayer->getCoordinates()) > 100.0) {continue;} /* You're too distant */
		rvPlayers[x]->spawnEntity(&NewPlayer); /* Spawn entity */
		
		/* Spawn other players to new player */
		EntityPlayer otherPlayer(rvPlayers[x]);
		_pSourcePlayer->spawnEntity(&otherPlayer);
	}
}

void PlayerDisconnectEvent::Execute(vector<PlayerThread*>& rvPlayers,PlayerPool* pPlayerPool) {
	if (rvPlayers.empty()) {return;}

	//Write disconnect message to chat
	pPlayerPool->sendMessageToAll(_sName + " left game" );

	//Despawn player 
	for (int x=0;x<=rvPlayers.size()-1;x++) {
		if (!(rvPlayers[x]->isAssigned() && rvPlayers[x]->isSpawned())) {continue;}
		if (rvPlayers[x] == _pSourcePlayer) {continue;}

		rvPlayers[x]->PlayerInfoList(false,_sName); //despawn name

		if (rvPlayers[x]->isEntitySpawned(_iEntityID)) {rvPlayers[x]->despawnEntity(_iEntityID);}
	}
}

void PlayerAnimationEvent::Execute(vector<PlayerThread*>& rvPlayers,PlayerPool* pPlayerPool) {
	if (rvPlayers.empty()) {return;}
	int iEID = _pSourcePlayer->getEntityID();

	for (int x=0;x<=rvPlayers.size()-1;x++) {
		if (!(rvPlayers[x]->isAssigned() && rvPlayers[x]->isSpawned())) {continue;}
		if (rvPlayers[x] == _pSourcePlayer) {continue;}

		if (rvPlayers[x]->isEntitySpawned(iEID)) {
			rvPlayers[x]->playAnimationOnEntity(iEID,_iAnimationID);
		}
	}
}

void PlayerUpdateFlagsEvent::Execute(vector<PlayerThread*>& rvPlayers,PlayerPool* pPlayerPool) {
	if (rvPlayers.empty()) {return;}

	EntityPlayer Player(_pSourcePlayer);

	for (int x=0;x<=rvPlayers.size()-1;x++) {
		if (!(rvPlayers[x]->isAssigned() && rvPlayers[x]->isSpawned())) {continue;}
		if (rvPlayers[x] == _pSourcePlayer) {continue;}

		if (rvPlayers[x]->isEntitySpawned(_pSourcePlayer->getEntityID())) {
			rvPlayers[x]->updateEntityMetadata(&Player);
		}
	}
}

void PlayerMoveEvent::Execute(vector<PlayerThread*>& rvPlayers,PlayerPool* pPlayerPool) {
	if (rvPlayers.empty()) {return;}

	EntityPlayer Player(_pSourcePlayer);

	for (int x=0;x<=rvPlayers.size()-1;x++) {
		if (!(rvPlayers[x]->isAssigned() && rvPlayers[x]->isSpawned())) {continue;} /* not spawned */
		if (rvPlayers[x] == _pSourcePlayer) {continue;} /* filter yourself */

		if (MathHelper::distance2D(rvPlayers[x]->getCoordinates(),_newCoordinates) > 100.0) { /* Too distant */
			if (rvPlayers[x]->isEntitySpawned(_pSourcePlayer->getEntityID())) { /* despawn if spawned */
				rvPlayers[x]->despawnEntity(_pSourcePlayer->getEntityID());
			}
			continue; 
		}

		if (!rvPlayers[x]->isEntitySpawned(_pSourcePlayer->getEntityID())) { //Spawn into players view circle
			rvPlayers[x]->spawnEntity(&Player);
		}else{ //Already spawned -> update position
			rvPlayers[x]->updateEntityPosition(&Player);	  	
		}
	}
}

void PlayerChangeHeldEvent::Execute(vector<PlayerThread*>& rvPlayers,PlayerPool* pPlayerPool) {
	if (rvPlayers.empty()) {return;}
	if (_fIgnore) {return;}

	EntityPlayer Player(_pSourcePlayer);

	for (int x=0;x<=rvPlayers.size()-1;x++) {
		if (!(rvPlayers[x]->isAssigned() && rvPlayers[x]->isSpawned())) {continue;}
		if (rvPlayers[x] == _pSourcePlayer) {continue;}

		if (rvPlayers[x]->isEntitySpawned(_pSourcePlayer->getEntityID())) {
			rvPlayers[x]->updateEntityEquipment(&Player);
		}
	}
}

void PlayerSetBlockEvent::Execute(vector<PlayerThread*>& rvPlayers,PlayerPool* pPlayerPool) {
	if (rvPlayers.empty()) {return;}

	ChunkCoordinates PlayerChkCoords;
	ChunkCoordinates BlockChkCoords = ChunkMath::toChunkCoords(_Coordinates);

	for (int x=0;x<=rvPlayers.size()-1;x++) {
		if (!(rvPlayers[x]->isAssigned() && rvPlayers[x]->isSpawned())) {continue;}
		if (rvPlayers[x]->getWorld() != _pWorld) { continue;} //Don't accept spawn block events from other worlds


		PlayerChkCoords = ChunkMath::toChunkCoords(rvPlayers[x]->getCoordinates());	
		if (int(MathHelper::distance2D(PlayerChkCoords,BlockChkCoords)) <= FC_VIEWDISTANCE) {
			rvPlayers[x]->spawnBlock(_Coordinates,_Item);
		}
	}
}
