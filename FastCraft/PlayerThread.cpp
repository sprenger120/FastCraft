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

#include "PlayerThread.h"
#include "EntityProvider.h"
#include "SettingsHandler.h"
#include "ServerTime.h"
#include "PlayerPool.h"
#include "Constants.h"
#include "Random.h"
#include "ChunkRoot.h"
#include <sstream>
#include <istream>
#include <Poco/Timespan.h>
#include <Poco/Net/NetException.h>
#include <Poco/Thread.h>
#include <Poco/Stopwatch.h>

using Poco::Thread;
using std::cout;
using std::endl;

PlayerThread::PlayerThread(SettingsHandler* pSettingsHandler,
	EntityProvider* pEntityProvider,
	ServerTime* pServerTime,
	PlayerPool* pPoolMaster,
	ChunkRoot* pChunkRoot,
	PackingThread* pPackingThread
	) : 
_sName(""),
	_sIP(""),
	_Connection(),
	_sTemp(""),
	_SendQueue(),
	_sConnectionHash(""),
	_Web_Session("session.minecraft.net"),
	_Web_Response(),
	_Network(&_SendQueue),
	_ChunkProvider(pChunkRoot,&_Network,pPackingThread,this)
{
	_Flags.Crouched = false;
	_Flags.Eating = false;
	_Flags.OnFire = false;
	_Flags.Riding = false;
	_Flags.Sprinting = false;
	_Coordinates.OnGround = false;
	_Coordinates.Pitch = 0.0F;
	_Coordinates.Stance = 0.0;
	_Coordinates.X = 0.0;
	_Coordinates.Y = 0.0;
	_Coordinates.Yaw = 0.0F;
	_Coordinates.Z = 0.0;

	_TimeJobs.LastTimeSend = 0;
	_TimeJobs.LastKeepAliveSend = 0;

	_iEntityID=0;
	setAuthStep(FC_AUTHSTEP_NOTCONNECTED);

	_pSettings = pSettingsHandler;
	_pEntityProvider = pEntityProvider;
	_pServerTime = pServerTime;
	_pPoolMaster = pPoolMaster;

	_fAssigned = false;
	_iThreadTicks = 0;
}

int PlayerThread::PlayerCount = 0;

PlayerThread::~PlayerThread() {
	ClearQueue();
}


bool PlayerThread::isAssigned() {
	return _fAssigned;
}

string PlayerThread::getUsername() {
	return _sName;
}

string PlayerThread::getIP() {
	return _sIP;
}

EntityCoordinates PlayerThread::getCoordinates() {
	return _Coordinates;
}

int PlayerThread::getConnectedPlayers() {
	return PlayerCount;
}



void PlayerThread::run() {
	int iTemp;
	unsigned char iPacket;
	EntityCoordinates TmpCoord;
	Poco::Stopwatch Timer;
	Timer.start();

	int iKeepActive_LastID = 0;
	long long iKeepActive_LastTimestamp = getTicks();

	while (1) {
		if (!isAssigned()) {
			Thread::sleep(50);
			continue;
		} 

		if (!isAssigned()) {continue;} //Connection was closed by queue processor 

		try {
			sendKeepAlive();
			sendTime();

			Timer.stop();
			_iThreadTicks += Timer.elapsed() / 1000;
			Timer.reset();
			Timer.start();

			ProcessAuthStep();
			ProcessQueue();
					
			iPacket = _Network.readByte();

			//cout<<"Package recovered:"<<std::hex<<int(iPacket)<<"\n";

			switch (iPacket) {
			case 0x0: //Keep Alive
				_Network.readInt(); //Get id
				break;
			case 0x1: //Login
				//Check login order
				if (getAuthStep() != FC_AUTHSTEP_HANDSHAKE) {
					Kick("False login order!");
					continue;
				}

				//Check minecraft version
				iTemp = _Network.readInt(); //Protocol Version

				if (iTemp > _pSettings->getSupportedProtocolVersion()) {
					Kick("Outdated server!");
					continue;
				}

				if (iTemp < _pSettings->getSupportedProtocolVersion()) {
					Kick("Outdated client!");
					continue;
				}

				_Network.readString(); //Username (already known)	
				_Network.read(16);

				//Check premium 
				if (_pSettings->isOnlineModeActivated()) {
					string sPath("/game/checkserver.jsp?user=");
					sPath.append(_sName);
					sPath.append("&serverId=");
					sPath.append(_sConnectionHash);

					Poco::Net::HTTPRequest Request ( 
						Poco::Net::HTTPRequest::HTTP_GET, 
						sPath,
						Poco::Net::HTTPMessage::HTTP_1_1);

					_Web_Session.sendRequest(Request);
					std::istream &is = _Web_Session.receiveResponse(_Web_Response);
					string sErg;
					is>>sErg;

					if (sErg.compare("YES") != 0) {
						Kick("Failed to verify username!");
						continue;
					}
				}

				//Answer
				_Network.Lock();

				_Network.addByte(0x1);
				_Network.addInt(_iEntityID);
				_Network.addString("");
				_Network.addInt64(1234568);
				_Network.addInt(_pSettings->getServerMode());
				_Network.addByte(0);
				_Network.addByte(_pSettings->getDifficulty());
				_Network.addByte(_pSettings->getWorldHeight());
				_Network.addByte(_pSettings->getMaxClients());

				_Network.Flush();
				_Network.UnLock();

				setAuthStep(FC_AUTHSTEP_TIME); 
				_ChunkProvider.newConnection();
				_pPoolMaster->Chat(_sName + " joined game",this,false); //false= no <Name> adding
				break;
			case 0x2: //Handshake
				//Check login order
				if (getAuthStep() != FC_AUTHSTEP_CONNECTEDONLY) {
					Kick("False login order!");
					continue;
				}
				_sName = _Network.readString();

				PlayerCount++;
				setAuthStep(FC_AUTHSTEP_HANDSHAKE);
				_iEntityID = _pEntityProvider->Add(FC_ENTITY_PLAYER); //Fetch a new entity id

				//Send response (Connection Hash)
				_Network.Lock();
				_Network.addByte(0x02);

				if (_pSettings->isOnlineModeActivated()) {
					generateConnectionHash();
					_Network.addString(_sConnectionHash);
				}else{
					_Network.addString("-");
				}
				_Network.Flush();
				_Network.UnLock();
				cout<<_sName<<" joined ("<<_sIP<<") EID:"<<_iEntityID<<endl;
				break;
			case 0x3: //Chat
				_sTemp.assign(_Network.readString());
				_pPoolMaster->Chat(_sTemp,this);
				break;
			case 0xa: //Player

				switch(isLoginDone()) {
				case true:
					_Coordinates.OnGround = _Network.readBool();

					if (!_ChunkProvider.isFullyCircleSpawned()) {
						_ChunkProvider.HandleMovement(_Coordinates);
					}

					break;
				case false:
					_Network.read(1);
					break;
				}

				break;
			case 0xb: //Coords & OnGround
				switch(isLoginDone()) {
				case true:
					//Read coordinates in a temporary variable
					TmpCoord.X = _Network.readDouble();
					TmpCoord.Y = _Network.readDouble();
					TmpCoord.Stance = _Network.readDouble();
					TmpCoord.Z = _Network.readDouble();
					TmpCoord.OnGround = _Network.readBool();

					//if X and Z ==  8.5000000000000000 , there is crap in the tcp buffer -> ignore it
					if (TmpCoord.X == 8.5000000000000000 && TmpCoord.Z == 8.5000000000000000) { 
						continue;
					}else{
						_Coordinates.X = TmpCoord.X;
						_Coordinates.Y = TmpCoord.Y;
						_Coordinates.Stance = TmpCoord.Stance;
						_Coordinates.Z = TmpCoord.Z;
						_Coordinates.OnGround = TmpCoord.OnGround;
						_ChunkProvider.HandleMovement(_Coordinates);
					}
					break;
				case false:
					_Network.read(33);
					break;
				}

				break;
			case 0xc: //Look & OnGround
				switch(isLoginDone()) {
				case true:
					_Coordinates.Yaw = _Network.readFloat();
					_Coordinates.Pitch = _Network.readFloat();
					_Coordinates.OnGround = _Network.readBool();

					if (!_ChunkProvider.isFullyCircleSpawned()) {
						_ChunkProvider.HandleMovement(_Coordinates);
					}
					break;
				case false:
					_Network.read(9);
					break;
				}
				break;
			case 0xd: //Full Client position
				switch(isLoginDone()) {
				case true:
					//Read coordinates in a temporary variable
					TmpCoord.X = _Network.readDouble();
					TmpCoord.Y = _Network.readDouble();
					TmpCoord.Stance = _Network.readDouble();
					TmpCoord.Z = _Network.readDouble();
					TmpCoord.Yaw = _Network.readFloat();
					TmpCoord.Pitch = _Network.readFloat();
					TmpCoord.OnGround = _Network.readBool();

					//if X and Z ==  8.5000000000000000 , there is crap in the tcp buffer -> ignore it
					if (TmpCoord.X == 8.5000000000000000 && TmpCoord.Z == 8.5000000000000000) { 
						continue;
					}else{

						_Coordinates = TmpCoord;
						_ChunkProvider.HandleMovement(_Coordinates);
					}
					break;
				case false:
					_Network.read(41);
					break;
				}
				break;
			case 0xFE: //Server List Ping
				_sTemp.clear();
				_sTemp.assign(_pSettings->getServerDescription()); //Server Description
				_sTemp.append("§");
				Poco::NumberFormatter::append(_sTemp,PlayerCount); //Player count
				_sTemp.append("§");
				Poco::NumberFormatter::append(_sTemp,_pSettings->getMaxClients()); //player slots
				_sTemp.append("§");

				Kick(_sTemp);
				break;
			case 0xFF: //Disconnect
				_Network.readString();
				Disconnect();
				break;
			default: 
				Kick("Unknown packet!");
				cout<<"Unknown packet received! 0x"<<std::hex<<int(iPacket)<<endl;
				break;
			}
		} catch (Poco::RuntimeException& err) {
			cout<<">>>Exception catched:"<<err.message()<<endl;
			Disconnect();
		}

	}

}
void PlayerThread::Disconnect(bool fNoLeaveMessage) {
	if (isNameSet()) {
		_ChunkProvider.Disconnect();
		PlayerCount--;
		_pEntityProvider->Remove(_iEntityID);

		if (!fNoLeaveMessage) {
			_pPoolMaster->Chat(_sName + " left game",this,false); //false= no <Name> adding
			cout<<_sName<<" left server."<<"\n"; 
		}
	}


	_Flags.Crouched = false;
	_Flags.Eating = false;
	_Flags.OnFire = false;
	_Flags.Riding = false;
	_Flags.Sprinting = false;

	setAuthStep(FC_AUTHSTEP_NOTCONNECTED);

	_fAssigned = false;

	_TimeJobs.LastKeepAliveSend = 0;
	_TimeJobs.LastTimeSend = 0;

	ClearQueue();
	_Network.closeConnection();
}


void PlayerThread::Connect(Poco::Net::StreamSocket& Sock) {
	if (_fAssigned) {
		cout<<"***INTERNAL SERVER WARNING: PlayerPool tryed to assign an already assigned player thread"<<"\n";
		Disconnect();
	}
	_fAssigned=true;
	setAuthStep(FC_AUTHSTEP_CONNECTEDONLY);

	_Connection = Sock; 
	_Network.newConnection(Sock);

	_sIP.assign(_Connection.peerAddress().toString());
	_sName.clear();
	_iThreadTicks = 0;
}

void PlayerThread::Kick(string sReason) {
	_Network.Lock();
	_Network.addByte(0xFF);
	_Network.addString(sReason);
	_Network.Flush(FC_JOB_CLOSECONN);
	_Network.UnLock();

	if (isNameSet()) { // If names is known
		cout<<_sName<<" was kicked for: "<<sReason<<"\n"; 
	}
}

void PlayerThread::Kick() {
	_Network.Lock();
	_Network.addByte(0xFF);
	_Network.addString("");
	_Network.Flush(FC_JOB_CLOSECONN);
	_Network.UnLock();

	if (isNameSet()) { // If names is known
		cout<<_sName<<" was kicked"<<"\n"; 
	}
}


void PlayerThread::generateConnectionHash() {
	std::stringstream StringStream;
	StringStream<<std::hex<< Random::uInt64();
	_sConnectionHash.assign(StringStream.str());
}

void PlayerThread::sendTime() {
	if (getAuthStep() < FC_AUTHSTEP_TIME) {return;}
	if (getAuthStep() == FC_AUTHSTEP_TIME) { //Increment authstep
		//Send first time packet
		_Network.Lock();
		_Network.addByte(0x4);
		_Network.addInt64(ServerTime::getTime());
		_Network.Flush();
		_Network.UnLock();

		_TimeJobs.LastTimeSend = getTicks();
		setAuthStep(FC_AUTHSTEP_PRECHUNKS);
		return;
	}


	if (_TimeJobs.LastTimeSend + FC_INTERVAL_TIMESEND <= getTicks()) {
		_TimeJobs.LastTimeSend = getTicks();
		_Network.Lock();
		_Network.addByte(0x4);
		_Network.addInt64(ServerTime::getTime());
		_Network.Flush();
		_Network.UnLock();
	}
}

long long PlayerThread::getTicks() {
	return _iThreadTicks;
}

NetworkIO& PlayerThread::getConnection() {
	return _Network;
}

void PlayerThread::sendClientPosition() {
	_Network.Lock();

	_Network.addByte(0x0D);
	_Network.addDouble(_Coordinates.X);
	_Network.addDouble(_Coordinates.Stance);
	_Network.addDouble(_Coordinates.Y);
	_Network.addDouble(_Coordinates.Z);
	_Network.addFloat(_Coordinates.Yaw);
	_Network.addFloat(_Coordinates.Pitch);
	_Network.addBool(_Coordinates.OnGround);
	_Network.Flush();

	_Network.UnLock();
}

void PlayerThread::sendKeepAlive() {
	if (_TimeJobs.LastKeepAliveSend + FC_INTERVAL_KEEPACTIVE <= getTicks()) { //Send new keep alive
		_TimeJobs.LastKeepAliveSend = getTicks();

		_Network.Lock();
		_Network.addByte(0x0);
		_Network.addInt(Random::Int());
		_Network.Flush();
		_Network.UnLock();
	}
}

void PlayerThread::UpdateHealth(short iHealth,short iFood,float nSaturation) {
	//Send Health
	_iHealth = fixRange<short>(iHealth,0,20);
	_iFood = fixRange<short>(iFood,0,20);
	_nSaturation = fixRange<float>(iFood,0.0F,5.0F);

	_Network.Lock();
	_Network.addByte(0x8);
	_Network.addShort(_iHealth);
	_Network.addShort(_iFood);
	_Network.addFloat(_nSaturation);
	_Network.Flush();
	_Network.UnLock();
}



template <class T> T PlayerThread::fixRange(T val,T min,T max) {
	if (val > max) {
		return max;
	}

	if (val < min) {
		return min;
	}

	return val;
}


/*
*	AUTHSTEP
*/
bool PlayerThread::isSpawned() {
	if (getAuthStep() >= FC_AUTHSTEP_PRECHUNKS) {
		return true;
	}else{
		return false;
	}
}

bool PlayerThread::isNameSet() {
	return _iLoginProgress >= FC_AUTHSTEP_HANDSHAKE;
}

void PlayerThread::setAuthStep(char iMode) {
	_iLoginProgress = iMode;
}

char PlayerThread::getAuthStep() {
	return _iLoginProgress;
}

bool PlayerThread::isLoginDone() {
	if (getAuthStep() == FC_AUTHSTEP_DONE) {
		return true;
	}else{
		return false;
	}
}

void PlayerThread::ProcessAuthStep() {
	switch (getAuthStep()) {
	case FC_AUTHSTEP_PRECHUNKS:
		//Set start coordinates
		_Coordinates.X = 100.0;
		_Coordinates.Y = 35.0;
		_Coordinates.Z = 100.0;
		_Coordinates.Stance = 35.0;
		_Coordinates.OnGround = false;
		_Coordinates.Pitch = 0.0F;
		_Coordinates.Yaw = 0.0F;

		_ChunkProvider.HandleMovement(_Coordinates); //Send first chunks
		setAuthStep(FC_AUTHSTEP_SPAWNPOS);
		break;
	case FC_AUTHSTEP_SPAWNPOS:
		//Send spawnpos & Health
		_Network.addByte(0x6);
		_Network.addInt(0); //X
		_Network.addInt(0); // Y
		_Network.addInt(0); // Z 
		_Network.Flush();

		UpdateHealth(20,20,5.0F);
		setAuthStep(FC_AUTHSTEP_INVENTORY);
		break;
	case FC_AUTHSTEP_INVENTORY:
		//ToDo: send inventory
		setAuthStep(FC_AUTHSTEP_DONE);
		break;
	}
}


/*
* QUEUE
*/
void PlayerThread::ClearQueue() {
	for (int x = 1;x<=_SendQueue.size();x++) {
		_SendQueue.pop();
	}
}

void PlayerThread::ProcessQueue() {
	QueueJob qJob;

	while (_SendQueue.size()) {
		qJob = _SendQueue.front();
		_SendQueue.pop();

		_Connection.sendBytes(qJob.Data.c_str(),qJob.Data.length());
		
		Thread::sleep(5);

		switch(qJob.Special) {
		case FC_JOB_NO:
			break;
		case FC_JOB_CLOSECONN:
			Thread::sleep(200); 
			Disconnect(true);
			break;
		default:
			cout<<"***INTERNAL SERVER ERROR: Unknown job ID: ("<<qJob.Special<<") !"<<"\n";
			Disconnect(true);
			break;
		}
	}
}

void PlayerThread::appendQueue(QueueJob& Job) {
	_SendQueue.push(Job);
}