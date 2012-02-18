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

#include "PlayerThread.h"
#include "EntityID.h"
#include "SettingsHandler.h"
#include "ServerTime.h"
#include "PlayerPool.h"
#include "Constants.h"
#include "World.h"
#include "ItemInfoStorage.h"
#include "PlayerEvents.h"
#include "MathHelper.h"
#include <sstream>
#include <istream>
#include <Poco/Timespan.h>
#include <Poco/Net/NetException.h>
#include <Poco/Stopwatch.h>
#include <math.h>
#include "Structs.h"
#include "ChunkMath.h"
#include "WorldStorage.h"

using Poco::Thread;
using std::cout;
using std::endl;
using std::dec;

PlayerThread::PlayerThread(PlayerPool* pPoolMaster,
	PackingThread& rPackingThread
	) : 
_sName(""),
	_sIP(""),
	_sLeaveMessage(""),
	_Connection(),
	_sTemp(""),
	_sConnectionHash(""),
	_WorldWhoIn(""),

	_NetworkOutRoot(),
	_NetworkInRoot(_Connection),
	_NetworkWriter(_NetworkOutRoot.getLowQueue(),_NetworkOutRoot.getHighQueue(),_Connection,this),

	_Web_Session("session.minecraft.net"),
	_Web_Response(),
	_Flags(),
	_Rand(),
	_threadNetworkWriter("NetworkWriter"),
	_ChunkProvider(_NetworkOutRoot,rPackingThread,this),
	_Inventory(_NetworkOutRoot,_NetworkInRoot),
	_vSpawnedEntities(0),

	_iThreadTicks(0),

	_timespanSendTime(&_iThreadTicks,FC_INTERVAL_TIMESEND),
	_timespanSendKeepAlive(&_iThreadTicks,FC_INTERVAL_KEEPALIVE),
	_timespanHandleMovement(&_iThreadTicks,FC_INTERVAL_HANDLEMOVEMENT),
	_timespanMovementSent(&_iThreadTicks,FC_INTERVAL_MOVEMENT),
	_timespanSpeedCalculation(&_iThreadTicks,FC_INTERVAL_CALCULATESPEED),
	_timespanPositionCheck(&_iThreadTicks,FC_INTERVAL_CHECKPOSITION),

	_timerLastBlockPlace(&_iThreadTicks,0L),
	_timerStartedEating(&_iThreadTicks,0L),
	_timerStartedDigging(&_iThreadTicks,0L),
	_timerLastAlivePacketSent(&_iThreadTicks,0L)
{
	_Coordinates.OnGround = false;
	_Coordinates.Pitch = 0.0F;
	_Coordinates.Stance = 0.0;
	_Coordinates.X = 0.0;
	_Coordinates.Y = 0.0;
	_Coordinates.Yaw = 0.0F;
	_Coordinates.Z = 0.0;


	_dRunnedMeters=0.0;

	_iEntityID=0;
	_Spawned_PlayerInfoList = 0;
	_pPoolMaster = pPoolMaster;
	_pWorld=NULL;

	_iPlayerPing= -1L;

	_fSpawned = false;
	_fAssigned = false;
	_fHandshakeSent=false;
	_fRunning=false;

	//Start NetworkWriter Thread
	_threadNetworkWriter.start(_NetworkWriter);
}

int PlayerThread::_PlayerCount = 0;

PlayerThread::~PlayerThread() {
	_NetworkWriter.shutdown();
	_vSpawnedEntities.clear();
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

	_fRunning=true;

	Timer.start();
	while (_fRunning) {
		if (!isAssigned()) {
			while(!isAssigned()) {
				if (_fRunning==false) { break; }
				Thread::sleep(50);
			}
			if (_fRunning==false) { continue; }
			Timer.reset();
		} 


		try {
			//Thread Ticks
			Timer.stop();
			_iThreadTicks += Timer.elapsed() / 1000;
			Timer.reset();
			Timer.start();


			if (!_fSpawned && _iThreadTicks >= FC_MAXLOGINTIME) {
				Disconnect("Login timed out");
				continue;
			}

			Interval_KeepAlive(); 
			Interval_Time();
			Interval_HandleMovement();
			Interval_Movement();
			Interval_CalculateSpeed();
			Interval_CheckPosition();
			Interval_CheckEating();


			iPacket = _NetworkInRoot.readByte();

			if (!_fSpawned &&  (iPacket!=0x01 && iPacket!=0x02 && iPacket!=0xFE)) {
				Disconnect("Login not done!");
				continue;
			}

			//cout<<"Package recovered:"<<std::hex<<int(iPacket)<<"\n";
			switch (iPacket) {
			case 0x0:
				Packet0_KeepAlive();
				break;
			case 0x1:
				if (isSpawned()) {
					Disconnect("Login already done!");
					continue;
				}
				Packet1_Login();
				break;
			case 0x2:
				if (isSpawned()) {
					Disconnect("Login already done!");
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
			case 0xe:
				Packet14_Digging();
				break;
			case 0xf: 
				Packet15_PlayerBlockPlacement();
				break;
			case 0x10: 
				Packet16_HoldingChange();
				break;
			case 0x12: 
				Packet18_Animation();
				break;
			case 0x13: 
				Packet19_EntityAction();
				break;
			case 0x65: 
				Packet101_CloseWindow();
				break;
			case 0x66:
				Packet102_WindowClick();
				break;
			case 0xFE: //Server List Ping
				Packet254_ServerListPing();
				break;
			case 0xFF: //Disconnect
				Packet255_Disconnect();
				break;
			default: 
				Disconnect("Unknown packet!");
				cout<<"Unknown packet received! 0x"<<std::hex<<int(iPacket)<<endl;
				break;
			}
		} catch (Poco::RuntimeException) {
			Disconnect(FC_LEAVE_OTHER);
		}
	}
	if(_fAssigned){Disconnect(FC_LEAVE_OTHER);}
	_fRunning=true;
}

void PlayerThread::Disconnect(string sReason,bool fShow) {
	if (!isAssigned()) {return;}
	if(isSpawned()) {
		_fSpawned=false;
		_NetworkWriter.clearQueues();

		stringstream strStrm;
		strStrm<<_sName<<" was kicked"<< (sReason.compare("")==0 ? "." : " for: ")<<sReason; 
		cout<<strStrm.str()<<"\n"; //Writer kick message to chat

		NetworkOut Out(&_NetworkOutRoot);
		Out.addByte(0xFF);
		Out.addString(sReason);
		Out.Finalize(FC_QUEUE_HIGH);

		ProcessQueue();
		Disconnect(FC_LEAVE_KICK);

		if (fShow) { //Write kick message
			PlayerEventBase* p = new PlayerChatEvent(this,"§6" + strStrm.str(),_Coordinates);
			_pPoolMaster->addEvent(p);
		}
	}else{
		NetworkOut Out(&_NetworkOutRoot);
		Out.addByte(0xFF);
		Out.addString(sReason);
		Out.Finalize(FC_QUEUE_HIGH);
		ProcessQueue();
		Disconnect(FC_LEAVE_QUIT);
	}
}

void PlayerThread::Disconnect(char iLeaveMode) {
	if (!_fAssigned) { return; }

	if (isSpawned() || iLeaveMode == FC_LEAVE_KICK) {
		_ChunkProvider.HandleDisconnect();
		_Inventory.HandleDisconnect();
		_PlayerCount--;

		PlayerEventBase* p = new PlayerDisconnectEvent(this,_iEntityID,_sName);

		switch (iLeaveMode) {
		case FC_LEAVE_QUIT:		
			cout<<_sName<<" quit ("<<_sLeaveMessage<<")"<<"\n";
			break;
		case FC_LEAVE_OTHER:
			cout<<_sName<<" quit (unknown reason)"<<"\n";
			break;
		}

		_pPoolMaster->addEvent(p);
	}

	_Flags.clear();

	_sName.clear();
	_sIP.clear();
	if (iLeaveMode != FC_LEAVE_KICK) {_NetworkWriter.clearQueues();}

	_dRunnedMeters=0;
	_Spawned_PlayerInfoList=0;
	_iPlayerPing= -1L;

	_Connection.close();
	_vSpawnedEntities.clear();
	_fSpawned = false;
	_fAssigned = false;
	_fHandshakeSent=false;
}


void PlayerThread::Connect(Poco::Net::StreamSocket& Sock) {
	if (_fAssigned) {
		cout<<"***INTERNAL SERVER WARNING: PlayerPool tryed to assign an already assigned player thread"<<"\n";
		Disconnect(FC_LEAVE_OTHER);
	}
	_fAssigned=true;

	_Connection = Sock; 
	_Connection.setLinger(true,5);
	_Connection.setNoDelay(false);
	_Connection.setBlocking(true);

	_Inventory.clear();
	_sIP.assign(_Connection.peerAddress().toString());

	_iThreadTicks = 0;

	_timespanSendTime.reset();
	_timespanSendKeepAlive.reset();
	_timespanHandleMovement.reset();
	_timespanMovementSent.reset();
	_timespanSpeedCalculation.reset();
	_timespanPositionCheck.reset();

	_timerLastBlockPlace.reset();
	_timerStartedEating.reset();
	_timerStartedDigging.reset();
}

string PlayerThread::generateConnectionHash() {
	std::stringstream StringStream;
	long long iVal = 0;


	while ((iVal&0xFFFFFFFFFFFFFF00) == 0) {
		iVal = ((((long long)_Rand.next())<<32) & 0xFFFFFFFF00000000)  |  
			(((long long)_Rand.next()) & 0x00000000FFFFFFFF);
	}

	StringStream<<std::hex<<iVal;
	_sConnectionHash.assign(StringStream.str());
	return _sConnectionHash;
}

void PlayerThread::Interval_Time() {
	if (!isSpawned()) {return;}
	if (_timespanSendTime.isGone()) {
		_timespanSendTime.reset();
		sendTime();
	}
}

void PlayerThread::Interval_HandleMovement() {
	if (!isSpawned()) {return;}
	if (_timespanHandleMovement.isGone()) {
		_timespanHandleMovement.reset();
		_ChunkProvider.HandleMovement(_Coordinates);
	}
}

void PlayerThread::sendTime() {
	NetworkOut Out(&_NetworkOutRoot);
	Out.addByte(0x4);
	Out.addInt64(ServerTime::getTime());
	Out.Finalize(FC_QUEUE_HIGH);
}

long long PlayerThread::getTicks() {
	return _iThreadTicks;
}


void PlayerThread::sendClientPosition() {
	NetworkOut Out(&_NetworkOutRoot);

	Out.addByte(0x0D);
	Out.addDouble(_Coordinates.X);

	switch(_Flags.isCrouched()) {//FastCraft will never modify Stance so we have to calculate it
	case true:
		_Coordinates.Stance = _Coordinates.Y + 1.5; 
		break;
	case false:
		_Coordinates.Stance = _Coordinates.Y + 1.6;
		break;
	}
	Out.addDouble(_Coordinates.Stance);
	Out.addDouble(_Coordinates.Y);
	Out.addDouble(_Coordinates.Z);
	Out.addFloat(_Coordinates.Yaw);
	Out.addFloat(_Coordinates.Pitch);
	Out.addBool(_Coordinates.OnGround);

	Out.Finalize(FC_QUEUE_HIGH);
}

void PlayerThread::Interval_KeepAlive() {
	if (!isSpawned()){return;}
	if (_timespanSendKeepAlive.isGone()) { //Send new keep alive
		_timespanSendKeepAlive.reset();
		sendKeepAlive();
	}
}

void PlayerThread::syncHealth() {
	NetworkOut Out(&_NetworkOutRoot);
	Out.addByte(0x8);
	Out.addShort(fixRange<short>(_iHealth,0,20));
	Out.addShort(fixRange<short>(_iFood,0,20));
	Out.addFloat(fixRange<float>(_nSaturation,0.0F,5.0F));
	Out.Finalize(FC_QUEUE_HIGH);
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


void PlayerThread::insertChat(string rString) {
	NetworkOut Out(&_NetworkOutRoot);
	Out.addByte(0x3);
	Out.addString(rString);
	Out.Finalize(FC_QUEUE_HIGH);
}


void PlayerThread::ProcessQueue() {			
	//All elements from high queue
	while (!_NetworkOutRoot.getHighQueue().empty()) {
		string & rJob = _NetworkOutRoot.getHighQueue().front();

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


		_NetworkOutRoot.getHighQueue().pop();
	}
}

void PlayerThread::Packet0_KeepAlive() {
	_NetworkInRoot.readInt(); //Get id
	_iPlayerPing = _timerLastAlivePacketSent.getGoneTime();
	cout<<"ping:"<<_iPlayerPing<<"\n";
}

void PlayerThread::Packet1_Login() {
	int iProtocolVersion = 0;
	if (!_fHandshakeSent) { Disconnect("Incorrect login order!",false); }

	try {
		//Check minecraft version
		iProtocolVersion = _NetworkInRoot.readInt(); //Protocol Version


		if (iProtocolVersion > SettingsHandler::getSupportedProtocolVersion()) {
			Disconnect("Outdated server! Needed Version: " + SettingsHandler::getSupportedMCVersion());
			return;
		}

		if (iProtocolVersion <  SettingsHandler::getSupportedProtocolVersion()) {
			Disconnect("Outdated client! Needed Version: " + SettingsHandler::getSupportedMCVersion());
			return;
		}

		_NetworkInRoot.readString(); //Username (already known)	
		_NetworkInRoot.readInt64();
		_NetworkInRoot.readString();
		_NetworkInRoot.readInt();
		_NetworkInRoot.readInt(); //This are 4 unused byte fields

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
				Disconnect("Failed to verify username!",false);
				return;
			}
		}

		//Check if there is a player with same name
		PlayerThread* pPlayer = _pPoolMaster->getPlayerByName(_sName,this);
		if (pPlayer != NULL) {
			pPlayer->Disconnect("Logged in from another location.",false);
		}


		//YES DUDE, you got it !
		_PlayerCount++; //There is an new spawned player
		_iEntityID = EntityID::New(); //Fetch a new ID

		_pWorld = WorldStorage::getWorldByName(SettingsHandler::getMainWorldName()); //Pointer to map who actual in
		_WorldWhoIn.assign(SettingsHandler::getMainWorldName());

		//Set start coordinates
		_Coordinates.X = 10.0;
		_Coordinates.Y = 0.0;
		_Coordinates.Z = 0.0;
		_Coordinates.Stance = 0.0;
		_Coordinates.OnGround = false;
		_Coordinates.Pitch = 0.0F;
		_Coordinates.Yaw = 0.0F;
		_lastCoordinates = _Coordinates;
		CheckPosition(false);

		/*
		* Response
		*/
		//Login response
		NetworkOut Out(&_NetworkOutRoot);

		Out.addByte(0x1);
		Out.addInt(_iEntityID);
		Out.addString("");
		Out.addInt64(-345L);
		Out.addString("SUPERFLAT");
		Out.addInt(SettingsHandler::getServerMode());
		Out.addByte(0);
		Out.addByte(SettingsHandler::getDifficulty());
		Out.addByte(SettingsHandler::getWorldHeight());
		Out.addByte((unsigned char)SettingsHandler::getPlayerSlotCount());

		Out.Finalize(FC_QUEUE_HIGH);

		//compass
		Out.addByte(0x6);
		Out.addInt(0); //X
		Out.addInt(0); // Y
		Out.addInt(0); // Z 
		Out.Finalize(FC_QUEUE_HIGH);

		//Time
		sendTime();

		_ChunkProvider.HandleNewPlayer();
		_ChunkProvider.HandleMovement(_Coordinates); //Pre Chunks

		//Health
		_iHealth= 10;
		_iFood=0;
		_nSaturation=0.0F;
		syncHealth();

		//Inventory
		ItemSlot Item1(std::make_pair(276,0),1);
		ItemSlot Item2(std::make_pair(360,0),64);
		ItemSlot Item3(std::make_pair(35,4),64);

		_Inventory.setSlot(38,Item1);
		_Inventory.setSlot(37,Item2);
		_Inventory.setSlot(36,Item3);
		_Inventory.synchronizeInventory();

		sendClientPosition();

		insertChat("§dWelcome to FastCraft 0.0.2 Alpha server.");
		ProcessQueue(); //Send login packages

		sendKeepAlive(); //Measure ping
		
		_fSpawned = true;	
		PlayerEventBase* p = new PlayerJoinEvent(this);//Push PlayerPool Join event
		_pPoolMaster->addEvent(p);
		

		//Spawn own name to playerinfo
		PlayerInfoList(true,_sName);

		//Spawn other player
		vector<string> vNames;
		vNames = _pPoolMaster->ListPlayers(59);

		if (!vNames.empty()) {
			for ( int x = 0;x<= vNames.size()-1;x++) {
				if(vNames[x].compare(_sName)==0) {continue;}//No double spawning of own name
				PlayerInfoList(true,vNames[x]);
			}
		}
	} catch(Poco::RuntimeException) {
		Disconnect(FC_LEAVE_OTHER);
	}
	cout<<_sName<<" joined ("<<_sIP<<") EID:"<<_iEntityID<<endl;  //Console log
}

void PlayerThread::Packet2_Handshake() {
	try {
		_sName = _NetworkInRoot.readString();
	} catch(Poco::RuntimeException) {
		Disconnect(FC_LEAVE_OTHER);
	}

	if (_sName.length() > 16) {
		Disconnect("Username too long");
		return;
	}

	//Send response (Connection Hash)
	NetworkOut Out(&_NetworkOutRoot);
	Out.addByte(0x02);

	if (SettingsHandler::isOnlineModeActivated()) {
		Out.addString(generateConnectionHash());
	}else{
		Out.addString("-");
	}
	Out.Finalize(FC_QUEUE_HIGH);

	_fHandshakeSent = true;
	ProcessQueue();
}

void PlayerThread::Packet3_Chat() {
	string Message("");

	try {
		Message = _NetworkInRoot.readString();
		if (Message.length() > 100) {
			Disconnect("Received string too long");
			return;
		}
	} catch(Poco::RuntimeException) {
		Disconnect(FC_LEAVE_OTHER);
	}

	_sTemp.assign("<§c");
	_sTemp.append(_sName);
	_sTemp.append("§f> ");
	_sTemp.append(Message); 

	ChatToAll(_sTemp);
}

void PlayerThread::Packet10_Player() {
	if (isSpawned()) {
		_ChunkProvider.NextChunk();
	}
	try {
		_Coordinates.OnGround = _NetworkInRoot.readBool();
	} catch(Poco::RuntimeException) {
		Disconnect(FC_LEAVE_OTHER);
	}
}

void PlayerThread::Packet11_Position() {
	EntityCoordinates TmpCoord;
	if (isSpawned()) {
		_ChunkProvider.NextChunk();
	}
	try {
		//Read coordinates in a temporary variable
		TmpCoord.X = _NetworkInRoot.readDouble();
		TmpCoord.Y = _NetworkInRoot.readDouble();
		TmpCoord.Stance = _NetworkInRoot.readDouble();
		TmpCoord.Z = _NetworkInRoot.readDouble();
		TmpCoord.OnGround = _NetworkInRoot.readBool();

		//if X and Z ==  8.5000000000000000 , there is crap in the tcp buffer -> ignore it
		if (TmpCoord.X == 8.5000000000000000 && TmpCoord.Z == 8.5000000000000000) { 
			return;
		}else{
			_dRunnedMeters += MathHelper::distance2D(_Coordinates,TmpCoord);
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
	if (isSpawned()) {
		_ChunkProvider.NextChunk();
	}
	try {
		_Coordinates.Yaw = _NetworkInRoot.readFloat();
		_Coordinates.Pitch = _NetworkInRoot.readFloat();
		_Coordinates.OnGround = _NetworkInRoot.readBool();
	} catch(Poco::RuntimeException) {
		Disconnect(FC_LEAVE_OTHER);
	}
}

void PlayerThread::Packet13_PosAndLook() {
	EntityCoordinates TmpCoord;

	if (isSpawned()) {
		_ChunkProvider.NextChunk();
	}

	//Read coordinates in a temporary variable
	try {
		TmpCoord.X = _NetworkInRoot.readDouble();
		TmpCoord.Y = _NetworkInRoot.readDouble();
		TmpCoord.Stance = _NetworkInRoot.readDouble();
		TmpCoord.Z = _NetworkInRoot.readDouble();
		TmpCoord.Yaw = _NetworkInRoot.readFloat();
		TmpCoord.Pitch = _NetworkInRoot.readFloat();
		TmpCoord.OnGround = _NetworkInRoot.readBool();

		//if X and Z ==  8.5000000000000000 , there is crap in the tcp buffer -> ignore it
		if (TmpCoord.X == 8.5000000000000000 && TmpCoord.Z == 8.5000000000000000) { 
			return;
		}else{
			_dRunnedMeters += MathHelper::distance2D(_Coordinates,TmpCoord);
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

	Disconnect(_sTemp);
}

void PlayerThread::Packet255_Disconnect() {
	try {
		_sLeaveMessage = _NetworkInRoot.readString();
	} catch(Poco::RuntimeException) {
		Disconnect(FC_LEAVE_OTHER);
	}
	Disconnect(FC_LEAVE_QUIT);
}

void PlayerThread::ChatToAll(string& rString) {
	PlayerEventBase* p = new PlayerChatEvent(this,rString,_Coordinates);
	_pPoolMaster->addEvent(p);
}

void PlayerThread::PlayerInfoList(bool fSpawn,string Name) {
	if (fSpawn) {
		if (_Spawned_PlayerInfoList == 60) {return;} //Workaround for a Minecraft render bug
		_Spawned_PlayerInfoList++;
	}else{
		_Spawned_PlayerInfoList--;
	}

	NetworkOut Out(&_NetworkOutRoot);
	Out.addByte(0xc9);
	Out.addString(Name);
	Out.addBool(fSpawn);
	Out.addShort(10);
	Out.Finalize(FC_QUEUE_HIGH);
}

void PlayerThread::Packet16_HoldingChange() {
	short iSlot;
	try{
		iSlot = _NetworkInRoot.readShort();
		if (iSlot < 0 || iSlot > 8) {
			Disconnect("Illegal holding slotID");
			return;
		}
		_Inventory.HandleSelectionChange(iSlot);


		PlayerEventBase* p = new PlayerChangeHeldEvent(this,_Inventory.getSlot(36+iSlot).getItem(),0);
		_pPoolMaster->addEvent(p);
	} catch(Poco::RuntimeException) {
		Disconnect(FC_LEAVE_OTHER);
	}
}

void PlayerThread::Packet101_CloseWindow() {
	try {	
		char iWinID = _NetworkInRoot.readByte();
		if (iWinID == 0) {
			_Inventory.HandleWindowClose(_pPoolMaster);
		}
	} catch(Poco::RuntimeException) {
		Disconnect(FC_LEAVE_OTHER);
	}
}

void PlayerThread::Packet102_WindowClick() {
	ItemSlot aHeld[5];
	ItemID Item;
	Item.second=0;

	aHeld[0] = _Inventory.getSlot(36 + _Inventory.getSlotSelection());
	aHeld[1] = _Inventory.getSlot(8);
	aHeld[2] = _Inventory.getSlot(7);
	aHeld[3] = _Inventory.getSlot(6);
	aHeld[4] = _Inventory.getSlot(5);

	_Inventory.HandleWindowClick(this);	
	_Inventory.synchronizeInventory();


	if (aHeld[0].getItem() != _Inventory.getSlot(36 + _Inventory.getSlotSelection()).getItem()) { //Check if held item were changed

		PlayerEventBase* p = new PlayerChangeHeldEvent(this,_Inventory.getSlot(36+_Inventory.getSlotSelection()).getItem(),0);
		_pPoolMaster->addEvent(p);
	}

	int s=8;
	for (int x=1;x<=4;x++) {
		if (aHeld[x].getItem() != _Inventory.getSlot(s).getItem()) {//Check if boots were changed

			PlayerEventBase* p = new PlayerChangeHeldEvent(this,_Inventory.getSlot(s).getItem(),x);
			_pPoolMaster->addEvent(p);
		}
		s--;
	}
}

void PlayerThread::Interval_Movement() {
	if (!isSpawned()) {return;}

	if (_timespanMovementSent.isGone()) {
		if (!(_Coordinates == _lastCoordinates)) {
			PlayerEventBase* p = new PlayerMoveEvent(this,_Coordinates);
			_pPoolMaster->addEvent(p);

			_lastCoordinates = _Coordinates;
			_timespanMovementSent.reset();
		}else{ //If position not changed after 1 second, push a movement event to player pool (Workaound for the invisable player bug)
			if (_timespanMovementSent.getGoneTime() >= 1000) {
				PlayerEventBase* p = new PlayerMoveEvent(this,_Coordinates);
				_pPoolMaster->addEvent(p);
				_timespanMovementSent.reset();
			}
		}

	}
}

void PlayerThread::spawnPlayer(int ID,EntityPlayer& rPlayer) {
	if (isEntitySpawned(ID)) {
		throw Poco::RuntimeException("Already spawned!");
	}
	if (ID == _iEntityID) {
		throw Poco::RuntimeException("Own EntityID can't be spawned!");
	}

	EntityListEntry Entry;

	Entry.EntityID = ID;
	Entry.Type = FC_ENTITY_PLAYER;
	Entry.oldPosition = rPlayer._Coordinates;

	_vSpawnedEntities.push_back(Entry);

	NetworkOut Out(&_NetworkOutRoot);
	//Spawn player
	Out.addByte(0x14);
	Out.addInt(ID);
	Out.addString(rPlayer._sName);

	Out.addInt( (int) (rPlayer._Coordinates.X * 32.0));
	Out.addInt( (int) (rPlayer._Coordinates.Y * 32.0));
	Out.addInt( (int) (rPlayer._Coordinates.Z * 32.0));

	Out.addByte( (char) rPlayer._Coordinates.Yaw);
	Out.addByte( (char) rPlayer._Coordinates.Pitch);

	Out.addShort(rPlayer._aHeldItems[0].getItem().first);

	Out.Finalize(FC_QUEUE_HIGH);

	//Spawn his equipment
	for (int x=0;x<=4;x++) {
		Out.addByte(0x05);
		Out.addInt(ID);
		Out.addShort(x);
		if (rPlayer._aHeldItems[x].getItem().first == 0) {
			Out.addShort(-1);
		}else{
			Out.addShort(rPlayer._aHeldItems[x].getItem().first);
		}
		Out.addShort(rPlayer._aHeldItems[x].getItem().second); //Damage/Metadata
		Out.Finalize(FC_QUEUE_HIGH);
	}
}

bool PlayerThread::isEntitySpawned(int ID) {
	if (!_vSpawnedEntities.empty()) {
		for ( int x= 0;x<=_vSpawnedEntities.size()-1;x++) {
			if (_vSpawnedEntities[x].EntityID == ID){
				return true;
			}
		}
	}
	return false;
}

void PlayerThread::updateEntityPosition(int ID,EntityCoordinates Coordinates) {
	int id = -1;

	//search element
	if (!_vSpawnedEntities.empty()) {
		for (int x=0;x<=_vSpawnedEntities.size()-1;x++){
			if (_vSpawnedEntities[x].EntityID == ID){
				id = x;
			}
		}
	}else{
		throw Poco::RuntimeException("Not spawned");
	}
	if(id==-1) {
		throw Poco::RuntimeException("Not spawned");
	}


	if (_vSpawnedEntities[id].oldPosition == Coordinates) {
		return; //Coordinates are equal -> no update
	}

	NetworkOut Out(&_NetworkOutRoot);

	double dX =  Coordinates.X - _vSpawnedEntities[id].oldPosition.X;
	double dY =  Coordinates.Y - _vSpawnedEntities[id].oldPosition.Y;
	double dZ =  Coordinates.Z - _vSpawnedEntities[id].oldPosition.Z;

	//cout<<"dX:"<<dX<<" dY:"<<dY<<" dZ:"<<dZ<<"\n";

	if(_vSpawnedEntities[id].oldPosition.LookEqual(Coordinates)) {	//Player just moved around and doesn't change camera 
		if (fabs(dX) <= 4.0 && fabs(dY) <= 4.0 && fabs(dZ) <= 4.0 ) {//Movement under 4 blocks
			//relative move
			Out.addByte(0x1F);
			Out.addInt(ID);
			Out.addByte(   (char) (dX*32.0) );
			Out.addByte(   (char) (dY*32.0) );
			Out.addByte(   (char) (dZ*32.0) );
			Out.Finalize(FC_QUEUE_HIGH);
			_vSpawnedEntities[id].oldPosition = Coordinates;
			return;
		}else {
			//Full update
		}
	}else{ //player moved camera
		if (_vSpawnedEntities[id].oldPosition.CoordinatesEqual(Coordinates)) { //Just moved camera
			Out.addByte(0x20);
			Out.addInt(ID);
			Out.addByte( (char) ((Coordinates.Yaw * 256.0F) / 360.0F) );
			Out.addByte( (char) ((Coordinates.Pitch * 256.0F) / 360.0F) );
			Out.Finalize(FC_QUEUE_HIGH);
			_vSpawnedEntities[id].oldPosition = Coordinates;
			return;
		}
		if (fabs(dX) <= 4.0 && fabs(dY) <= 4.0 && fabs(dZ) <= 4.0 ) {//Movement under 4 blocks
			//relative move + camera
			Out.addByte(0x21);
			Out.addInt(ID);
			Out.addByte(   (char) (dX*32.0) );
			Out.addByte(   (char) (dY*32.0) );
			Out.addByte(   (char) (dZ*32.0) );
			Out.addByte( (char) ((Coordinates.Yaw * 256.0F) / 360.0F) );
			Out.addByte( (char) ((Coordinates.Pitch * 256.0F) / 360.0F) );
			Out.Finalize(FC_QUEUE_HIGH);
			_vSpawnedEntities[id].oldPosition = Coordinates;
			return;
		}else {
			//Full update
		}
	}

	Out.addByte(0x22);

	Out.addInt(ID);

	Out.addInt( (int) (Coordinates.X * 32.0));
	Out.addInt( (int) (Coordinates.Y * 32.0));
	Out.addInt( (int) (Coordinates.Z * 32.0));

	Out.addByte( (char) ((Coordinates.Yaw * 256.0F) / 360.0F) );
	Out.addByte( (char) ((Coordinates.Pitch * 256.0F) / 360.0F) );

	Out.Finalize(FC_QUEUE_HIGH);

	_vSpawnedEntities[id].oldPosition = Coordinates;
}

void PlayerThread::despawnEntity(int ID) {
	int id = -1;

	//search element
	if (!_vSpawnedEntities.empty()) {
		for (int x=0;x<=_vSpawnedEntities.size()-1;x++){
			if (_vSpawnedEntities[x].EntityID == ID){
				id = x;
			}
		}
	}else{
		throw Poco::RuntimeException("Not spawned");
	}
	if(id==-1) {
		throw Poco::RuntimeException("Not spawned");
	}

	_vSpawnedEntities.erase(_vSpawnedEntities.begin()+id);

	NetworkOut Out(&_NetworkOutRoot);
	Out.addByte(0x1D);
	Out.addInt(ID);
	Out.Finalize(FC_QUEUE_HIGH);
}

int PlayerThread::getEntityID() {
	return _iEntityID;
}

void PlayerThread::Packet18_Animation() {
	try {
		int iEID = _NetworkInRoot.readInt();
		char iAnimID = _NetworkInRoot.readByte();

		if (iEID != _iEntityID){
			Disconnect("You can't use other EntityID's as yours"); 
		}


		PlayerEventBase* p = new PlayerAnimationEvent(this,iAnimID);
		_pPoolMaster->addEvent(p);
	}catch(Poco::RuntimeException) {
		Disconnect(FC_LEAVE_OTHER);
	}
}

void PlayerThread::Packet19_EntityAction() {
	try {
		int iEID = _NetworkInRoot.readInt();
		char iActionID = _NetworkInRoot.readByte();

		if (iEID != _iEntityID){
			Disconnect("You can't use other EntityID's as yours"); 
		}

		switch(iActionID) {
		case FC_ACTION_CROUCH:
			if (_Flags.isCrouched()) {return;}
			_Flags.setCrouched(true);
			break;
		case FC_ACTION_UNCROUCH:
			if (!_Flags.isCrouched()) {return;}
			_Flags.setCrouched(false);
			break;
		case FC_ACTION_STARTSPRINTING:
			if (_Flags.isRightClicking()) {
				Disconnect("Non vanilla behavior (SP while C)");
				return;
			}
			if (_Flags.isSprinting()) {return;}
			_Flags.setSprinting(true);
			break;
		case FC_ACTION_STOPSPRINTING:
			if (!_Flags.isSprinting()) {return;}
			_Flags.setSprinting(false);
			break;
		default:
			Disconnect("Unsupported action.");
			break;
		}

		syncFlagsWithPP();
	}catch(Poco::RuntimeException) {
		Disconnect(FC_LEAVE_OTHER);
	}
}

void PlayerThread::playAnimationOnEntity(int ID,char AnimID) {
	if (!isEntitySpawned(ID)) {
		throw Poco::RuntimeException("Not spawned!");
	}

	NetworkOut Out(&_NetworkOutRoot);
	Out.addByte(0x12);
	Out.addInt(ID);
	Out.addByte(AnimID);

	Out.Finalize(FC_QUEUE_HIGH);
}


void PlayerThread::updateEntityMetadata(int ID,EntityFlags Flags) {
	if (!isEntitySpawned(ID)) {
		throw Poco::RuntimeException("Not spawned!");
	}
	NetworkOut Out(&_NetworkOutRoot);

	Out.addByte(0x28);
	Out.addInt(ID);


	char iMetadata = 0;
	//Flags 

	//Index 0 , general metadata
	Out.addByte(0); //Index = 0| Type=0
	iMetadata=0;
	iMetadata |= ((char)Flags.isOnFire()) & 1;
	iMetadata |= (((char)Flags.isCrouched()) & 1)<<1;
	iMetadata |= (((char)Flags.isRiding()) & 1 )<<2;
	iMetadata |= (((char)Flags.isSprinting()) & 1)<<3;
	iMetadata |= (((char)Flags.isRightClicking()) & 1)<<4;
	Out.addByte(iMetadata);

	Out.addByte(127); //End of metadata
	Out.Finalize(FC_QUEUE_HIGH);
}

void PlayerThread::Interval_CalculateSpeed() {
	if (!isSpawned()) {return;}
	if (_timespanSpeedCalculation.isGone()) {
		double iTime = double(_timespanSpeedCalculation.getGoneTime());
		_timespanSpeedCalculation.reset();


		double iSpeed = (_dRunnedMeters * 60.0 * 60.0) / (iTime*3.6);
		if (iSpeed > SettingsHandler::getMaxMovementSpeed()) {
			cout<<_sName<<" moved too fast ("<<int(iSpeed)<<" km/h)"<<"\n";
			Disconnect("You moved too fast!");
		}
		_dRunnedMeters=0.0;
	}
}

PlayerInventory& PlayerThread::getInventory() {
	return _Inventory;
}

EntityFlags PlayerThread::getFlags() {
	return _Flags;
}

void PlayerThread::updateEntityEquipment(int ID,short Slot,ItemID Item) {
	if (!isEntitySpawned(ID)) {
		throw Poco::RuntimeException("Not spawned!");
	}
	if (Slot < 0 || Slot > 4) {
		throw Poco::RuntimeException("Invalid slot");
	}

	NetworkOut Out(&_NetworkOutRoot);
	Out.addByte(0x05);
	Out.addInt(ID);
	Out.addShort(Slot);
	if (Item.first == 0) {
		Out.addShort(-1);
	}else{
		Out.addShort(Item.first);
	}
	Out.addShort(Item.second); //Damage/Metadata
	Out.Finalize(FC_QUEUE_HIGH);
}

void PlayerThread::CheckPosition(bool fSynchronize) {
	if (_pWorld->isSuffocating(_Coordinates)) {
		char iHeight = -1;
		BlockCoordinates blockCoords = ChunkMath::toBlockCoords(_Coordinates);
		while(iHeight == -1) {
			iHeight = _pWorld->getFreeSpace(blockCoords.X,blockCoords.Z);
			if (iHeight == -1) {
				_Coordinates.X += 1.0;
				_Coordinates.Z += 1.0;
				blockCoords = ChunkMath::toBlockCoords(_Coordinates);
				continue;
			}
		}

		_Coordinates.Y = ((double) iHeight) + 1.0;
		if (fSynchronize) {sendClientPosition();}
	}
}

void PlayerThread::Interval_CheckPosition() {
	if (!isSpawned()) {return;}
	if (_timespanPositionCheck.isGone()) {
		_timespanPositionCheck.reset();
		CheckPosition();
	}
}

void PlayerThread::Packet15_PlayerBlockPlacement() {
	try {
		char Direction;
		ItemSlot Slot;
		BlockCoordinates ClientCoordinats = ChunkMath::toBlockCoords(_Coordinates);
		BlockCoordinates blockCoordinates;

		blockCoordinates.X = _NetworkInRoot.readInt();
		blockCoordinates.Y = _NetworkInRoot.readByte();
		blockCoordinates.Z = _NetworkInRoot.readInt();
		Direction = _NetworkInRoot.readByte();
		Slot.readFromNetwork(_NetworkInRoot);


		if (_timerLastBlockPlace.getGoneTime() < 100) { //Client sends placeblock packet two times if you are aiming at a block
			return;
		}
		_timerLastBlockPlace.reset();

		_Inventory.getSelectedSlot().removeUnnecessarySubID();
		ItemSlot& InHand = _Inventory.getSelectedSlot();
		ItemID iSelectedBlock = std::make_pair(0,0);
		if (InHand.isEmpty()) {
			InHand.clear();
			return;
		}

		if (ItemInfoStorage::isBlock(InHand.getItem())) {
			iSelectedBlock = InHand.getItem();
		}

		if (blockCoordinates.X==-1 && blockCoordinates.Y == -1 && blockCoordinates.Z == -1 && Direction == -1) { //Special action

			//Special actions are not allowed with blocks
			if (ItemInfoStorage::isBlock(_Inventory.getSelectedSlot().getItem())) { 
				spawnBlock(blockCoordinates,std::make_pair(0,0));
				return;
			}

			//Eating
			if (ItemInfoStorage::getItem(_Inventory.getSelectedSlot().getItem()).Eatable) {
				if (_iFood==20) {return;}
				_timerStartedEating.reset();
				_Flags.setRightClicking(true);
				syncFlagsWithPP();
				return;
			}

			//Weapon blocking
			if (ItemInfoStorage::getItem(_Inventory.getSelectedSlot().getItem()).Weapon) {
				_Flags.setRightClicking(true);
				syncFlagsWithPP();
			}
			return; 
		}else{ //Client wants to place a block
			if (! ItemInfoStorage::isBlock(_Inventory.getSelectedSlot().getItem())) { //But it's a item...
				ItemEntry Entry = ItemInfoStorage::getItem(InHand.getItem());
				if (Entry.ConnectedBlock.first == -1 && Entry.ConnectedBlock.first == -1 ) { //No connected item
					return; 
				}
				iSelectedBlock = Entry.ConnectedBlock;
			}
		}

		switch (Direction) {
		case 0:
			blockCoordinates.Y--;
			break;
		case 1:
			blockCoordinates.Y++;
			break;
		case 2:
			blockCoordinates.Z--;
			break;
		case 3:
			blockCoordinates.Z++;
			break;
		case 4:
			blockCoordinates.X--;
			break;
		case 5:
			blockCoordinates.X++;
			break;
		default:
			Disconnect("Illegal block direction: " + int(Direction));
			return;
		}

		/*
		* Check blockCoordinates
		*/

		if(ItemInfoStorage::isSolid(iSelectedBlock)) {
			//Prevent: set a block into your body
			if ((ClientCoordinats.X == blockCoordinates.X && ClientCoordinats.Z == blockCoordinates.Z)  &&
				(ClientCoordinats.Y == blockCoordinates.Y || ClientCoordinats.Y+1==blockCoordinates.Y)) { 
					return;
			}

			//Prevent: set a block into other body
			if (_pPoolMaster->willHurtOther(blockCoordinates,this)) { 
				spawnBlock(blockCoordinates,std::make_pair(0,0));
				return;
			}
		}

		//Prevent: set a block into other block 
		//ToDo: Exception for liquids
		if (_pWorld->getBlock(blockCoordinates) != 0) { 
			return;
		}

		//Prevent: set a block above the map
		if (blockCoordinates.Y >= SettingsHandler::getWorldHeight()-2) { 
			spawnBlock(blockCoordinates,std::make_pair(0,0));
			return;
		}

		//Prevent: set a block under the map
		if(blockCoordinates.Y < 0) {
			return;
		}

		//Prevent: set too distant blocks
		if (MathHelper::distance3D(blockCoordinates,ClientCoordinats) > 6.0) {
			Disconnect("Target block too distant!");
			return;
		}


		//Prevent: set non solid blocks ontop of non solid blocks
		if (blockCoordinates.Y>0 && !ItemInfoStorage::isSolid(iSelectedBlock)) {
			BlockCoordinates temp = blockCoordinates;
			temp.Y--;

			char iBlock = _pWorld->getBlock(temp);
			ItemID iBlockBelow = std::make_pair( 
				short(iBlock),
				(ItemInfoStorage::getBlock(iBlock).hasSubBlocks ? _pWorld->getMetadata(temp) : 0)
				);
			switch(ItemInfoStorage::getBlock(iSelectedBlock).Stackable) {
			case true: //You can stack it, but it need a solid base block
				if (!ItemInfoStorage::getBlock(iBlockBelow).Solid && iBlockBelow.first != iSelectedBlock.first) {
					spawnBlock(blockCoordinates,std::make_pair(0,0));
					return;
				}
				break;
			case false: //Not stackable
				if( (iBlockBelow==iSelectedBlock) || 
					(!ItemInfoStorage::getBlock(iBlockBelow).Solid) //unsolid blocks need a solid ground
					) 
				{
					spawnBlock(blockCoordinates,std::make_pair(0,0));
					return;
				}
				break;
			}
		}


		/*
		* All test done. 
		*/
		//Send block acception
		spawnBlock(blockCoordinates,iSelectedBlock);


		//Send inventory update to player
		_Inventory.DecreaseInHandStack();


		//Append to map
		_pWorld->setBlock(blockCoordinates.X,blockCoordinates.Y,blockCoordinates.Z,char(iSelectedBlock.first));
		_pWorld->setMetadata(blockCoordinates.X,blockCoordinates.Y,blockCoordinates.Z,(char)iSelectedBlock.second);

		//Event to player pool
		PlayerEventBase* p = new PlayerSetBlockEvent(this,blockCoordinates,iSelectedBlock);
		_pPoolMaster->addEvent(p);
	} catch(Poco::RuntimeException& ex ) {
		cout<<"Exception cateched: "<<ex.message()<<"\n";
		Disconnect(FC_LEAVE_OTHER);
	}
}

int PlayerThread::getChunksInQueue() {
	return _NetworkOutRoot.getLowQueue().size();
}

void PlayerThread::spawnBlock(BlockCoordinates blockCoords,ItemID Item) {
	if (!ItemInfoStorage::isRegistered(Item)) {
		throw Poco::RuntimeException("Block not registered");
	}

	ChunkCoordinates Coords;
	Coords = ChunkMath::toChunkCoords(blockCoords);

	if (!_ChunkProvider.isSpawned(Coords)) {
		return;
	}

	NetworkOut Out(&_NetworkOutRoot);
	Out.addByte(0x35);
	Out.addInt(blockCoords.X);
	Out.addByte((char)blockCoords.Y);
	Out.addInt(blockCoords.Z);
	Out.addByte((unsigned char)Item.first);
	Out.addByte(Item.second);	
	Out.Finalize(FC_QUEUE_HIGH);
}


void PlayerThread::syncFlagsWithPP() {
	PlayerEventBase* p = new PlayerUpdateFlagsEvent(this,_Flags);
	_pPoolMaster->addEvent(p);
}

void PlayerThread::Packet14_Digging() {
	try {
		char iStatus,Y,iFace;
		int X,Z;

		iStatus = _NetworkInRoot.readByte();
		X = _NetworkInRoot.readInt();
		Y = _NetworkInRoot.readByte();
		Z = _NetworkInRoot.readInt();
		iFace = _NetworkInRoot.readByte();

		if (X == 0 && Y == 0 && Z == 0) { //Special case
			switch (iStatus) {
			case 4: //Drop item
				cout<<_sName<<" dropped item\n";
				break;
			case 5: //Shot arrow / finish eating
				if (_Flags.isRightClicking()) {
					_Flags.setRightClicking(false);
					syncFlagsWithPP();
					return;
				}
				break;
			default:
				Disconnect("Invalid digging status code!");
				return;
			}
		}


		cout<<"iStatus:"<<int(iStatus)<<"\tiFace:"<<int(iFace)<<"\n";
	} catch(Poco::RuntimeException& ex ) {
		cout<<"Exception cateched: "<<ex.message()<<"\n";
		Disconnect(FC_LEAVE_OTHER);
	}
}

string PlayerThread::getWorldWhoIn() {
	return _WorldWhoIn;
}

void PlayerThread::shutdown() {
	if (!_fRunning) {return;}
	_fRunning=false;
	while(!_fRunning){ //Wait till _fRunning turns true
	}
	_fRunning=false;
}

void PlayerThread::setEntityStatus(char iCode) {
	NetworkOut Out(&_NetworkOutRoot);

	Out.addByte(0x26);
	Out.addInt(_iEntityID);
	Out.addByte(iCode);
	Out.Finalize(FC_QUEUE_HIGH);
}

void PlayerThread::setEntityStatus(int iID,char iCode) {
	if (!isEntitySpawned(iID)) {
		cout<<"PlayerThread::setEntityStatus "<<iID<<" is not spawned!\n";
		return;
	}
	NetworkOut Out(&_NetworkOutRoot);

	Out.addByte(0x26);
	Out.addInt(iID);
	Out.addByte(iCode);
	Out.Finalize(FC_QUEUE_HIGH);
}

void PlayerThread::Interval_CheckEating() {
	if (!isSpawned()) {return;}
	handleEating();
}


void PlayerThread::handleEating() {
	if (!_Flags.isRightClicking()) {return;}

	ItemSlot& rInHandSlot = _Inventory.getSelectedSlot();
	if (rInHandSlot.isEmpty() || ItemInfoStorage::isBlock(rInHandSlot.getItem())) { //Block or empty
		_Flags.setRightClicking(false);
		syncFlagsWithPP();
		return;
	}

	ItemEntry IEntry = ItemInfoStorage::getItem(rInHandSlot.getItem());
	if (IEntry.Weapon) {return;}
	if (!IEntry.Eatable) {
		_Flags.setRightClicking(false);
		syncFlagsWithPP();
		return;
	}

	if (_timerStartedEating.getGoneTime() >= 1600 + (_iPlayerPing>500 ||_iPlayerPing == -1 ? 500 : _iPlayerPing)  ) {return;}
	if (_timerStartedEating.getGoneTime() >= 1500) {
		_iFood = fixRange<short>(_iFood+IEntry.FoodValue,0,20); 
		_nSaturation = fixRange<float>(_nSaturation + float(IEntry.FoodValue)*0.3F,0.0F,5.0F);
		
		setEntityStatus(FC_ENTITYSTATUS_ACCEPTEATING); //Accept eating
		syncHealth();
		_Inventory.DecreaseInHandStack();
		

		_Flags.setRightClicking(false);
		syncFlagsWithPP();
	}
}

void PlayerThread::sendKeepAlive() {
	_timerLastAlivePacketSent.reset();

	NetworkOut Out(&_NetworkOutRoot);
	Out.addByte(0x0);
	Out.addInt(_Rand.next());
	Out.Finalize(FC_QUEUE_HIGH);
}