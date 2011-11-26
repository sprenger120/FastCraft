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
	ChunkRoot* pChunkRoot
	) : 
_sName(""),
	_sNickName(""),
	_sIP(""),
	_Connection(),
	_sTemp(""),
	_SendQueue(),
	_sConnectionHash(""),
	_Web_Session("session.minecraft.net"),
	_Web_Response(),
	_Network(&_SendQueue),
	_ChunkProvider(pChunkRoot,&_Network)
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


	_iEntityID=0;
	setAuthStep(FC_AUTHSTEP_NOTCONNECTED);

	_pSettings = pSettingsHandler;
	_pEntityProvider = pEntityProvider;
	_pServerTime = pServerTime;
	_pPoolMaster = pPoolMaster;


	_fAssigned = false;
	_iThreadTicks = 0;

	InstanceCounter++;
	_iThreadID = InstanceCounter;
}

int PlayerThread::InstanceCounter = 0;
int PlayerThread::PlayerCount = 0;

void PlayerThread::ClearQueue() {
	for (int x = 1;x<=_SendQueue.size();x++) {
		_SendQueue.pop();
	}
}

PlayerThread::~PlayerThread() {
	ClearQueue();
	InstanceCounter--;
}


void PlayerThread::run() {
	int iTemp;
	unsigned char iPacket;

	int iKeepActive_LastID = 0;
	long long iKeepActive_LastTimestamp = getTicks();

	while (1) {
		if (!isAssigned()) {
			Thread::sleep(50);
			continue;
		} 

		if (!isAssigned()) {continue;} //Connection was closed by queue processor 

		try {

			if (iKeepActive_LastTimestamp + FC_INTERVAL_KEEPACTIVE == getTicks()) { //Send new keep alive
				iKeepActive_LastTimestamp = getTicks();

				iKeepActive_LastID = Random::Int();
				_Network.addByte(0x0);
				_Network.addInt(iKeepActive_LastID);
				_Network.Flush();
			}
			sendTime();
			IncrementTicks();
			ProcessQueue();

			if (getAuthStep() == FC_AUTHSTEP_PRECHUNKS) {
				_Coordinates.X = 100.0;
				_Coordinates.Y = 35.0;
				_Coordinates.Z = 100.0;
				_Coordinates.Stance = 35.0;
				_Coordinates.OnGround = true;
				_Coordinates.Pitch = 0.0F;
				_Coordinates.Yaw = 0.0F;

				sendClientPosition();
				setAuthStep(FC_AUTHSTEP_SPAWNPOS);
			}


			if (getAuthStep() >= FC_AUTHSTEP_SPAWNPOS) {
				_ChunkProvider.HandleMovement(_Coordinates);
				setAuthStep(FC_AUTHSTEP_INVENTORY);
			}

			iPacket = _Network.readByte();

			cout<<"Package recovered:"<<std::hex<<int(iPacket)<<"\n";

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

				setAuthStep(FC_AUTHSTEP_TIME); 
				_ChunkProvider.newConnection();
				break;
			case 0x2: //Handshake
				//Check login order
				if (getAuthStep() != FC_AUTHSTEP_CONNECTEDONLY) {
					Kick("False login order!");
					continue;
				}
				_sName = _sNickName = _Network.readString();

				PlayerCount++;
				setAuthStep(FC_AUTHSTEP_HANDSHAKE);
				_iEntityID = _pEntityProvider->Add(FC_ENTITY_PLAYER); //Fetch a new entity id

				//Send response (Connection Hash)
				_Network.addByte(0x02);

				if (_pSettings->isOnlineModeActivated()) {
					generateConnectionHash();
					_Network.addString(_sConnectionHash);
				}else{
					_Network.addString("-");
				}
				_Network.Flush();

				cout<<_sName<<" joined ("<<_sIP<<") EID:"<<_iEntityID<<endl;
				break;
			case 0xa: //Player
				if (getAuthStep() < FC_AUTHSTEP_PRECHUNKS) {
					_Network.read(1);
					continue;
				}

				_Coordinates.OnGround = _Network.readBool();
				break;
			case 0xb:
				if (getAuthStep() < FC_AUTHSTEP_PRECHUNKS) {
					_Network.read(33);
					continue;
				}
				_Coordinates.X = _Network.readDouble();
				_Coordinates.Y = _Network.readDouble();
				_Coordinates.Stance = _Network.readDouble();
				_Coordinates.Z = _Network.readDouble();
				_Coordinates.OnGround = _Network.readBool();
				break;
			case 0xc:
				if (getAuthStep() < FC_AUTHSTEP_PRECHUNKS) {
					_Network.read(9);
					continue;
				}
				_Coordinates.Yaw = _Network.readFloat();
				_Coordinates.Pitch = _Network.readFloat();
				_Coordinates.OnGround = _Network.readBool();
				break;
			case 0xd: //Client Pos Update
				if (getAuthStep() < FC_AUTHSTEP_PRECHUNKS) {
					_Network.read(41);
					continue;
				}
				_Coordinates.X = _Network.readDouble();
				_Coordinates.Y = _Network.readDouble();
				_Coordinates.Stance = _Network.readDouble();
				_Coordinates.Z = _Network.readDouble();
				_Coordinates.Yaw = _Network.readFloat();
				_Coordinates.Pitch = _Network.readFloat();
				_Coordinates.OnGround = _Network.readBool();
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
	}

	if (isNameSet() && !fNoLeaveMessage) { // If names is known
		cout<<_sName<<" left server."<<"\n"; 
	}

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

	setAuthStep(FC_AUTHSTEP_NOTCONNECTED);

	_fAssigned = false;
	_iEntityID = 0;

	_sName.assign("");
	_sNickName.assign("");
	_sIP.assign("");
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
}

bool PlayerThread::isAssigned() {
	return _fAssigned;
}

string PlayerThread::getUsername() {
	return _sName;
}

string PlayerThread::getNickname() {
	return _sNickName;
}

string PlayerThread::getIP() {
	return _sIP;
}

int PlayerThread::getThreadID() {
	return _iThreadID;
}

void PlayerThread::Kick(string sReason) {
	_Network.addByte(0xFF);
	_Network.addString(sReason);
	_Network.Flush(FC_JOB_CLOSECONN);


	if (isNameSet()) { // If names is known
		cout<<_sName<<" was kicked for: "<<sReason<<"\n"; 
	}
}

void PlayerThread::Kick() {
	_Network.addByte(0xFF);
	_Network.addString("");
	_Network.Flush(FC_JOB_CLOSECONN);

	if (isNameSet()) { // If names is known
		cout<<_sName<<" was kicked"<<"\n"; 
	}
}

void PlayerThread::ProcessQueue() {
	QueueJob qJob;
	while (1) {
		if (_SendQueue.size() == 0) {return;}

		qJob = _SendQueue.front();
		_SendQueue.pop();

		_Connection.sendBytes(qJob.Data.c_str(),qJob.Data.length());

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

bool PlayerThread::isNameSet() {
	return _iLoginProgress >= FC_AUTHSTEP_HANDSHAKE;
}

void PlayerThread::generateConnectionHash() {
	std::stringstream StringStream;
	string sTemp("");

	StringStream<<std::hex<< Random::uInt64();;
	_sConnectionHash.clear();


	while (!StringStream.eof()) {
		std::getline(StringStream,sTemp);
		_sConnectionHash.append(sTemp);
	}
}

void PlayerThread::setAuthStep(char iMode) {
	_iLoginProgress = iMode;
}

char PlayerThread::getAuthStep() {
	return _iLoginProgress;
}

void PlayerThread::sendTime() {
	if (getAuthStep() < FC_AUTHSTEP_TIME) {return;}
	if (getAuthStep() == FC_AUTHSTEP_TIME) { //Increment authstep
		setAuthStep(FC_AUTHSTEP_PRECHUNKS);
	}


	if (_TimeJobs.LastTimeSend + FC_INTERVAL_TIMESEND > getTicks()) {
		if (_TimeJobs.LastTimeSend != 0) {
			return;
		}
	}


	_Network.addByte(0x4);
	_Network.addInt64(_pServerTime->getTime());
	_Network.Flush();

	_TimeJobs.LastTimeSend = getTicks();
}

long long PlayerThread::getTicks() {
	return _iThreadTicks;
}

void PlayerThread::IncrementTicks() {
	_iThreadTicks++;
}

int PlayerThread::getConnectedPlayers() {
	return PlayerCount;
}

NetworkIO& PlayerThread::getConnection() {
	return _Network;
}

void PlayerThread::sendClientPosition() {
	_Network.addByte(0x0D);
	_Network.addDouble(_Coordinates.X);
	_Network.addDouble(_Coordinates.Stance);
	_Network.addDouble(_Coordinates.Y);
	_Network.addDouble(_Coordinates.Z);
	_Network.addFloat(_Coordinates.Yaw);
	_Network.addFloat(_Coordinates.Pitch);
	_Network.addBool(_Coordinates.OnGround);
	_Network.Flush();
}