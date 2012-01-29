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

#include "PlayerPool.h"
#include "PlayerThread.h"
#include "SettingsHandler.h"
#include "PackingThread.h"
#include "EntityFlags.h"
#include "MathHelper.h"
#include "PlayerInventory.h"
#include "EntityCoordinates.h"
#include <Poco/Thread.h>
#include <Poco/Exception.h>
#include <iostream>
#include <cmath>
#include <Poco/String.h>
#include "ChunkMath.h"
#include "WorldStorage.h"

using Poco::Thread;
using std::cout;

PlayerPool::PlayerPool(PackingThread& rPackingThread):
_vPlayerThreads(0),
	_ThreadPool("PlayerThreads",1,SettingsHandler::getPlayerSlotCount()),
	_qEventQueue(),
	_PackingThread(rPackingThread) 
{	
	int iSlotCount = SettingsHandler::getPlayerSlotCount();

	_vPlayerThreads.resize(iSlotCount); //Resize to slot count
	for (int x=0;x<=_vPlayerThreads.size()-1;x++) {
		_vPlayerThreads[x] = NULL;
	}
}

PlayerPool::~PlayerPool() {
	_ThreadPool.stopAll(); //Stop all threads

	for (int x=0;x<=_vPlayerThreads.size()-1;x++) { //Release objects
		if (_vPlayerThreads[x] != NULL) {
			delete _vPlayerThreads[x];
		}
	}
}


void PlayerPool::run() {
	std::string sData;

	//Create Player Threads
	for (int x=0;x<=_vPlayerThreads.size()-1;x++) {
		_vPlayerThreads[x] = new PlayerThread(this,_PackingThread);

		_ThreadPool.defaultPool().start(*_vPlayerThreads[x]);
	}

	while (1) {
		if (_qEventQueue.empty()) {
			Thread::sleep(50);
			continue;
		}
		try {

			PlayerPoolEvent& Event = _qEventQueue.front();


			switch (Event.getJobID()) {
			case FC_PPEVENT_CHAT:
				if(Event.getPtr()->isSpawned()) { //Not a kick message
					cout<<"Chat: "<<Event.getMessage()<<std::endl; //Server console
				}
				sendMessageToAll(Event.getMessage());
				break;
			case FC_PPEVENT_MOVE:
				{
					//Build playerEntity
					EntityPlayer SourcePlayer = buildEntityPlayerFromPlayerPtr(Event.getPtr());
					int SourcePlayerID = Event.getPtr()->getEntityID();

					for (int x=0;x<=_vPlayerThreads.size()-1;x++) {
						if( _vPlayerThreads[x]->isAssigned() && _vPlayerThreads[x]->isSpawned()) {
							if (_vPlayerThreads[x] == Event.getPtr() ) {continue;}

							//Check distance 
							if ( MathHelper::distance2D(_vPlayerThreads[x]->getCoordinates(),Event.getCoordinates()) > FC_PLAYERSPAWNRADIUS) {
								if (_vPlayerThreads[x]->isEntitySpawned(SourcePlayerID)) { //Despawn player, too distant
									_vPlayerThreads[x]->despawnEntity(SourcePlayerID);
								}
								continue;
							}

							if (!_vPlayerThreads[x]->isEntitySpawned(SourcePlayerID)) { //Spawn into players view circle
								_vPlayerThreads[x]->spawnPlayer(SourcePlayerID,SourcePlayer);
							}else{ //Already spawned -> update position
								_vPlayerThreads[x]->updateEntityPosition(SourcePlayerID,Event.getCoordinates());
							}
						}
					}

				}
				break;
			case FC_PPEVENT_JOIN:
				{
					sendMessageToAll(Event.getName() + " joined game");

					EntityPlayer SourcePlayer = buildEntityPlayerFromPlayerPtr(Event.getPtr());
					int SourcePlayerID = Event.getPtr()->getEntityID();


					for (int x=0;x<=_vPlayerThreads.size()-1;x++) {
						if( _vPlayerThreads[x]->isAssigned() && _vPlayerThreads[x]->isSpawned()) {
							_vPlayerThreads[x]->PlayerInfoList(true,Event.getName()); //Update player list

							if (_vPlayerThreads[x] == Event.getPtr() ) {continue;} //dont spawn event source
							if (MathHelper::distance2D(_vPlayerThreads[x]->getCoordinates(),SourcePlayer._Coordinates) > 100.0) {continue;} //Too distant -> don't spawn

							_vPlayerThreads[x]->spawnPlayer(SourcePlayerID,SourcePlayer); //Spawn new player	

							//Spawn this player to event source
							EntityPlayer TargetPlayer = buildEntityPlayerFromPlayerPtr(_vPlayerThreads[x]); 
							int TargetPlayerID = _vPlayerThreads[x]->getEntityID();

							Event.getPtr()->spawnPlayer(TargetPlayerID,TargetPlayer);
						}
					}
				}
				break;
			case FC_PPEVENT_DISCONNECT:
				{
					sendMessageToAll(Event.getName() + " left game");
					int SourcePlayerID = Event.getPtr()->getEntityID(); //Disconnect don't clear the entity so we can use it 

					for (int x=0;x<=_vPlayerThreads.size()-1;x++) {
						if( _vPlayerThreads[x]->isAssigned() && _vPlayerThreads[x]->isSpawned()) {
							_vPlayerThreads[x]->PlayerInfoList(false,Event.getName());
							if (_vPlayerThreads[x] == Event.getPtr() ) {continue;}

							if (_vPlayerThreads[x]->isEntitySpawned(SourcePlayerID)) {
								_vPlayerThreads[x]->despawnEntity(SourcePlayerID);
							}
						}
					}
				}
				break;
			case FC_PPEVENT_ANIMATION:
				{
					int SourcePlayerID = Event.getPtr()->getEntityID(); 

					for (int x=0;x<=_vPlayerThreads.size()-1;x++) {
						if( _vPlayerThreads[x]->isAssigned() && _vPlayerThreads[x]->isSpawned()) {
							if (_vPlayerThreads[x] == Event.getPtr() ) {continue;}

							if (_vPlayerThreads[x]->isEntitySpawned(SourcePlayerID)) {
								_vPlayerThreads[x]->playAnimationOnEntity(SourcePlayerID,Event.getAnimationID());
							}
						}
					}
				}
				break;
			case FC_PPEVENT_METADATA: 
				{
					int SourcePlayerID = Event.getPtr()->getEntityID(); 
					for (int x=0;x<=_vPlayerThreads.size()-1;x++) {
						if( _vPlayerThreads[x]->isAssigned() && _vPlayerThreads[x]->isSpawned()) {
							if (_vPlayerThreads[x] == Event.getPtr() ) {continue;}

							if (_vPlayerThreads[x]->isEntitySpawned(SourcePlayerID)) {
								_vPlayerThreads[x]->updateEntityMetadata(SourcePlayerID,Event.getFlags());
							}
						}
					}
				}
				break;
			case FC_PPEVENT_CHANGEHELD:
				{
					int SourcePlayerID = Event.getPtr()->getEntityID(); 
					for (int x=0;x<=_vPlayerThreads.size()-1;x++) {
						if( _vPlayerThreads[x]->isAssigned() && _vPlayerThreads[x]->isSpawned()) {
							if (_vPlayerThreads[x] == Event.getPtr() ) {continue;}

							if (_vPlayerThreads[x]->isEntitySpawned(SourcePlayerID)) {
								_vPlayerThreads[x]->updateEntityEquipment(SourcePlayerID,Event.getSlot(),Event.getItem());
							}
						}
					}
				}
				break;
			case FC_PPEVENT_SETBLOCK:
				for (int x=0;x<=_vPlayerThreads.size()-1;x++) {
					if( _vPlayerThreads[x]->isAssigned() && _vPlayerThreads[x]->isSpawned()) {
						if (_vPlayerThreads[x] == Event.getPtr() ) {continue;}
						_vPlayerThreads[x]->spawnBlock(Event.getBlockCoordinates(),Event.getBlockID());
					}
				}
				break;
			}

			_qEventQueue.pop();
		} catch(Poco::RuntimeException& ex) {
			cout<<"PlayerPool::run exception:"<<ex.message()<<"\n";
			_qEventQueue.pop();
		}
	}
}


bool PlayerPool::isAnySlotFree() {
	if (getFreeSlot() == -1) {
		return false;
	}else{
		return true;
	}
}

void PlayerPool::Assign(Poco::Net::StreamSocket& StrmSocket) {
	int iSlot;

	iSlot = getFreeSlot();
	if (iSlot == -1) {return;}

	_vPlayerThreads[iSlot]->Connect(StrmSocket);
}


int PlayerPool::getFreeSlot() {
	for (int x=0;x<=_vPlayerThreads.size()-1;x++) { //Release objects
		if (!_vPlayerThreads[x]->isAssigned()) {
			return x;
		}
	}
	return -1;
}

void PlayerPool::Event(PlayerPoolEvent& rEvent) {
	_qEventQueue.push(rEvent);
}

void PlayerPool::sendMessageToAll(string str) {
	for (int x=0;x<=_vPlayerThreads.size()-1;x++) {
		if( _vPlayerThreads[x]->isAssigned() && _vPlayerThreads[x]->isSpawned()) {
			_vPlayerThreads[x]->insertChat(str);
		}
	}
}

vector<string> PlayerPool::ListPlayers(int iMax) {
	int iElements = 0;
	vector<string> vNames(0);

	if (iMax <= 0) {
		return vNames;
	}

	for (int x=0;x<=_vPlayerThreads.size()-1;x++) {
		if( _vPlayerThreads[x]->isAssigned() && _vPlayerThreads[x]->isSpawned()) {
			vNames.push_back(_vPlayerThreads[x]->getUsername());
			iElements++;
			if (iElements == iMax) {
				break;
			}
		}
	}

	return vNames;
}

EntityPlayer PlayerPool::buildEntityPlayerFromPlayerPtr(PlayerThread* pPlayer) {
	EntityPlayer Player;

	Player._Coordinates = pPlayer->getCoordinates();
	Player._sName = pPlayer->getUsername();
	Player._Flags = pPlayer->getFlags();

	PlayerInventory & Inventory =  pPlayer->getInventory();

	Player._aHeldItems[0] = Inventory.getSlot(36 + Inventory.getSlotSelection());
	Player._aHeldItems[1] = Inventory.getSlot(8);
	Player._aHeldItems[2] = Inventory.getSlot(7);
	Player._aHeldItems[3] = Inventory.getSlot(6);
	Player._aHeldItems[4] = Inventory.getSlot(5);
	return Player;
}


PlayerThread* PlayerPool::getPlayerByName(string Name,PlayerThread* pCaller) {
	for (int x=0;x<=_vPlayerThreads.size()-1;x++) {
		if (_vPlayerThreads[x] == pCaller) { continue; }
		if (Poco::icompare(_vPlayerThreads[x]->getUsername(),Name) == 0) {
			return _vPlayerThreads[x];
		}
	}
	return NULL;
}


bool PlayerPool::willHurtOther(BlockCoordinates blockCoord,PlayerThread* pPlayer) {
	if (_vPlayerThreads.size() == 0) {return false;}

	BlockCoordinates playerCoord;

	for ( int x = 0;x<=_vPlayerThreads.size()-1;x++){
		if ((!_vPlayerThreads[x]->isAssigned()) || (!_vPlayerThreads[x]->isSpawned()) || _vPlayerThreads[x] == pPlayer) {continue;}

		playerCoord = ChunkMath::toBlockCoords(_vPlayerThreads[x]->getCoordinates());

		if ((playerCoord.X == blockCoord.X && playerCoord.Z == blockCoord.Z) && (playerCoord.Y == blockCoord.Y ||  playerCoord.Y+1 == blockCoord.Y)) {
			return true;
		}else{
			return false;
		}
	}
	return false;
}