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
	_vSpawnedEntities(0)
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
	_TimeJobs.LastMovementSend = 0L;
	_TimeJobs.LastSpeedCalculation=0L;
	_TimeJobs.LastPositionCheck=0L;
	_TimeJobs.LastBlockPlace=0L;
	_TimeJobs.StartedEating=0L;
	_dRunnedMeters=0.0;

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

			iPacket = _NetworkInRoot.readByte();

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
			/*case 0xe:
				Packet14_Digging();
				break;
			*/
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
			PlayerEventBase* p = new PlayerChatEvent(this,"�6" + strStrm.str(),_Coordinates);
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

	if (isSpawned() || iLeaveMode==FC_LEAVE_KICK) {
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
	if (iLeaveMode!=FC_LEAVE_KICK) {_NetworkWriter.clearQueues();}

	_TimeJobs.LastKeepAliveSend = 0L;
	_TimeJobs.LastTimeSend = 0L;
	_TimeJobs.LastHandleMovement = 0L;
	_TimeJobs.LastMovementSend = 0L;
	_TimeJobs.LastSpeedCalculation = 0L;
	_TimeJobs.LastPositionCheck=0L;
	_TimeJobs.LastBlockPlace=0L;
	_TimeJobs.StartedEating=0L;
	_dRunnedMeters=0;
	_Spawned_PlayerInfoList=0;

	_Connection.close();
	_vSpawnedEntities.clear();
	_fSpawned = false;
	_fAssigned = false;
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
}

string PlayerThread::generateConnectionHash() {
	std::stringstream StringStream;

	StringStream<<std::hex<<_Rand.next();
	if(StringStream.str().length() % 2) {
		StringStream<<"0";
	}
	StringStream<<std::hex<<_Rand.next();
	_sConnectionHash.assign(StringStream.str());
	return _sConnectionHash;
}

void PlayerThread::Interval_Time() {
	if (!isSpawned()) {return;}
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
	if (_TimeJobs.LastKeepAliveSend + FC_INTERVAL_KEEPACTIVE <= getTicks()) { //Send new keep alive
		_TimeJobs.LastKeepAliveSend = getTicks();

		NetworkOut Out(&_NetworkOutRoot);
		Out.addByte(0x0);
		Out.addInt(_Rand.next());
		Out.Finalize(FC_QUEUE_HIGH);
	}
}

void PlayerThread::UpdateHealth(short iHealth,short iFood,float nSaturation) {
	//Send Health
	_iHealth = fixRange<short>(iHealth,0,20);
	_iFood = fixRange<short>(iFood,0,20);
	_nSaturation = fixRange<float>(iFood,0.0F,5.0F);

	NetworkOut Out(&_NetworkOutRoot);
	Out.addByte(0x8);
	Out.addShort(_iHealth);
	Out.addShort(_iFood);
	Out.addFloat(_nSaturation);
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
}

void PlayerThread::Packet1_Login() {
	int iProtocolVersion = 0;

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
				Disconnect("Failed to verify username!");
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
		UpdateHealth(10,10,5.0F); //Health

		//Inventory
		ItemSlot Item1(1,64);
		ItemSlot Item2(5,64);
		ItemSlot Item3(260,64);

		_Inventory.setSlot(38,Item1);
		_Inventory.setSlot(37,Item2);
		_Inventory.setSlot(36,Item3);
		_Inventory.synchronizeInventory();

		sendClientPosition();

		insertChat("�dWelcome to FastCraft 0.0.2 Alpha server.");
		ProcessQueue(); //Send login packages

		PlayerEventBase* p = new PlayerJoinEvent(this);//Push PlayerPool Join event
		_pPoolMaster->addEvent(p);
		_fSpawned = true;	


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

	_sTemp.assign("<�c");
	_sTemp.append(_sName);
	_sTemp.append("�f> ");
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
	_sTemp.append("�");
	Poco::NumberFormatter::append(_sTemp,_PlayerCount); //Player count
	_sTemp.append("�");
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

		ItemID Item;
		Item.first = _Inventory.getSlot(36+iSlot).getItemID();
		Item.second=0;

		PlayerEventBase* p = new PlayerChangeHeldEvent(this,Item,0);
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
	//_Inventory.synchronizeInventory();


	if (aHeld[0].getItemID() != _Inventory.getSlot(36 + _Inventory.getSlotSelection()).getItemID()) { //Check if held item were changed
		Item.first = _Inventory.getSlot(36+_Inventory.getSlotSelection()).getItemID();
	
		PlayerEventBase* p = new PlayerChangeHeldEvent(this,Item,0);
		_pPoolMaster->addEvent(p);
	}

	int s=8;
	for (int x=1;x<=4;x++) {
		if (aHeld[x].getItemID() != _Inventory.getSlot(s).getItemID()) {//Check if boots were changed
		Item.first = _Inventory.getSlot(s).getItemID();
	
		PlayerEventBase* p = new PlayerChangeHeldEvent(this,Item,x);
		_pPoolMaster->addEvent(p);
		}
		s--;
	}
}

void PlayerThread::Interval_Movement() {
	if (!isSpawned()) {return;}
	if (_TimeJobs.LastMovementSend + FC_INTERVAL_MOVEMENT <= getTicks()) { //Send new keep alive
		if (!(_Coordinates == _lastCoordinates)) {
			PlayerEventBase* p = new PlayerMoveEvent(this,_Coordinates);
			_pPoolMaster->addEvent(p);

			_lastCoordinates = _Coordinates;
			_TimeJobs.LastMovementSend = getTicks();
		}else{
			if (_TimeJobs.LastMovementSend + 1000 <= getTicks()) {  //Workaound for the invisable player bug
				//Send a "still alive" event
				PlayerEventBase* p = new PlayerMoveEvent(this,_Coordinates);
				_pPoolMaster->addEvent(p);
				_TimeJobs.LastMovementSend = getTicks();
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

	Out.addShort(rPlayer._aHeldItems[0].getItemID());

	Out.Finalize(FC_QUEUE_HIGH);

	//Spawn his equipment
	for (int x=0;x<=4;x++) {
		Out.addByte(0x05);
		Out.addInt(ID);
		Out.addShort(x);
		if (rPlayer._aHeldItems[x].getItemID() == 0) {
			Out.addShort(-1);
		}else{
			Out.addShort(rPlayer._aHeldItems[x].getItemID());
		}
		Out.addShort(0); //Damage/Metadata
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
	if (_vSpawnedEntities.size()>0) {
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
			_Flags.setCrouched(true);
			break;
		case FC_ACTION_UNCROUCH:
			_Flags.setCrouched(false);
			break;
		case FC_ACTION_STARTSPRINTING:
			if (_Flags.isRightClicking()) {
				Disconnect("Non vanilla behavior (SP while C)");
				return;
			}
			_Flags.setSprinting(true);
			break;
		case FC_ACTION_STOPSPRINTING:
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
	if (_TimeJobs.LastSpeedCalculation + FC_INTERVAL_CALCULATESPEED <= getTicks()) {
		double iTime = double(getTicks()) - double(_TimeJobs.LastSpeedCalculation);
		_TimeJobs.LastSpeedCalculation = getTicks();


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
	if (_TimeJobs.LastPositionCheck + FC_INTERVAL_CHECKPOSITION <= getTicks()) {
		_TimeJobs.LastPositionCheck = getTicks();
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


		if (getTicks()-_TimeJobs.LastBlockPlace < 100) { //Client sends placeblock packet two times if you are aiming at a block
			return;
		}
		_TimeJobs.LastBlockPlace = getTicks();

		ItemSlot & InHand = _Inventory.getSelectedSlot();
		char iBlock = 0;
		if (InHand.isEmpty()) {
			InHand.clear();
			return;
		}

		if (ItemInfoStorage::isBlock(InHand.getItemID())) {
			iBlock = (char)InHand.getItemID();
		}

		if (blockCoordinates.X==-1 && blockCoordinates.Y == -1 && blockCoordinates.Z == -1 && Direction == -1) { //Special action

			//Special actions are not allowed with blocks
			if (ItemInfoStorage::isBlock(_Inventory.getSelectedSlot().getItemID())) { 
				sendEmptyBlock(blockCoordinates);
				return;
			}

			//Eating
			if (ItemInfoStorage::getItem(_Inventory.getSelectedSlot().getItemID()).Eatable) {
				if (_Flags.isRightClicking()) {
					if (_iFood==20) {return;}

					NetworkOut Out(&_NetworkOutRoot);

					Out.addByte(0x26);
					Out.addInt(_iEntityID);
					Out.addByte(9);
					Out.Finalize(FC_QUEUE_HIGH);

					_Flags.setRightClicking(true);
					syncFlagsWithPP();
					_TimeJobs.StartedEating = getTicks();
				}
				return;
			}

			//Weapon blocking
			if (ItemInfoStorage::getItem(_Inventory.getSelectedSlot().getItemID()).Weapon) {
				_Flags.setRightClicking(true);
				syncFlagsWithPP();
			}
			return; 
		}else{ //Client wants to place a block
			if (! ItemInfoStorage::isBlock(_Inventory.getSelectedSlot().getItemID())) { //But it's a item...
				ItemEntry Entry = ItemInfoStorage::getItem(InHand.getItemID());
				if (Entry.ConnectedBlock == 0) { 
					return; 
				}
				iBlock = Entry.ConnectedBlock;
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

		if(ItemInfoStorage::isSolid(InHand.getItemID())) {
			//Prevent: set a block into your body
			if ((ClientCoordinats.X == blockCoordinates.X && ClientCoordinats.Z == blockCoordinates.Z)  &&
				(ClientCoordinats.Y == blockCoordinates.Y || ClientCoordinats.Y+1==blockCoordinates.Y)) { 
					return;
			}

			//Prevent: set a block into other body
			if (_pPoolMaster->willHurtOther(blockCoordinates,this)) { 
				sendEmptyBlock(blockCoordinates);
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
			sendEmptyBlock(blockCoordinates);
			return;
		}

		//Prevent: set a block under the map
		if(blockCoordinates.Y < 0) {
			return;
		}

		//Prevent: set blocks in air (without a block in near)
		if (_pWorld->isSurroundedByAir(blockCoordinates)) {
			Disconnect("You are not able to set blocks into air!");
			return;
		}

		//Prevent: set too distant blocks
		if (MathHelper::distance3D(blockCoordinates,ClientCoordinats) > 6.0) {
			Disconnect("Target block too distant!");
			return;
		}


		//Prevent: set non solid blocks ontop of non solid blocks
		if (blockCoordinates.Y>0 && !ItemInfoStorage::isSolid(iBlock)) {
			BlockCoordinates temp = blockCoordinates;
			temp.Y--;
			char iBlockBelow = _pWorld->getBlock(temp);

			switch(ItemInfoStorage::getBlock(iBlock).Stackable) {
			case true:
				if (!ItemInfoStorage::getBlock(iBlockBelow).Stackable) { //Disallow placing fern or bushes or sugarcane on redstone
					sendEmptyBlock(blockCoordinates);
					return;
				}
				break;
			case false:
				if( (iBlockBelow==iBlock) || //Disallow redstone stacking
					(!ItemInfoStorage::getBlock(iBlockBelow).Solid)//Redstone needs a solid ground
					) 
				{
					sendEmptyBlock(blockCoordinates);
					return;
				}
				break;
			}
		}


		/*
		* All test done. 
		*/
		//Send block acception
		NetworkOut Out(&_NetworkOutRoot);
		Out.addByte(0x35);
		Out.addInt(blockCoordinates.X);
		Out.addByte(blockCoordinates.Y);
		Out.addInt(blockCoordinates.Z);
		Out.addByte(iBlock);
		Out.addByte(0);	
		Out.Finalize(FC_QUEUE_HIGH);


		//Send inventory update to player
		_Inventory.DecreaseInHandStack();


		//Append to map
		_pWorld->setBlock(blockCoordinates.X,blockCoordinates.Y,blockCoordinates.Z,iBlock);

		//Event to player pool
		ItemID Item;
		Item.first = (short)iBlock;
		Item.second=0;
		PlayerEventBase* p = new PlayerSetBlockEvent(this,blockCoordinates,Item);
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

void PlayerThread::sendEmptyBlock(BlockCoordinates coords) {
	NetworkOut Out(&_NetworkOutRoot);
	Out.addByte(0x35);
	Out.addInt(coords.X);
	Out.addByte((char)coords.Y);
	Out.addInt(coords.Z);
	Out.addByte(0);
	Out.addByte(0);		
	Out.Finalize(FC_QUEUE_HIGH);
}

void PlayerThread::syncFlagsWithPP() {
	PlayerEventBase* p = new PlayerUpdateFlagsEvent(this,_Flags);
	_pPoolMaster->addEvent(p);
}

void PlayerThread::Packet14_Digging() {
	try {







	} catch(Poco::RuntimeException& ex ) {
		cout<<"Exception cateched: "<<ex.message()<<"\n";
		Disconnect(FC_LEAVE_OTHER);
	}
}

string PlayerThread::getWorldWhoIn() {
	return _WorldWhoIn;
}

void PlayerThread::shutdown() {
	_fRunning=false;
	while(!_fRunning){ //Wait till _fRunning turns true
	}
	_fRunning=false;
}