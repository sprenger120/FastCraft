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

#include "PlayerPool.h"
#include "PlayerThread.h"
#include "SettingsHandler.h"
#include "PackingThread.h"
#include "EntityPlayer.h"
#include "EntityFlags.h"
#include "MathHelper.h"
#include <Poco/Thread.h>
#include <Poco/Exception.h>
#include <iostream>
#include <cmath>

using Poco::Thread;
using std::cout;

PlayerPool::PlayerPool(PackingThread& rPackingThread):
_vPlayerThreads(0),
	_ThreadPool("PlayerThreads",1,SettingsHandler::getPlayerSlotCount()),
	_qEventQueue(),
	_ChunkRoot(),
	_PackingThread(rPackingThread) 
{	
	try {
		_ChunkRoot.generateMap(0,0,20,20);
	} catch (Poco::RuntimeException) {
		throw Poco::RuntimeException("Chunk generation failed");
	}

	int iSlotCount = SettingsHandler::getPlayerSlotCount();

	_vPlayerThreads.resize(iSlotCount); //Resize to slot count

	//Create Threads
	for (int x=0;x<=_vPlayerThreads.size()-1;x++) {
		_vPlayerThreads[x] = new PlayerThread(this,_ChunkRoot,_PackingThread);

		_ThreadPool.defaultPool().start(*_vPlayerThreads[x]);
	}
}

PlayerPool::~PlayerPool() {
	_ThreadPool.stopAll(); //Stop all threads

	for (int x=0;x<=_vPlayerThreads.size()-1;x++) { //Release objects
		delete _vPlayerThreads[x];
	}
}


void PlayerPool::run() {
	std::string sData;
	

	while (1) {
		if (_qEventQueue.empty()) {
			Thread::sleep(50);
			continue;
		}
		try {

		PlayerPoolEvent& Event = _qEventQueue.front();
		

		switch (Event.getJobID()) {
		case FC_PPEVENT_CHAT:
			cout<<"Chat: "<<Event.getMessage()<<endl; //Server console
			sendMessageToAll(Event.getMessage());
			break;
		case FC_PPEVENT_MOVE:
			{
				//Build playerEntity
				EntityPlayer SourcePlayer;
				SourcePlayer._Coordinates = Event.getPtr()->getCoordinates();
				SourcePlayer._sName = Event.getPtr()->getUsername();
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

				EntityPlayer SourcePlayer;
				SourcePlayer._Coordinates = Event.getPtr()->getCoordinates();
				SourcePlayer._sName = Event.getName();
				int SourcePlayerID = Event.getPtr()->getEntityID();


				for (int x=0;x<=_vPlayerThreads.size()-1;x++) {
					if( _vPlayerThreads[x]->isAssigned() && _vPlayerThreads[x]->isSpawned()) {
						_vPlayerThreads[x]->PlayerInfoList(true,Event.getName()); //Update player list

						if (_vPlayerThreads[x] == Event.getPtr() ) {continue;} //dont spawn event source
						if (MathHelper::distance2D(_vPlayerThreads[x]->getCoordinates(),SourcePlayer._Coordinates) > 100.0) {continue;} //Too distant -> don't spawn

						_vPlayerThreads[x]->spawnPlayer(SourcePlayerID,SourcePlayer); //Spawn new player	

						//Spawn this player to event source
						EntityPlayer TargetPlayer; 

						TargetPlayer._Coordinates = _vPlayerThreads[x]->getCoordinates();
						TargetPlayer._sName = _vPlayerThreads[x]->getUsername();
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
				EntityFlags & rF = Event.getFlags();

				for (int x=0;x<=_vPlayerThreads.size()-1;x++) {
					if( _vPlayerThreads[x]->isAssigned() && _vPlayerThreads[x]->isSpawned()) {
						if (_vPlayerThreads[x] == Event.getPtr() ) {continue;}

						if (_vPlayerThreads[x]->isEntitySpawned(SourcePlayerID)) {
							_vPlayerThreads[x]->updateEntityMetadata(SourcePlayerID,rF);
						}
					}
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

void PlayerPool::sendMessageToAll(string rString) {
	for (int x=0;x<=_vPlayerThreads.size()-1;x++) {
		if( _vPlayerThreads[x]->isAssigned() && _vPlayerThreads[x]->isSpawned()) {
			_vPlayerThreads[x]->insertChat(rString);
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