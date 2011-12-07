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
	_SendQueue(),
	_ChatQueue(),
	_sConnectionHash(""),
	_Web_Session("session.minecraft.net"),
	_Web_Response(),
	_Network(&_SendQueue),
	_rEntityProvider(rEntityProvider),
	_Flags(),
	_ChunkProvider(rChunkRoot,_Network,rPackingThread,this)
{
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
	_pPoolMaster = pPoolMaster;

	_fSpawned = false;
	_fAssigned = false;
	_iThreadTicks = 0;
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
			ProcessQueue();

			iPacket = _Network.readByte();

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
		_ChunkProvider.Disconnect();
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

	ClearQueue();
	_Network.closeConnection();
}


void PlayerThread::Connect(Poco::Net::StreamSocket& Sock) {
	if (_fAssigned) {
		cout<<"***INTERNAL SERVER WARNING: PlayerPool tryed to assign an already assigned player thread"<<"\n";
		Disconnect(FC_LEAVE_OTHER);
	}
	_fAssigned=true;

	_Connection = Sock; 
	_Network.newConnection(Sock);

	_sIP.assign(_Connection.peerAddress().toString());
	_iThreadTicks = 0;
}

void PlayerThread::Kick(string sReason) {
	_sTemp.clear();
	_sTemp.append<char>(1,0xFF);
	NetworkIO::packString(_sTemp,sReason);
	_Connection.sendBytes(_sTemp.c_str(),_sTemp.length());

//	Thread::sleep(100);
	if (isSpawned()) { // If names is known
		cout<<_sName<<" was kicked for: "<<sReason<<"\n"; 
	}
	Disconnect(FC_LEAVE_KICK);
}

void PlayerThread::Kick() {
	_sTemp.clear();
	_sTemp.append<char>(1,0xFF);
	_sTemp.append<char>(2,0x0);
	_Connection.sendBytes(_sTemp.c_str(),_sTemp.length());

	//Thread::sleep(100);
	if (isSpawned()) { // If names is known
		cout<<_sName<<" was kicked"<<"\n"; 
	}
	Disconnect(FC_LEAVE_KICK);
}


void PlayerThread::generateConnectionHash() {
	std::stringstream StringStream;
	StringStream<<std::hex<< Random::uInt64();
	_sConnectionHash.assign(StringStream.str());
}

void PlayerThread::Interval_Time() {
	if (_TimeJobs.LastTimeSend + FC_INTERVAL_TIMESEND <= getTicks()) {
		_TimeJobs.LastTimeSend = getTicks();
		sendTime();
	}
}


void PlayerThread::sendTime() {
	_Network.Lock();
	_Network.addByte(0x4);
	_Network.addInt64(ServerTime::getTime());
	_Network.Flush();
	_Network.UnLock();
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

void PlayerThread::Interval_KeepAlive() {
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


bool PlayerThread::isSpawned() {
	return _fSpawned;
}


/*
* QUEUE
*/
void PlayerThread::appendQueue(string& rString) {
	_SendQueue.push(rString);
}

void PlayerThread::insertChat(string& rString) {
	_ChatQueue.push(rString);
}

void PlayerThread::ClearQueue() {
	while (_SendQueue.size()){	
		_SendQueue.pop();
	}
}

void PlayerThread::ProcessQueue() {
	string Buffer("");
	string Message("");
	int iChunkCount = 0;

	//Chat queue
	while (_ChatQueue.size()) {
		Message = _ChatQueue.front();
		_ChatQueue.pop();

		Buffer.clear();
		Buffer.append<char>(1,3);

		NetworkIO::packString(Buffer,Message);

		_Connection.sendBytes(Buffer.c_str(),Buffer.length());
	}

	//packet queue
	while (_SendQueue.size()) {
		_Connection.sendBytes(_SendQueue.front().c_str(),_SendQueue.front().length());
		
		if (_SendQueue.front().c_str()[0] == 0x32) { 
			_SendQueue.pop();
			continue; 
		} //Pre chunks are so tiny, they  can be send in a row

		_SendQueue.pop();
		break;
	}
}


/*
* Packets
*/

void PlayerThread::Packet0_KeepAlive() {
	_Network.readInt(); //Get id
}

void PlayerThread::Packet1_Login() {
	int iProtocolVersion = 0;

	//Check minecraft version
	iProtocolVersion = _Network.readInt(); //Protocol Version

	if (iProtocolVersion > SettingsHandler::getSupportedProtocolVersion()) {
		Kick("Outdated server!");
		return;
	}

	if (iProtocolVersion <  SettingsHandler::getSupportedProtocolVersion()) {
		Kick("Outdated client!");
		return;
	}

	_Network.readString(); //Username (already known)	
	_Network.read(16);

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


	//YES DUDE, you got it :D

	_PlayerCount++; //There is an new spawned player
	_iEntityID = _rEntityProvider.Add(FC_ENTITY_PLAYER); //Fetch a new entity id
	_fSpawned = true;
	_ChunkProvider.newConnection();


	//Set start coordinates
	_Coordinates.X = 100.0;
	_Coordinates.Y = 35.0;
	_Coordinates.Z = 100.0;
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
	_Network.Lock();

	_Network.addByte(0x1);
	_Network.addInt(_iEntityID);
	_Network.addString("");
	_Network.addInt64(SettingsHandler::getMapSeed());
	_Network.addInt(SettingsHandler::getServerMode());
	_Network.addByte(0);
	_Network.addByte(SettingsHandler::getDifficulty());
	_Network.addByte(SettingsHandler::getWorldHeight());
	_Network.addByte((unsigned char)SettingsHandler::getPlayerSlotCount());
	_Network.Flush();

	//compass
	_Network.addByte(0x6);
	_Network.addInt(0); //X
	_Network.addInt(0); // Y
	_Network.addInt(0); // Z 
	_Network.Flush();
	_Network.UnLock();

	//Client position
	sendClientPosition(); //Make client leave downloading map screen | chunk provider will send clients position again, after spawning chunk who standing on
	
	//Time
	sendTime(); 
	
	//Health
	UpdateHealth(20,20,5.0F); //Health

	//Chunks
	_ChunkProvider.HandleMovement(_Coordinates);
}

void PlayerThread::Packet2_Handshake() {
	_sName = _Network.readString();

	if (_sName.length() > 16) {
		Kick("Username too long");
		return;
	}

	//Send response (Connection Hash)
	_Network.Lock();
	_Network.addByte(0x02);

	if (SettingsHandler::isOnlineModeActivated()) {
		generateConnectionHash();
		_Network.addString(_sConnectionHash);
	}else{
		_Network.addString("-");
	}
	_Network.Flush();
	_Network.UnLock();
}

void PlayerThread::Packet3_Chat() {
	string Message("");
	
	Message = _Network.readString();
	if (Message.length() > 100) {
		Kick("Received string too long");
		return;
	}

	_sTemp.assign("<");
	_sTemp.append(_sName);
	_sTemp.append("> ");
	_sTemp.append(Message); 

	pushChatEvent(_sTemp);
}

void PlayerThread::Packet10_Player() {
	_Coordinates.OnGround = _Network.readBool();
}

void PlayerThread::Packet11_Position() {
	EntityCoordinates TmpCoord;

	//Read coordinates in a temporary variable
	TmpCoord.X = _Network.readDouble();
	TmpCoord.Y = _Network.readDouble();
	TmpCoord.Stance = _Network.readDouble();
	TmpCoord.Z = _Network.readDouble();
	TmpCoord.OnGround = _Network.readBool();

	//if X and Z ==  8.5000000000000000 , there is crap in the tcp buffer -> ignore it
	if (TmpCoord.X == 8.5000000000000000 && TmpCoord.Z == 8.5000000000000000) { 
		return;
	}else{
		_Coordinates.X = TmpCoord.X;
		_Coordinates.Y = TmpCoord.Y;
		_Coordinates.Stance = TmpCoord.Stance;
		_Coordinates.Z = TmpCoord.Z;
		_Coordinates.OnGround = TmpCoord.OnGround;
		_ChunkProvider.HandleMovement(_Coordinates);
	}
}

void PlayerThread::Packet12_Look() {
	_Coordinates.Yaw = _Network.readFloat();
	_Coordinates.Pitch = _Network.readFloat();
	_Coordinates.OnGround = _Network.readBool();
}

void PlayerThread::Packet13_PosAndLook() {
	EntityCoordinates TmpCoord;

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
		return;
	}else{
		_Coordinates = TmpCoord;
		_ChunkProvider.HandleMovement(_Coordinates);
	}
}

void PlayerThread::Packet254_ServerListPing() {
	_sTemp.clear();
	_sTemp.assign(SettingsHandler::getServerDescription()); //Server Description
	_sTemp.append("§");
	Poco::NumberFormatter::append(_sTemp,_PlayerCount); //Player count
	_sTemp.append("§");
	Poco::NumberFormatter::append(_sTemp,SettingsHandler::getPlayerSlotCount()); //player slots
	_sTemp.append("§");

	Kick(_sTemp);
}

void PlayerThread::Packet255_Disconnect() {
	_sLeaveMessage = _Network.readString();
	Disconnect(FC_LEAVE_QUIT);
}

void PlayerThread::pushChatEvent(string& rString) {
	_ppEvent.Coordinates = _Coordinates;
	_ppEvent.Job = FC_PPEVENT_CHAT;
	_ppEvent.Message.assign(rString);
	_ppEvent.pThread = this;
	_pPoolMaster->Event(_ppEvent);
}