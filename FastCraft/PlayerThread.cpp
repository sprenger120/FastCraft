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
#include <Poco/Stopwatch.h>

using Poco::Thread;
using std::cout;
using std::endl;
using std::dec;

PlayerThread::PlayerThread(EntityProvider& rEntityProvider,
	PlayerPool* pPoolMaster,
	ChunkRoot& rChunkRoot,
	PackingThread& rPackingThread
	) : 
_sName(""),
	_sIP(""),
	_sLeaveMessage(""),
	_Connection(),
	_sTemp(""),
	_sConnectionHash(""),

	_lowLevelSendQueue(),
	_highLevelSendQueue(),
	_lowNetwork(_lowLevelSendQueue,_Connection),
	_highNetwork(_highLevelSendQueue,_Connection),
	_NetworkWriter(_lowLevelSendQueue,_highLevelSendQueue,_Connection,this),

	_Web_Session("session.minecraft.net"),
	_Web_Response(),
	_rEntityProvider(rEntityProvider),
	_Flags(),
	_ChunkProvider(rChunkRoot,_lowNetwork,rPackingThread,this),
	_threadNetworkWriter("NetworkWriter")
{
	_Coordinates.OnGround = false;
	_Coordinates.Pitch = 0.0F;
	_Coordinates.Stance = 0.0;
	_Coordinates.X = 0.0;
	_Coordinates.Y = 0.0;
	_Coordinates.Yaw = 0.0F;
	_Coordinates.Z = 0.0;

	_TimeJobs.LastTimeSend = 0L;
	_TimeJobs.LastKeepAliveSend = 0L;
	_TimeJobs.LastHandleMovement = 0L;

	_iEntityID=0;
	_Spawned_PlayerInfoList = 0;
	_pPoolMaster = pPoolMaster;

	_fSpawned = false;
	_fAssigned = false;
	_iThreadTicks = 0;

	//Start NetworkWriter Thread
	_threadNetworkWriter.start(_NetworkWriter);
}

int PlayerThread::_PlayerCount = 0;

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
	return _PlayerCount;
}

void PlayerThread::run() {
	unsigned char iPacket;
	Poco::Stopwatch Timer;

	Timer.start();

	while (1) {
		if (!isAssigned()) {
			Thread::sleep(50);
			continue;
		} 


		try {
			//Thread Ticks
			Timer.stop();
			_iThreadTicks += Timer.elapsed() / 1000;
			Timer.reset();
			Timer.start();


			Interval_KeepAlive(); 
			Interval_Time();
			Interval_HandleMovement();

			iPacket = _lowNetwork.readByte();

			//cout<<"Package recovered:"<<std::hex<<int(iPacket)<<"\n";
			switch (iPacket) {
			case 0x0:
				Packet0_KeepAlive();
				break;
			case 0x1:
				if (isSpawned()) {
					Kick("Login already done!");
					continue;
				}
				Packet1_Login();
				break;
			case 0x2:
				if (isSpawned()) {
					Kick("Login already done!");
					continue;
				}
				Packet2_Handshake();
				break;
			case 0x3:
				Packet3_Chat();
				break;
			case 0xa: 
				Packet10_Player();
				break;
			case 0xb:
				Packet11_Position();
				break;
			case 0xc: 
				Packet12_Look();
				break;
			case 0xd: 
				Packet13_PosAndLook();
				break;
			case 0xFE: //Server List Ping
				Packet254_ServerListPing();
				break;
			case 0xFF: //Disconnect
				Packet255_Disconnect();
				break;
			default: 
				Kick("Unknown packet!");
				cout<<"Unknown packet received! 0x"<<std::hex<<int(iPacket)<<endl;
				break;
			}
		} catch (Poco::RuntimeException) {
			Disconnect(FC_LEAVE_OTHER);
		}

	}
}


void PlayerThread::Disconnect(char iLeaveMode) {

	if (isSpawned()) {
		_ChunkProvider.HandleDisconnect();
		_PlayerCount--;
		_rEntityProvider.Remove(_iEntityID);

		//Push disconnect event
		_ppEvent.Coordinates = _Coordinates;
		_ppEvent.Job = FC_PPEVENT_DISCONNECT;
		_ppEvent.pThread = this;
		_pPoolMaster->Event(_ppEvent);

		switch (iLeaveMode) {
		case FC_LEAVE_KICK:
			break;
		case FC_LEAVE_QUIT:
			_sTemp.assign( _sName + " quit game" );
			pushChatEvent(_sTemp);
			cout<<_sName<<" left server. ("<<_sLeaveMessage<<")"<<"\n"; 
			break;
		case FC_LEAVE_OTHER:
			_sTemp.assign( _sName + " closed connection (unknown reason)" );
			pushChatEvent(_sTemp);
			cout<<_sName<<"  closed connection (unknown reason)"<<"\n"; 
			break;
		default:
			cout<<"***INTERNAL SERVER WARNING: Unknown disconnect mode"<<"\n";
		}
	}

	_Flags.clear();

	_fSpawned = false;
	_fAssigned = false;
	_sName.clear();
	_sIP.clear();


	_TimeJobs.LastKeepAliveSend = 0;
	_TimeJobs.LastTimeSend = 0;
	_TimeJobs.LastHandleMovement = 0;

	ClearQueue();
	_Connection.close();
}


void PlayerThread::Connect(Poco::Net::StreamSocket& Sock) {
	if (_fAssigned) {
		cout<<"***INTERNAL SERVER WARNING: PlayerPool tryed to assign an already assigned player thread"<<"\n";
		Disconnect(FC_LEAVE_OTHER);
	}
	_fAssigned=true;
	_Connection = Sock; 

	_sIP.assign(_Connection.peerAddress().toString());
	_iThreadTicks = 0;
}

void PlayerThread::Kick(string sReason) {
	_sTemp.clear();
	_sTemp.append<char>(1,0xFF);
	NetworkIO::packString(_sTemp,sReason);

	try {
		_Connection.sendBytes(_sTemp.c_str(),_sTemp.length());
	}catch(Poco::Net::ConnectionAbortedException) {
		Disconnect(FC_LEAVE_OTHER);
		return;
	}catch(Poco::Net::InvalidSocketException) {
		Disconnect(FC_LEAVE_OTHER);
		return;
	}catch(Poco::TimeoutException) {
		Disconnect(FC_LEAVE_OTHER);
		return;
	}catch(Poco::Net::ConnectionResetException) {
		Disconnect(FC_LEAVE_OTHER);
		return;
	}catch(Poco::IOException) {
		Disconnect(FC_LEAVE_OTHER);
		return;
	}


	if (isSpawned()) { // If names is known
		cout<<_sName<<" was kicked for: "<<sReason<<"\n"; 
	}

	Thread::sleep(10);

	Disconnect(FC_LEAVE_KICK);
}

void PlayerThread::Kick() {
	_sTemp.clear();
	_sTemp.append<char>(1,0xFF);
	_sTemp.append<char>(2,0x0);

	try {
		_Connection.sendBytes(_sTemp.c_str(),_sTemp.length());
	}catch(Poco::Net::ConnectionAbortedException) {
		Disconnect(FC_LEAVE_OTHER);
		return;
	}catch(Poco::Net::InvalidSocketException) {
		Disconnect(FC_LEAVE_OTHER);
		return;
	}catch(Poco::TimeoutException) {
		Disconnect(FC_LEAVE_OTHER);
		return;
	}catch(Poco::Net::ConnectionResetException) {
		Disconnect(FC_LEAVE_OTHER);
		return;
	}catch(Poco::IOException) {
		Disconnect(FC_LEAVE_OTHER);
		return;
	}


	if (isSpawned()) { // If names is known
		cout<<_sName<<" was kicked"<<"\n"; 
	}

	Thread::sleep(10);

	Disconnect(FC_LEAVE_KICK);
}


string PlayerThread::generateConnectionHash() {
	std::stringstream StringStream;
	StringStream<<std::hex<< Random::uInt64();
	_sConnectionHash.assign(StringStream.str());
	return _sConnectionHash;
}

void PlayerThread::Interval_Time() {
	if (_TimeJobs.LastTimeSend + FC_INTERVAL_TIMESEND <= getTicks()) {
		_TimeJobs.LastTimeSend = getTicks();
		sendTime();
	}
}

void PlayerThread::Interval_HandleMovement() {
	if (!isSpawned()) {return;}
	if (_TimeJobs.LastHandleMovement + FC_INTERVAL_HANDLEMOVEMENT <= getTicks()) {
		_TimeJobs.LastHandleMovement = getTicks();
		_ChunkProvider.HandleMovement(_Coordinates);
	}
}

void PlayerThread::sendTime() {
	_highNetwork.Lock();
	_highNetwork.addByte(0x4);
	_highNetwork.addInt64(ServerTime::getTime());
	_highNetwork.Flush();
	_highNetwork.UnLock();
}

long long PlayerThread::getTicks() {
	return _iThreadTicks;
}

NetworkIO& PlayerThread::getConnection() {
	return _lowNetwork;
}

void PlayerThread::sendClientPosition() {
	_highNetwork.Lock();

	_highNetwork.addByte(0x0D);
	_highNetwork.addDouble(_Coordinates.X);
	_highNetwork.addDouble(_Coordinates.Stance);
	_highNetwork.addDouble(_Coordinates.Y);
	_highNetwork.addDouble(_Coordinates.Z);
	_highNetwork.addFloat(_Coordinates.Yaw);
	_highNetwork.addFloat(_Coordinates.Pitch);
	_highNetwork.addBool(_Coordinates.OnGround);
	_highNetwork.Flush();

	_highNetwork.UnLock();
}

void PlayerThread::Interval_KeepAlive() {
	if (_TimeJobs.LastKeepAliveSend + FC_INTERVAL_KEEPACTIVE <= getTicks()) { //Send new keep alive
		_TimeJobs.LastKeepAliveSend = getTicks();

		_highNetwork.Lock();
		_highNetwork.addByte(0x0);
		_highNetwork.addInt(Random::Int());
		_highNetwork.Flush();
		_highNetwork.UnLock();
	}
}

void PlayerThread::UpdateHealth(short iHealth,short iFood,float nSaturation) {
	//Send Health
	_iHealth = fixRange<short>(iHealth,0,20);
	_iFood = fixRange<short>(iFood,0,20);
	_nSaturation = fixRange<float>(iFood,0.0F,5.0F);

	_highNetwork.Lock();
	_highNetwork.addByte(0x8);
	_highNetwork.addShort(_iHealth);
	_highNetwork.addShort(_iFood);
	_highNetwork.addFloat(_nSaturation);
	_highNetwork.Flush();
	_highNetwork.UnLock();
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


bool PlayerThread::isSpawned() {
	return _fSpawned;
}


/*
* QUEUE
*/
void PlayerThread::appendQueue(string& rString) {
	_lowLevelSendQueue.push(rString);
}

void PlayerThread::insertChat(string& rString) {
	_highNetwork.Lock();
	_highNetwork.addByte(0x3);
	_highNetwork.addString(rString);
	_highNetwork.Flush();
	_highNetwork.UnLock();
}

void PlayerThread::ClearQueue() {
	_highLevelSendQueue.clear();
	_lowLevelSendQueue.clear();
}

void PlayerThread::ProcessQueue() {			
	while (!_highLevelSendQueue.empty()) {
		string & rJob = _highLevelSendQueue.front();

		try {
			_Connection.sendBytes(rJob.c_str(),rJob.length()); //Send
		}catch(Poco::Net::ConnectionAbortedException) {
			Disconnect(FC_LEAVE_OTHER);
			return;
		}catch(Poco::Net::InvalidSocketException) {
			Disconnect(FC_LEAVE_OTHER);
			return;
		}catch(Poco::TimeoutException) {
			Disconnect(FC_LEAVE_OTHER);
			return;
		}catch(Poco::Net::ConnectionResetException) {
			Disconnect(FC_LEAVE_OTHER);
			return;
		}catch(Poco::IOException) {
			Disconnect(FC_LEAVE_OTHER);
			return;
		}


		_highLevelSendQueue.pop();
	}
}


/*
* Packets
*/

void PlayerThread::Packet0_KeepAlive() {
	_lowNetwork.readInt(); //Get id
}

void PlayerThread::Packet1_Login() {
	int iProtocolVersion = 0;

	try {
		//Check minecraft version
		iProtocolVersion = _lowNetwork.readInt(); //Protocol Version


		if (iProtocolVersion > SettingsHandler::getSupportedProtocolVersion()) {
			Kick("Outdated server! Needed Version: " + SettingsHandler::getSupportedMCVersion());
			return;
		}

		if (iProtocolVersion <  SettingsHandler::getSupportedProtocolVersion()) {
			Kick("Outdated client! Needed Version: " + SettingsHandler::getSupportedMCVersion());
			return;
		}

		_lowNetwork.readString(); //Username (already known)	
		_lowNetwork.read(16);

		//Check premium 
		if (SettingsHandler::isOnlineModeActivated()) {
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
				return;
			}
		}


		//YES DUDE, you got it !
		_PlayerCount++; //There is an new spawned player
		_iEntityID = _rEntityProvider.Add(FC_ENTITY_PLAYER); //Fetch a new entity id
		_ChunkProvider.HandleNewPlayer();

		//Set start coordinates
		_Coordinates.X = 5.0;
		_Coordinates.Y = 35.0;
		_Coordinates.Z = 5.0;
		_Coordinates.Stance = 35.0;
		_Coordinates.OnGround = false;
		_Coordinates.Pitch = 0.0F;
		_Coordinates.Yaw = 0.0F;


		//Push PlayerPool Join event
		_ppEvent.Coordinates = _Coordinates;
		_ppEvent.Job = FC_PPEVENT_JOIN;
		_ppEvent.pThread = this;
		_pPoolMaster->Event(_ppEvent);


		cout<<_sName<<" joined ("<<_sIP<<") EID:"<<_iEntityID<<endl;  //Console log
		_sTemp.assign(  _sName + " joined game" );
		pushChatEvent(_sTemp);

		/*
		* Response
		*/
		//Login response
		_highNetwork.Lock();

		_highNetwork.addByte(0x1);
		_highNetwork.addInt(_iEntityID);
		_highNetwork.addString("");
		_highNetwork.addInt64(SettingsHandler::getMapSeed());
		_highNetwork.addInt(SettingsHandler::getServerMode());
		_highNetwork.addByte(0);
		_highNetwork.addByte(SettingsHandler::getDifficulty());
		_highNetwork.addByte(SettingsHandler::getWorldHeight());
		_highNetwork.addByte((unsigned char)SettingsHandler::getPlayerSlotCount());
		_highNetwork.Flush();

		//compass
		_highNetwork.addByte(0x6);
		_highNetwork.addInt(0); //X
		_highNetwork.addInt(0); // Y
		_highNetwork.addInt(0); // Z 
		_highNetwork.Flush();
		_highNetwork.UnLock();

		//Client position
		sendClientPosition(); //Make client leave downloading map screen | chunk provider will send clients position again, after spawning chunk who standing on

		//Time
		sendTime(); 

		//Health
		UpdateHealth(20,20,5.0F); //Health

		//Spawn PlayerInfo
		vector<string> vNames;
		vNames = _pPoolMaster->ListPlayers(60);

		if (vNames.size() > 0) {
			for ( int x = 0;x<= vNames.size()-1;x++) {
				PlayerInfoList(true,vNames[x]);
			}
		}

		//Send login packages
		ProcessQueue();
		_fSpawned = true;


		//Chunks
		_ChunkProvider.HandleMovement(_Coordinates);
	} catch(Poco::RuntimeException) {
		Disconnect(FC_LEAVE_OTHER);
	}
}

void PlayerThread::Packet2_Handshake() {
	try {
		_sName = _highNetwork.readString();
	} catch(Poco::RuntimeException) {
		Disconnect(FC_LEAVE_OTHER);
	}

	if (_sName.length() > 16) {
		Kick("Username too long");
		return;
	}

	//Send response (Connection Hash)
	_highNetwork.Lock();
	_highNetwork.addByte(0x02);

	if (SettingsHandler::isOnlineModeActivated()) {
		_highNetwork.addString(generateConnectionHash());
	}else{
		_highNetwork.addString("-");
	}
	_highNetwork.Flush();
	_highNetwork.UnLock();

	ProcessQueue();
}

void PlayerThread::Packet3_Chat() {
	string Message("");

	try {
		Message = _lowNetwork.readString();
		if (Message.length() > 100) {
			Kick("Received string too long");
			return;
		}
	} catch(Poco::RuntimeException) {
		Disconnect(FC_LEAVE_OTHER);
	}

	_sTemp.assign("<");
	_sTemp.append(_sName);
	_sTemp.append("> ");
	_sTemp.append(Message); 

	pushChatEvent(_sTemp);
}

void PlayerThread::Packet10_Player() {
	try {
		_Coordinates.OnGround = _lowNetwork.readBool();
	} catch(Poco::RuntimeException) {
		Disconnect(FC_LEAVE_OTHER);
	}
}

void PlayerThread::Packet11_Position() {
	EntityCoordinates TmpCoord;

	try {
		//Read coordinates in a temporary variable
		TmpCoord.X = _lowNetwork.readDouble();
		TmpCoord.Y = _lowNetwork.readDouble();
		TmpCoord.Stance = _lowNetwork.readDouble();
		TmpCoord.Z = _lowNetwork.readDouble();
		TmpCoord.OnGround = _lowNetwork.readBool();

		//if X and Z ==  8.5000000000000000 , there is crap in the tcp buffer -> ignore it
		if (TmpCoord.X == 8.5000000000000000 && TmpCoord.Z == 8.5000000000000000) { 
			return;
		}else{
			_Coordinates.X = TmpCoord.X;
			_Coordinates.Y = TmpCoord.Y;
			_Coordinates.Stance = TmpCoord.Stance;
			_Coordinates.Z = TmpCoord.Z;
			_Coordinates.OnGround = TmpCoord.OnGround;
		}
	} catch(Poco::RuntimeException) {
		Disconnect(FC_LEAVE_OTHER);
	}
}

void PlayerThread::Packet12_Look() {
	try {
		_Coordinates.Yaw = _lowNetwork.readFloat();
		_Coordinates.Pitch = _lowNetwork.readFloat();
		_Coordinates.OnGround = _lowNetwork.readBool();
	} catch(Poco::RuntimeException) {
		Disconnect(FC_LEAVE_OTHER);
	}
}

void PlayerThread::Packet13_PosAndLook() {
	EntityCoordinates TmpCoord;

	//Read coordinates in a temporary variable
	try {
		TmpCoord.X = _lowNetwork.readDouble();
		TmpCoord.Y = _lowNetwork.readDouble();
		TmpCoord.Stance = _lowNetwork.readDouble();
		TmpCoord.Z = _lowNetwork.readDouble();
		TmpCoord.Yaw = _lowNetwork.readFloat();
		TmpCoord.Pitch = _lowNetwork.readFloat();
		TmpCoord.OnGround = _lowNetwork.readBool();

		//if X and Z ==  8.5000000000000000 , there is crap in the tcp buffer -> ignore it
		if (TmpCoord.X == 8.5000000000000000 && TmpCoord.Z == 8.5000000000000000) { 
			return;
		}else{
			_Coordinates = TmpCoord;
		}
	} catch(Poco::RuntimeException) {
		Disconnect(FC_LEAVE_OTHER);
	}
}

void PlayerThread::Packet254_ServerListPing() {
	_sTemp.clear();
	_sTemp.assign(SettingsHandler::getServerDescription()); //Server Description
	_sTemp.append("§");
	Poco::NumberFormatter::append(_sTemp,_PlayerCount); //Player count
	_sTemp.append("§");
	Poco::NumberFormatter::append(_sTemp,SettingsHandler::getPlayerSlotCount()); //player slots

	Kick(_sTemp);
}

void PlayerThread::Packet255_Disconnect() {
	try {
		_sLeaveMessage = _lowNetwork.readString();
	} catch(Poco::RuntimeException) {
		Disconnect(FC_LEAVE_OTHER);
	}
	Disconnect(FC_LEAVE_QUIT);
}

void PlayerThread::pushChatEvent(string& rString) {
	_ppEvent.Coordinates = _Coordinates;
	_ppEvent.Job = FC_PPEVENT_CHAT;
	_ppEvent.Message.assign(rString);
	_ppEvent.pThread = this;
	_pPoolMaster->Event(_ppEvent);
}

void PlayerThread::PlayerInfoList(bool fSpawn,string& rName) {
	if (fSpawn) {
		if (_Spawned_PlayerInfoList == 60) {return;} //Workaround for a Minecraft render bug
		_Spawned_PlayerInfoList++;
	}else{
		_Spawned_PlayerInfoList--;
	}

	_highNetwork.Lock();
	_highNetwork.addByte(0xc9);
	_highNetwork.addString(rName);
	_highNetwork.addBool(fSpawn);
	_highNetwork.addShort(10);
	_highNetwork.Flush();
	_highNetwork.UnLock();
}

void PlayerThread::sendLowClientPosition() {
	_lowNetwork.Lock();

	_lowNetwork.addByte(0x0D);
	_lowNetwork.addDouble(_Coordinates.X);
	_lowNetwork.addDouble(_Coordinates.Stance);
	_lowNetwork.addDouble(_Coordinates.Y);
	_lowNetwork.addDouble(_Coordinates.Z);
	_lowNetwork.addFloat(_Coordinates.Yaw);
	_lowNetwork.addFloat(_Coordinates.Pitch);
	_lowNetwork.addBool(_Coordinates.OnGround);
	_lowNetwork.Flush();

	_lowNetwork.UnLock();
}