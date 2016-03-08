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
#include "PlayerPool.h"
#include "Constants.h"
#include "World.h"
#include "ItemInformationProvider.h"
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
#include "MinecraftServer.h"
#include "FCRuntimeException.h"
#include "EntityListEntry.h"
#include <rsa.h>
#include <cryptlib.h>
#include <Poco\ByteOrder.h>
#include <Poco\SHA1Engine.h>

using Poco::Thread;
using std::cout;
using std::endl;
using std::dec;

PlayerThread::PlayerThread(MinecraftServer* pServer) : 
	_fSpawned(false),
	_fHandshakeReceived(false),
	_fAssigned(false),
	_fKeysExchanged(false),
	_fCanSpawnChunks(false),
	ServerThreadBase("PlayerThread"),
	_NetworkOutRoot(pServer,this),
	_NetworkInRoot(_Connection,pServer,this),
	_NetworkWriter(_NetworkOutRoot.getLowQueue(),_NetworkOutRoot.getHighQueue(),_Connection,this),
	_Web_Session("session.minecraft.net"),
	_ChunkProvider(_NetworkOutRoot,this,pServer),
	_Inventory(this,pServer,_NetworkInRoot),
	_timeJobServer(this)
{
	_iEntityID = FC_UNKNOWNEID;

	_iHealth = 0;
	_iFood = 0;
	_nSaturation = 0.0F;

	_Spawned_PlayerInfoList = 0;
	_dRunnedMeters = 0.0;

	_iPlayerPing = -1;

	_pActualWorld = NULL;
	_pMinecraftServer = pServer;

	_timeJobServer.addJob(&PlayerThread::Interval_KeepAlive,15000);
	_timeJobServer.addJob(&PlayerThread::Interval_CheckEating,100);


	_sSecretKey.first = NULL;
	_iViewDistance = 0;
	_iChatFlags = 0;
	_iDifficulty = 0;

	startThread(this);
}


PlayerThread::~PlayerThread() {
	if (isAssigned()) {Disconnect("Server shutdown",false);}
	killThread();
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

void PlayerThread::run() {
	unsigned char iPacket;
	std::queue<unsigned char> qLastPackages;

	_iThreadStatus = FC_THREADSTATUS_RUNNING;

	while (_iThreadStatus == FC_THREADSTATUS_RUNNING) {
		if (!isAssigned()) {
			if (!qLastPackages.empty()) {while(!qLastPackages.empty()) {qLastPackages.pop();}}
			Thread::sleep(100);
			continue;
		} 


		try {
			_timeJobServer.doJobs();

			iPacket = _NetworkInRoot.readByte();
			/*cout<<std::hex<<(int(iPacket)&0xff)<<"\n";*/

			qLastPackages.push(iPacket);
			if (qLastPackages.size() > 10) {qLastPackages.pop();}


			if (!_fSpawned) { //Avoid accepting packets that require a successful login when not logged in completely
				switch(iPacket) {
				case 0x0:
				case 0xFE:
				case 0x2:
				case 0xFC:
				case 0xCD:
					break;
				default:
					Disconnect("Illegal login order");
					break;
				}
			}

			switch (iPacket) {
			case 0x0:
				Packet0_KeepAlive();
				break;
			case 0x2:
				if (!_fHandshakeReceived && !_fSpawned && !_fKeysExchanged/* && !_fClientReadyToLogin*/) {
					Packet2_Handshake();
				}else{
					Disconnect("Illegal login order");
				}
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
			case 0xCA: 
				Packet202_PlayerAbilities();
				break;
			case 0xCC:
				Packet204_LocalesAndViewDistance();
				break;
			case 0xCD: 
				if (_fHandshakeReceived && _fSpawned && _fKeysExchanged /*&& _fClientReadyToLogin*/) {
					Packet205_ClientStatus();
				}else{
					if (_fHandshakeReceived && !_fSpawned && _fKeysExchanged /*&& !_fClientReadyToLogin*/) {
						Packet205_ClientStatus();
						continue;
					}
					Disconnect("Illegal login order");
				}	
				break;
			case 0xFC:
				if (_fHandshakeReceived && !_fSpawned && !_fKeysExchanged/* && !_fClientReadyToLogin*/) {
					Packet252_EncryptionKeyResponse();
				}else{
					Disconnect("Illegal login order");
				}
				break;
			case 0xFE: 
				Packet254_ServerListPing();
				break;
			case 0xFF:
				Packet255_Disconnect();
				break;
			default: 
				Disconnect("Unknown packet!");
				cout<<"Unknown packet received! 0x"<<std::hex<<int(iPacket)<<endl;

				if (!qLastPackages.empty()) {
					cout<<"Last packets:";
					while(!qLastPackages.empty()) {
						cout<<"0x"<<std::hex<<(int(qLastPackages.front())&0xff);
						if (qLastPackages.size() != 1) {cout<<",";}
						qLastPackages.pop();
					}
					cout<<"\n";
				}
				break;
			}
		} catch (FCRuntimeException) {
			Disconnect(FC_LEAVE_OTHER);
		}
	}
	if(_fAssigned){Disconnect(FC_LEAVE_OTHER);}
	_iThreadStatus = FC_THREADSTATUS_DEAD;
}

void PlayerThread::Disconnect(string sReason,bool fShow) {
	try {
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

			_NetworkWriter.ProcessHighQueue();
			Disconnect(FC_LEAVE_KICK);

			if (fShow) { //Write kick message
				PlayerEventBase* p = new PlayerChatEvent(this,"§6" + strStrm.str(),_Coordinates);
				_pMinecraftServer->getPlayerPool()->addEvent(p);
			}
		}else{
			NetworkOut Out(&_NetworkOutRoot);
			Out.addByte(0xFF);
			Out.addString(sReason);
			/*cout<<Out.getStr().length()<<"\n";*/

			Out.Finalize(FC_QUEUE_HIGH);
			_NetworkWriter.ProcessHighQueue();

			Disconnect(FC_LEAVE_QUIT);
		}
	}catch(FCRuntimeException) {
		Disconnect(FC_LEAVE_OTHER);
	}
}

void PlayerThread::Disconnect(char iLeaveMode) {
	if (!_fAssigned) { return; }
	if (isSpawned() || iLeaveMode == FC_LEAVE_KICK) {
		PlayerEventBase* p = new PlayerDisconnectEvent(this,_iEntityID,_sName);

		switch (iLeaveMode) {
		case FC_LEAVE_QUIT:		
			cout<<_sName<<" quit ("<<_sLeaveMessage<<")"<<"\n";
			break;
		case FC_LEAVE_OTHER:
			cout<<_sName<<" quit (unknown reason)"<<"\n";
			break;
		}

		_pMinecraftServer->getPlayerPool()->addEvent(p);
	}

	_sName.clear();
	_sLeaveMessage.clear();

	_iEntityID	= FC_UNKNOWNEID;
	_iHealth	= -1;
	_iFood		= -1;
	_nSaturation= 0.0F;

	_Flags.clear();

	_Spawned_PlayerInfoList = 0;
	_dRunnedMeters = 0.0;


	_sIP.clear();
	_Connection.close();
	_iPlayerPing = -1;
	_iViewDistance = 0;
	_iChatFlags = 0;
	_iDifficulty = 0;

	_NetworkWriter.clearQueues();
	_heapSpawnedEntities.cleanupElements();
	_ChunkProvider.HandleDisconnect();
	_Inventory.onDisconnect();

	_timer_Ping.reset();
	_timer_LastBlockPut.reset();
	/*_timer_lastSpeedCalculation.reset();*/
	_timer_lastArmSwing.reset();
	/*_timer_lastPositionUpdateEvent.reset();*/
	_timer_StartedEating.reset();

	if (_sSecretKey.first != NULL) {
		delete [] _sSecretKey.first;
		_sSecretKey.first = NULL;
	}

	_NetworkInRoot.setCryptMode(false);
	_NetworkWriter.setCryptMode(false);

	_fSpawned			= false;
	_fHandshakeReceived = false;
	_fKeysExchanged		= false;
	/*_fClientReadyToLogin= false;*/
	_fCanSpawnChunks    = false;
	_fAssigned			= false;
}


void PlayerThread::Connect(Poco::Net::StreamSocket& Sock) {
	if (_fAssigned) {
		cout<<"***INTERNAL SERVER WARNING: PlayerPool tryed to assign an already assigned player thread"<<"\n";
		Disconnect(FC_LEAVE_OTHER);
	}
	_Connection = Sock; 
	_Connection.setLinger(true,5);
	_Connection.setNoDelay(true);
	_Connection.setBlocking(true);
	_Connection.setReceiveTimeout(Poco::Timespan(0,0,0,10,0));
	_Connection.setSendTimeout(Poco::Timespan(0,0,0,10,0));

	_Inventory.clear();
	_sIP.assign(_Connection.peerAddress().toString());

	_fAssigned = true;
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

void PlayerThread::sendTime() {
	try{
		NetworkOut Out(&_NetworkOutRoot);
		Out.addByte(0x4);
		Out.addInt64(_pMinecraftServer->getTimestamp());
		//Out.addInt64(_pMinecraftServer->getTimestamp()%24000L);
		Out.Finalize(FC_QUEUE_HIGH);
	}catch(FCRuntimeException) {
		Disconnect(FC_LEAVE_OTHER);
	}
}


void PlayerThread::sendClientPosition() {
	try{
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
	}catch(FCRuntimeException) {
		Disconnect(FC_LEAVE_OTHER);
	}
}

void PlayerThread::Interval_KeepAlive() {
	try {
		if (!isSpawned()){return;}
		sendKeepAlive();
	}catch(FCRuntimeException) {
		Disconnect(FC_LEAVE_OTHER);
	}
}

void PlayerThread::syncHealth() {
	try{
		NetworkOut Out(&_NetworkOutRoot);
		Out.addByte(0x8);
		Out.addShort(fixRange<short>(_iHealth,0,20));
		Out.addShort(fixRange<short>(_iFood,0,20));
		Out.addFloat(fixRange<float>(_nSaturation,0.0F,5.0F));
		Out.Finalize(FC_QUEUE_HIGH);
	}catch(FCRuntimeException) {
		Disconnect(FC_LEAVE_OTHER);
	}
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
	try{
		NetworkOut Out(&_NetworkOutRoot);
		Out.addByte(0x3);
		Out.addString(rString);
		Out.Finalize(FC_QUEUE_HIGH);
	}catch(FCRuntimeException) {
		Disconnect(FC_LEAVE_OTHER);
	}
}

void PlayerThread::Packet0_KeepAlive() {
	try{
		_NetworkInRoot.readInt(); 
	}catch(FCRuntimeException) {
		Disconnect(FC_LEAVE_OTHER);
	}
}

void PlayerThread::Packet2_Handshake() {
	try {
		int iProtocolVersion; 

		/* Read from network */
		iProtocolVersion = _NetworkInRoot.readByte();
		_sName = _NetworkInRoot.readString();
		_NetworkInRoot.readString();
		_NetworkInRoot.readInt();


		/* Check data */
		if (iProtocolVersion > FC_SUPPORTED_PROTOCOLVERSION) {
			Disconnect("Update your client to " + string(FC_SUPPORTED_MINCRAFTVERSION));
			return;
		}

		if (iProtocolVersion < FC_SUPPORTED_PROTOCOLVERSION) {
			Disconnect("Downgrade your client to " + string(FC_SUPPORTED_MINCRAFTVERSION));
			return;
		}

		if (_sName.length() > 16) {
			Disconnect("Username too long");
			return;
		}

		/* Response */
		NetworkOut Out(&_NetworkOutRoot);
		Out.addByte(0xFD);

		if (_pMinecraftServer->isOnlineModeActivated()) {
			Out.addString(generateConnectionHash());
		}else{
			Out.addString(string("-"));
		}

		Poco::Random rnd;
		rnd.seed();
		_iVerifyToken = rnd.next();

		Out.addByteArray(_pMinecraftServer->getCertificate());
		Out.addByteArray(std::make_pair((char*)&_iVerifyToken,(short)4));

		Out.Finalize(FC_QUEUE_HIGH);

		_fHandshakeReceived = true;
		_NetworkWriter.ProcessHighQueue();
	} catch(FCRuntimeException& ex) {
		cout<<"Exception catched "<<ex.what()<<"\n";
		Disconnect(FC_LEAVE_OTHER);
	}
}

void PlayerThread::Packet3_Chat() {
	string Message("");

	try {
		Message = _NetworkInRoot.readString();
		if (Message.length() > 100) {
			Disconnect("Received string too long");
			return;
		}
	} catch(FCRuntimeException) {
		Disconnect(FC_LEAVE_OTHER);
	}

	if (Message.compare("/stop") == 0) {
		_pMinecraftServer->shutdown();
		return;
	}

	_sTemp.assign("<§c");
	_sTemp.append(_sName);
	_sTemp.append("§f> ");
	_sTemp.append(Message); 

	ChatToAll(_sTemp);
}

void PlayerThread::Packet10_Player() {
	try {
		_Coordinates.OnGround = _NetworkInRoot.readBool();

		if (isSpawned()) {updatePing();}
	} catch(FCRuntimeException) {
		Disconnect(FC_LEAVE_OTHER);
	}
}

void PlayerThread::Packet11_Position() {
	EntityCoordinates TmpCoord = _Coordinates;

	try {
		//Read coordinates in a temporary variable
		TmpCoord.X = _NetworkInRoot.readDouble();
		TmpCoord.Y = _NetworkInRoot.readDouble();
		TmpCoord.Stance = _NetworkInRoot.readDouble();
		TmpCoord.Z = _NetworkInRoot.readDouble();
		TmpCoord.OnGround = _NetworkInRoot.readBool();

		if (!MathHelper::isValid(TmpCoord)) {
			cout<<_sName<<" tried to crash the server with illegal coordiantes\n";
			Disconnect("Illegal Coordinates!");
			return;
		}

		_dRunnedMeters += MathHelper::distance2D(_Coordinates,TmpCoord);
		_lastCoordinates = _Coordinates;
		_Coordinates = TmpCoord;

		if (isSpawned()) {
			CheckPosition();
			updatePing();
			syncMovement();
		}
	} catch(FCRuntimeException) {
		Disconnect(FC_LEAVE_OTHER);
	}
}

void PlayerThread::Packet12_Look() {
	try {
		EntityCoordinates TmpCoord = _Coordinates;
		TmpCoord.Yaw = _NetworkInRoot.readFloat();
		TmpCoord.Pitch = _NetworkInRoot.readFloat();
		TmpCoord.OnGround = _NetworkInRoot.readBool();

		if (!MathHelper::isValid(TmpCoord)) {
			cout<<_sName<<" tried to crash the server with illegal coordiantes\n";
			Disconnect("Illegal Coordinates!");
			return;
		}

		_Coordinates = TmpCoord;

		if (isSpawned()) {
			updatePing();
			syncMovement();
		}
	} catch(FCRuntimeException) {
		Disconnect(FC_LEAVE_OTHER);
	}
}

void PlayerThread::Packet13_PosAndLook() {
	EntityCoordinates TmpCoord;

	try {
		TmpCoord.X = _NetworkInRoot.readDouble();
		TmpCoord.Y = _NetworkInRoot.readDouble();
		TmpCoord.Stance = _NetworkInRoot.readDouble();
		TmpCoord.Z = _NetworkInRoot.readDouble();
		TmpCoord.Yaw = _NetworkInRoot.readFloat();
		TmpCoord.Pitch = _NetworkInRoot.readFloat();
		TmpCoord.OnGround = _NetworkInRoot.readBool();

		if (!MathHelper::isValid(TmpCoord)) {
			cout<<_sName<<" tried to crash the server with illegal coordiantes\n";
			Disconnect("Illegal Coordinates!");
			return;
		}

		_dRunnedMeters += MathHelper::distance2D(_Coordinates,TmpCoord);
		_lastCoordinates = _Coordinates;
		_Coordinates = TmpCoord;

		if (isSpawned()) {
			CheckPosition();
			updatePing();
			syncMovement();
		}
	} catch(FCRuntimeException) {
		Disconnect(FC_LEAVE_OTHER);
	}
}

void PlayerThread::Packet254_ServerListPing() {
	_sTemp.clear();
	_sTemp.assign(_pMinecraftServer->getServerDescription()); //Server Description
	_sTemp.append("§");
	Poco::NumberFormatter::append(_sTemp, _pMinecraftServer->getPlayerPool()->getConnectedPlayerCount());//!++ Playercount
	_sTemp.append("§");
	Poco::NumberFormatter::append(_sTemp,_pMinecraftServer->getPlayerSlotCount()); //player slots

	Disconnect(_sTemp);
}

void PlayerThread::Packet255_Disconnect() {
	try {
		_sLeaveMessage = _NetworkInRoot.readString();
	} catch(FCRuntimeException) {
		Disconnect(FC_LEAVE_OTHER);
	}
	Disconnect(FC_LEAVE_QUIT);
}

void PlayerThread::ChatToAll(string& rString) {
	PlayerEventBase* p = new PlayerChatEvent(this,rString,_Coordinates);
	_pMinecraftServer->getPlayerPool()->addEvent(p);
}

void PlayerThread::PlayerInfoList(bool fSpawn,string Name) {
	if (fSpawn) {
		if (_Spawned_PlayerInfoList == 60) {return;} //Workaround for a Minecraft render bug
		_Spawned_PlayerInfoList++;
	}else{
		_Spawned_PlayerInfoList--;
	}

	try {
		NetworkOut Out(&_NetworkOutRoot);
		Out.addByte(0xc9);
		Out.addString(Name);
		Out.addBool(fSpawn);
		Out.addShort(10);
		Out.Finalize(FC_QUEUE_HIGH);
	}catch(FCRuntimeException) {
		Disconnect(FC_LEAVE_OTHER);
	}
}

void PlayerThread::Packet16_HoldingChange() {
	try{
		ItemEntry* pOldItem = _Inventory.getSelectedSlot()->getItemEntryCache();
		_Inventory.readHeldItemChange();

		_pMinecraftServer->getPlayerPool()->addEvent(new PlayerChangeHeldEvent(this));


		if (_Flags.isRightClicking()){ 
			//ItemEntry* pItem = _Inventory.getSelectedSlot()->getItemEntryCache();

			_Flags.setRightClicking(false); //Kill right klick flag

			if (pOldItem != NULL && pOldItem->Eatable) { //Switched from a eatable thing to ... 
				_timer_StartedEating.reset();/* Stop eating timers */
			}
		}
	} catch(FCRuntimeException) {
		Disconnect(FC_LEAVE_OTHER);
	}
}

void PlayerThread::Packet101_CloseWindow() {
	try {	
		_NetworkInRoot.readByte();
		cout<<"PlayerThread::Packet101_CloseWindow() is not implemented\n";
	} catch(FCRuntimeException) {
		Disconnect(FC_LEAVE_OTHER);
	}
}

void PlayerThread::Packet102_WindowClick() {
	try{
		_Inventory.readClickWindow(this,_NetworkInRoot);

#ifdef _DEBUG
		NetworkOut Out(&_NetworkOutRoot);
		_Inventory.syncInventory(Out);
#endif

		_pMinecraftServer->getPlayerPool()->addEvent(new PlayerChangeHeldEvent(this));
	}catch(FCRuntimeException) {
		Disconnect(FC_LEAVE_OTHER);
	}
}

void PlayerThread::syncMovement() {
	if (!isSpawned()) {return;}

	if (!(_Coordinates == _lastCoordinates)) {
		cleanupSpawnedPlayers();
		_pMinecraftServer->getPlayerPool()->addEvent(new PlayerMoveEvent(this,_Coordinates));
	}
}

void PlayerThread::spawnEntity(Entity* pEntity) {
	if (!isSpawned()) {throw FCRuntimeException("Not spawned!");}
	if (pEntity == NULL)  {throw FCRuntimeException("Nullpointer");}

	try {
		NetworkOut out(&_NetworkOutRoot);
		pEntity->spawn(out);

		_heapSpawnedEntities.add(pEntity->getEntityID(),new EntityListEntry(pEntity));
	}catch(FCRuntimeException& ex) {
		cout<<"Exception catched "<<ex.what()<<"\n";
		Disconnect(FC_LEAVE_OTHER);
	}
}

bool PlayerThread::isEntitySpawned(int ID) {
	return _heapSpawnedEntities.has(ID);
}

void PlayerThread::updateEntityPosition(Entity* pEntity) {
	if (!isSpawned()) {throw FCRuntimeException("Not spawned!");}
	if (pEntity == NULL)  {throw FCRuntimeException("Nullpointer");}
	try {

		EntityListEntry* ptr;
		if ((ptr = _heapSpawnedEntities.get(pEntity->getEntityID())) == NULL) {throw FCRuntimeException("Not spawned!");}

		NetworkOut Out(&_NetworkOutRoot);
		pEntity->syncCoordinates(Out,ptr->Position);

		ptr->Position = pEntity->Coordinates;
	}catch(FCRuntimeException) {
		Disconnect(FC_LEAVE_OTHER);
	}
}

void PlayerThread::despawnEntity(int ID) {
	if (!isSpawned()) {throw FCRuntimeException("Not spawned!");}
	if (!_heapSpawnedEntities.has(ID)) {throw FCRuntimeException("Entity not spawned");}
	try {

		_heapSpawnedEntities.erase(ID);

		NetworkOut Out(&_NetworkOutRoot);
		Out.addByte(0x1D);
		Out.addInt(ID);
		Out.Finalize(FC_QUEUE_HIGH);
	}catch(FCRuntimeException) {
		Disconnect(FC_LEAVE_OTHER);
	}
}

int PlayerThread::getEntityID() {
	return _iEntityID;
}

void PlayerThread::Packet18_Animation() {
	try {
		int iEID = _NetworkInRoot.readInt();
		char iAnimID = _NetworkInRoot.readByte();

		if (iEID != _iEntityID) { 
			cout<<_sName<<" tried to use EntityAction on a strange EID\n"; 
			return;
		}

		if (!Constants::isDefined(iAnimID,"/Entity/Alive/Animation/")) {
			cout<<_sName<<" tried to use an invalid animationID\n"; 
			return;
		}


		Poco::Timestamp::TimeDiff time = _timer_lastArmSwing.elapsed();

		if (time == 0) { /* First time-> start timer*/	 
			_timer_lastArmSwing.start();
		}else{
			if (time / 1000 >= FC_ARMSWINGCOOLDOWN) {
				_timer_lastArmSwing.stop();
				_timer_lastArmSwing.reset();
				_timer_lastArmSwing.start();
			}else{
				return;
			}
		}

		PlayerEventBase* p = new PlayerAnimationEvent(this,iAnimID);
		_pMinecraftServer->getPlayerPool()->addEvent(p);
	}catch(FCRuntimeException) {
		Disconnect(FC_LEAVE_OTHER);
	}
}

void PlayerThread::Packet19_EntityAction() {
	try {
		int iEID = _NetworkInRoot.readInt();
		char iActionID = _NetworkInRoot.readByte();

		if (iEID != _iEntityID){
			cout<<_sName<<" tried to use EntityAction on a strange EID\n";
			return;
		}

		switch(iActionID) {
		case FC_ENTITY_ACTION_CROUCH:
			_Flags.setCrouched(true);
			break;
		case FC_ENTITY_ACTION_UNCROUCH:
			_Flags.setCrouched(false);
			break;
		case FC_ENTITY_ACTION_STARTSPRINTING:
			if (_Flags.isRightClicking()) {return;} /* Not a vanilla behavior */
			_Flags.setSprinting(true);
			break;
		case FC_ENTITY_ACTION_STOPSPRINTING:
			_Flags.setSprinting(false);
			break;
		default:
			cout<<_sName<<" sent an invalid actionID\n";
			return;
		}

		syncFlagsWithPP();
	}catch(FCRuntimeException) {
		Disconnect(FC_LEAVE_OTHER);
	}
}

void PlayerThread::playAnimationOnEntity(int ID,char AnimID) {
	if (!isEntitySpawned(ID)) {throw FCRuntimeException("Not spawned!");}
	if (!Constants::isDefined(AnimID,"/Entity/Alive/Animation/")){throw FCRuntimeException("Animation is not defined");}
	try{

		NetworkOut Out(&_NetworkOutRoot);
		Out.addByte(0x12);
		Out.addInt(ID);
		Out.addByte(AnimID);
		Out.Finalize(FC_QUEUE_HIGH);
	}catch(FCRuntimeException) {
		Disconnect(FC_LEAVE_OTHER);
	}
}


void PlayerThread::updateEntityMetadata(EntityLiving* pEntity) {
	if (!isSpawned()) {throw FCRuntimeException("Not spawned!");}
	if (pEntity == NULL)  {throw FCRuntimeException("Nullpointer");}

	try {
		NetworkOut Out(&_NetworkOutRoot);
		pEntity->sendMetadata(Out);
	}catch(FCRuntimeException) {
		Disconnect(FC_LEAVE_OTHER);
	}
}

//void PlayerThread::Interval_CheckSpeed() {
//	if (!isSpawned()) {return;}
//	if (_timer_lastSpeedCalculation.elapsed() == 0) { /* Start timer the first time */
//		_timer_lastSpeedCalculation.start();
//		return;
//	}
//
//	double iTime  = double(_timer_lastSpeedCalculation.elapsed()) / 1000000.0; //microseconds -> seconds
//	double iSpeed = _dRunnedMeters / iTime;
//
//	cout<<_sName<<" v="<<iSpeed<<" s="<<_dRunnedMeters<<" t="<<iTime<<"\n";
//
//	if (iSpeed > 15.0) {
//		cout<<_sName<<" moved too fast ("<<int(iSpeed)<<" km/h)"<<"\n";
//		Disconnect("You moved too fast!");
//	}
//
//	_dRunnedMeters = 0.0;
//	_timer_lastSpeedCalculation.reset();
//}

InventoryPlayer& PlayerThread::getInventory() {
	return _Inventory;
}

EntityFlags PlayerThread::getFlags() {
	return _Flags;
}

void PlayerThread::updateEntityEquipment(EntityLiving* pEntity) {
	if (!isEntitySpawned(pEntity->getEntityID())) {throw FCRuntimeException("Not spawned!");}

	try {
		EntityListEntry* pEntityLEntry = _heapSpawnedEntities.get(pEntity->getEntityID());

		NetworkOut Out(&_NetworkOutRoot);
		pEntity->updateEquipment(Out,pEntityLEntry->Equipment);
	}catch(FCRuntimeException) {
		Disconnect(FC_LEAVE_OTHER);
	}
}

void PlayerThread::CheckPosition() {
	//if (_pActualWorld->isSuffocating(this)) {
	//	cout<<_pMinecraftServer->getInGameTime()<<"\tsuffocating Y:"<<_Coordinates.Y<<"\n";
	//	
	//	_Coordinates = _lastCoordinates;
	//	sendClientPosition();
	//}
}

void PlayerThread::Packet15_PlayerBlockPlacement() {
	try {
		char Direction;
		ItemSlot Slot(_pMinecraftServer->getItemInfoProvider());
		BlockCoordinates ClientCoordinats = ChunkMath::toBlockCoords(_Coordinates);
		BlockCoordinates blockCoordinates;

		blockCoordinates.X = _NetworkInRoot.readInt();
		blockCoordinates.Y = _NetworkInRoot.readByte();
		blockCoordinates.Z = _NetworkInRoot.readInt();
		Direction = _NetworkInRoot.readByte();
		Slot.readFromNetwork(_NetworkInRoot);

		/* Crosshair pos*/
		_NetworkInRoot.readByte(); 
		_NetworkInRoot.readByte();
		_NetworkInRoot.readByte();

		Poco::Timestamp::TimeDiff lastBlockPut = _timer_LastBlockPut.elapsed()/1000;

		/*
		* The client sends this packet two times if you are standing in front of a block very close and you cant
		* set it 
		*/
		if (lastBlockPut < 100 && lastBlockPut>0) {
			_timer_LastBlockPut.stop();
			_timer_LastBlockPut.reset();
			return;
		} 


		ItemSlot* InHand = _Inventory.getSelectedSlot();

		ItemID idSelectedBlock;
		if (InHand->isEmpty()) { /* No item in hand */
			InHand->clear();
			return;
		}

		if (InHand->isBlock()) {idSelectedBlock = InHand->getItem();}

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
		case -1:
			if (blockCoordinates.X==-1 && blockCoordinates.Y == -1 && blockCoordinates.Z == -1) {
				if (InHand->getItem().first == 326 || InHand->getItem().first == 327) {return;} //Filter lava and water buckets

				/* Special actions are not allowed with blocks */
				if (InHand->isBlock()) {
					spawnBlock(blockCoordinates,ItemID(0,0));
					return;
				}

				/* Eating */
				if (InHand->getItemEntryCache()->Eatable) {
					if (_iFood==20) {return;} /* You're sated */

					cout<<"start eating\n";

					_timer_StartedEating.stop();
					_timer_StartedEating.reset();
					_timer_StartedEating.start();


					switch (_Flags.isRightClicking()) {
					case true: //Already right klicking -> "restart" right klicking
						_Flags.setRightClicking(false);
						syncFlagsWithPP();
						Thread::sleep(200); // Wait 4 ticks
						_Flags.setRightClicking(true);
						syncFlagsWithPP();
						break;
					case false:
						_Flags.setRightClicking(true);
						syncFlagsWithPP();
					}
					return;
				}

				/* Weapon blocking */
				if (InHand->getItemEntryCache()->Weapon) {
					cout<<"start blocking\n";
					switch (_Flags.isRightClicking()) {
					case true: //Already right klicking -> "restart" right klicking
						_Flags.setRightClicking(false);
						syncFlagsWithPP();
						Thread::sleep(200); // Wait 4 ticks
						_Flags.setRightClicking(true);
						syncFlagsWithPP();
						break;
					case false:
						_Flags.setRightClicking(true);
						syncFlagsWithPP();
					}
				}
				return; 
			}
			break;
		default:
			cout<<_sName<<" sent an illegal block direction\n";
			return;
		}


		/* Items who are connected with blocks */
		if (!InHand->isBlock()) { 

			/* Buckets and fluids */
			if (InHand->getItem().first == 325) {
				ItemID Block = _pActualWorld->getBlock(blockCoordinates);

				if (Block.first == 9 || Block.first == 11) {
					_pActualWorld->setBlock(blockCoordinates,ItemID(0,0));

					switch(Block.first) {
					case 9: //Water
						InHand->setItem(ItemID(326,0));
						break;
					case 11: //Lava
						InHand->setItem( ItemID(327,0));
						break;
					}
				}
				return;
			}


			ItemEntry* Entry = InHand->getItemEntryCache();
			if (Entry->ConnectedBlock.first == -1 && Entry->ConnectedBlock.second == -1) { //No connected item
				spawnBlock(blockCoordinates,ItemID(0,0));
				_Inventory.syncInHandStack();
				return; 
			}
			idSelectedBlock = Entry->ConnectedBlock;
		}



		BlockEntry* pSelectedBlock = _pMinecraftServer->getItemInfoProvider()->getBlock(idSelectedBlock);

		/* Prevent: set a block above the map */
		if (pSelectedBlock->Fluid) { /* Fluids can be placed one block above the max block level */
			if (blockCoordinates.Y >= FC_WORLDHEIGHT-1) {return;}
		}else{
			if (blockCoordinates.Y >= FC_WORLDHEIGHT-2) {return;}
		}

		/* Prevent: set a block under the map */
		if(blockCoordinates.Y < 0) {return;}

		/* Prevent: set too distant blocks */
		if (MathHelper::distance3D(blockCoordinates,ClientCoordinats) > 6.0) {return;}


		ItemID BlockAt = _pActualWorld->getBlock(blockCoordinates);
		BlockEntry* pBlockAt = _pMinecraftServer->getItemInfoProvider()->getBlock(BlockAt);		


		/* Prevent: Set a block into a non fluid block/non air */
		if (BlockAt.first != 0 && !pBlockAt->Fluid && !pBlockAt->isSpreadBlock) {			
			spawnBlock(blockCoordinates,BlockAt);
			_Inventory.syncInHandStack();
			return;
		}

		/* Prevent: Set not placeable blocks */
		if (!pSelectedBlock->Placeable) {
			spawnBlock(blockCoordinates,ItemID(0,0));
			_Inventory.syncInHandStack();
			return;
		}


		/* Prevent: Set a solid block into you/other player */
		if(pSelectedBlock->Solid) {
			vector<PlayerThread*>& rvpPlayerThreads = _pMinecraftServer->getPlayerPool()->ListPlayers();
			BlockCoordinates coords;

			for (short x=0;x<=rvpPlayerThreads.size()-1;x++){
				if ( !rvpPlayerThreads[x]->isAssigned() || !rvpPlayerThreads[x]->isSpawned()) {continue;}
				coords.X = int(floor(rvpPlayerThreads[x]->getCoordinates().X));
				coords.Y = int(floor(rvpPlayerThreads[x]->getCoordinates().Y));
				coords.Z = int(floor(rvpPlayerThreads[x]->getCoordinates().Z));

				if (coords.X == blockCoordinates.X &&
					coords.Z == blockCoordinates.Z &&
					(coords.Y == blockCoordinates.Y ||coords.Y+1 == blockCoordinates.Y)
					)
				{ /* it will hurt someone */
					spawnBlock(blockCoordinates,ItemID(0,0));
					_Inventory.syncInHandStack();
					return;
				}
			}
		}


		/* Prevent: set non solid blocks ontop of other non solid blocks that aren't allow stacking of itself*/
		if (blockCoordinates.Y>=1 && !pSelectedBlock->Solid) { /* Not needed if you are setting a block with Y=0 / solid block */
			ItemID BlockBelow = _pActualWorld->getBlock(blockCoordinates.X,blockCoordinates.Y-1,blockCoordinates.Z);
			BlockEntry* pBlockBelow = _pMinecraftServer->getItemInfoProvider()->getBlock(BlockBelow);

			if (!pBlockBelow->Solid && /* Unsolid ground */
				pSelectedBlock->Stackable && /* You can place one of the same type above this block*/
				BlockBelow.first != idSelectedBlock.first /* Bottom block is not the same one than the to be set one*/
				)
			{
				spawnBlock(blockCoordinates,ItemID(0,0));
				_Inventory.syncInHandStack();
				return;
			}
		}


		/* All tests done! */
		/* Send inventory update to player */
		_Inventory.DecreaseInHandStack();


		/* Append to map */
		_pActualWorld->setBlock(blockCoordinates,idSelectedBlock);

		_timer_LastBlockPut.reset();
		_timer_LastBlockPut.start();
	} catch(FCRuntimeException) {
		Disconnect(FC_LEAVE_OTHER);
	}
}

int PlayerThread::getChunksInQueue() {
	return _NetworkOutRoot.getLowQueue().size();
}

void PlayerThread::spawnBlock(BlockCoordinates& blockCoords,ItemID& Item) {
	if (!_pMinecraftServer->getItemInfoProvider()->isRegistered(Item)) {
		throw FCRuntimeException("Block not registered");
	}
	try {
		ChunkCoordinates Coords = ChunkMath::toChunkCoords(blockCoords);

		NetworkOut Out(&_NetworkOutRoot);
		Out.addByte(0x35);
		Out.addInt(blockCoords.X);
		Out.addByte((char)blockCoords.Y);
		Out.addInt(blockCoords.Z);
		Out.addByte((unsigned char)Item.first);
		Out.addByte(Item.second);	
		Out.Finalize(FC_QUEUE_HIGH);
	}catch(FCRuntimeException) {
		Disconnect(FC_LEAVE_OTHER);
	}
}


void PlayerThread::syncFlagsWithPP() {
	PlayerEventBase* p = new PlayerUpdateFlagsEvent(this,_Flags);
	_pMinecraftServer->getPlayerPool()->addEvent(p);
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
			case 5: //Shot arrow / finish eating / stop blocking
				if (_Flags.isRightClicking()) {
					cout<<"end right-click\n";
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


		/*	cout<<"iStatus:"<<int(iStatus)<<"\tiFace:"<<int(iFace)<<"\n";*/
	} catch(FCRuntimeException ) {
		Disconnect(FC_LEAVE_OTHER);
	}
}

World* PlayerThread::getWorld() {
	return _pActualWorld;
}

void PlayerThread::setEntityStatus(char iCode) {
	try {
		NetworkOut Out(&_NetworkOutRoot);

		Out.addByte(0x26);
		Out.addInt(_iEntityID);
		Out.addByte(iCode);
		Out.Finalize(FC_QUEUE_HIGH);
	}catch(FCRuntimeException) {
		Disconnect(FC_LEAVE_OTHER);
	}
}

void PlayerThread::setEntityStatus(int iID,char iCode) {
	if (!isEntitySpawned(iID)) {
		cout<<"PlayerThread::setEntityStatus "<<iID<<" is not spawned!\n";
		return;
	}
	try{
		NetworkOut Out(&_NetworkOutRoot);

		Out.addByte(0x26);
		Out.addInt(iID);
		Out.addByte(iCode);
		Out.Finalize(FC_QUEUE_HIGH);
	}catch(FCRuntimeException) {
		Disconnect(FC_LEAVE_OTHER);
	}
}

void PlayerThread::Interval_CheckEating() {
	if (!isSpawned()) {return;}
	handleEating();
}


void PlayerThread::handleEating() {
	if (!_Flags.isRightClicking()) {return;}
	if (_timer_StartedEating.elapsed() == 0) {return;}

	ItemSlot* pInHandSlot = _Inventory.getSelectedSlot();
	if (pInHandSlot->isEmpty() || pInHandSlot->isBlock()) { //Block or empty
		_Flags.setRightClicking(false);
		syncFlagsWithPP();
		return;
	}

	ItemEntry* IEntry = pInHandSlot->getItemEntryCache();
	if (IEntry->Weapon) {return;}
	if (!IEntry->Eatable) {
		_Flags.setRightClicking(false);
		syncFlagsWithPP();
		return;
	}

	if (_timer_StartedEating.elapsed()/1000 >= 1200) {
		_iFood = fixRange<short>(_iFood+IEntry->FoodValue,0,20); 
		_nSaturation = fixRange<float>(_nSaturation + float(IEntry->FoodValue)*0.3F,0.0F,5.0F);

		setEntityStatus(Constants::get("/Entity/Alive/Status/AcceptEating")); //Accept eating
		syncHealth();
		_Inventory.DecreaseInHandStack();


		_Flags.setRightClicking(false);
		syncFlagsWithPP();
		_timer_StartedEating.stop();
		_timer_StartedEating.reset();
	}
}

void PlayerThread::sendKeepAlive() {
	try{
		NetworkOut Out(&_NetworkOutRoot);
		Out.addByte(0x0);
		Out.addInt(_Rand.next());
		Out.Finalize(FC_QUEUE_HIGH);
	}catch(FCRuntimeException) {
		Disconnect(FC_LEAVE_OTHER);
	}
}

void PlayerThread::updatePing() {
	_iPlayerPing = short(_timer_Ping.elapsed()/1000);
	_timer_Ping.reset();
	_timer_Ping.start();
}

short PlayerThread::getPing() {
	return _iPlayerPing;
}

MinecraftServer* PlayerThread::getMinecraftServer(){
	return _pMinecraftServer;
}

void PlayerThread::cleanupSpawnedPlayers() {
	auto it = _heapSpawnedEntities.begin();
	char iTypePlayer = Constants::get("/Entity/Alive/TypeID/Player");
	try {

		while (!it.isEndReached()) {
			if (!it->isAlive() || it->getType() != iTypePlayer) {continue;}
			if (MathHelper::distance2D(it->Position,_Coordinates) > FC_PLAYERSPAWNRADIUS) { /* too distant -> despawn */
				despawnEntity(it->getEntiyID());
			}
			it++;
		}
	}catch(FCRuntimeException) {
		Disconnect(FC_LEAVE_OTHER);
	}
}

NetworkOutRoot* PlayerThread::getNetworkOutRoot() {
	return &_NetworkOutRoot;
}

void PlayerThread::Packet252_EncryptionKeyResponse() {
	try {
		_sSecretKey = _NetworkInRoot.readByteArray(true);
		if (_sSecretKey.second != 16) {throw FCRuntimeException("Illegal key length");}

		pair<char*,short> VerifyToken = _NetworkInRoot.readByteArray(true);
		if (VerifyToken.second != 4) {throw FCRuntimeException("Illegal key length");}

		int iClientToken;
		memcpy(&iClientToken,VerifyToken.first,4);
		delete [] VerifyToken.first;

		if(_sSecretKey.second == 0) {throw FCRuntimeException("Illegal encryption key");}
		if (iClientToken != _iVerifyToken) {throw FCRuntimeException("Invalid token");}


		/* Check premium if needed */
		if (_pMinecraftServer->isOnlineModeActivated()) {
			//Compute connection hash for minecraft.net
			Poco::SHA1Engine sha1;
			bool fNegative = false;
			sha1.update(_sConnectionHash.c_str(),_sConnectionHash.length());
			sha1.update(_sSecretKey.first,_sSecretKey.second);
			sha1.update(_pMinecraftServer->getCertificate().first,_pMinecraftServer->getCertificate().second);


			vector<unsigned char> digests = sha1.digest();
			if (digests.size() != 20) {
				Disconnect("Unable to verify username (internal error)");
				return;
			}
			if (digests[0] & 0x80) {
				fNegative = true;
				char x;
				for (x=0;x<=19;x++) {
					digests[x] = ~(digests[x]);
				}

				bool fCarry = true;
				x = 19;

				while (fCarry) {
					fCarry = bool(digests[x] == 0xff);
					digests[x--]++;
				}
			}

			string serverID( fNegative ? "-" : "");
			serverID.append(sha1.digestToHex(digests));

			if (serverID.size() < 40) {
				Disconnect("Unable to verify username (internal error)");
				return;
			}

			//Remove leading zeros
			char iOffset = fNegative ? 1 : 0;
			if (serverID[iOffset] == '0') {
				for (int x = iOffset;x<=serverID.size()-1;x++) {
					if (serverID[x] != '0') {
						serverID.erase(iOffset,x-iOffset);
						break;
					}
				}
			}


			/* Send it */
			string sPath("/game/checkserver.jsp?user=");
			sPath.append(_sName);
			sPath.append("&serverId=");
			sPath.append(serverID);

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

		_fKeysExchanged = true;

		NetworkOut Out(&_NetworkOutRoot);
		Out.addByte(0xFC);
		Out.addShort(0);
		Out.addShort(0);
		Out.Finalize(FC_QUEUE_HIGH);
		_NetworkWriter.ProcessHighQueue();

		_NetworkInRoot.setCryptMode(true);
		//_NetworkOutRoot.setCryptMode(true);
		_NetworkWriter.setCryptMode(true);
	}catch(FCRuntimeException) {
		Disconnect(FC_LEAVE_OTHER);
	}
}

pair<char*,short>& PlayerThread::getSecretKey() {
	return _sSecretKey;
}

void PlayerThread::Packet205_ClientStatus() {
	try {
		char iStatus = _NetworkInRoot.readByte();

		if (!_fSpawned) {
			if (iStatus != 0) {
				Disconnect("Illegal login order");
				return;
			}
			/* Init stuff */
			_pActualWorld = _pMinecraftServer->getWorldByName(_pMinecraftServer->getMainWorldName());
			if(_pActualWorld == NULL) {
				Disconnect("Spawn world not found!");
				return;
			}

			PlayerThread* pPlayer = _pMinecraftServer->getPlayerPool()->getPlayerByName(_sName,this);
			if (pPlayer != NULL) {
				pPlayer->Disconnect("Logged in from another location.",false);
			}

			_iEntityID = _pMinecraftServer->generateID();


			/* Build login request */
			NetworkOut Out(&_NetworkOutRoot);
			Out.addByte(0x1);
			Out.addInt(_iEntityID);
			Out.addString(string("SUPERFLAT"));
			Out.addByte(_pMinecraftServer->getServerMode() ? 1 : 0);
			Out.addByte(_pActualWorld->getDimension());
			Out.addByte(_pMinecraftServer->getDifficulty());
			Out.addByte(0); //Unused
			Out.addByte((_pMinecraftServer->getPlayerSlotCount() > 255 ? 255 : (unsigned char)_pMinecraftServer->getPlayerSlotCount()));
			Out.Finalize(FC_QUEUE_HIGH);


			_Coordinates.X = (double)_pActualWorld->getSpawnX();
			_Coordinates.Y = 120.0; //(double)_pActualWorld->getSpawnY();
			_Coordinates.Z = (double)_pActualWorld->getSpawnZ();
			_Coordinates.Stance = 0.0;
			_Coordinates.OnGround = false;
			_Coordinates.Pitch = 0.0F;
			_Coordinates.Yaw = 0.0F;
			_lastCoordinates = _Coordinates;
			CheckPosition();


			/* sending compass */
			Out.addByte(0x6);
			Out.addInt(0); 
			Out.addInt(0); 
			Out.addInt(0);  
			Out.Finalize(FC_QUEUE_HIGH);

			/* sending time */
			sendTime();

			_ChunkProvider.HandleNewPlayer();

			/* sending/setting health */
			_iHealth	= 10;
			_iFood		= 0;
			_nSaturation= 0.0F;
			syncHealth();


			/* sending clients position */
			sendClientPosition();


			/* Push join event */
			_fSpawned = true;
			PlayerEventBase* p = new PlayerJoinEvent(this);//Push PlayerPool Join event
			_pMinecraftServer->getPlayerPool()->addEvent(p);


			/* send active player list */
			vector<PlayerThread*>& rvPlayers = _pMinecraftServer->getPlayerPool()->ListPlayers();
			if (!rvPlayers.empty()) {
				short iSpawnedPlayers=0;
				for (short x = 0;x<= rvPlayers.size()-1;x++) {
					if (!rvPlayers[x]->isSpawned()) {continue;}
					PlayerInfoList(true,rvPlayers[x]->getUsername());
					iSpawnedPlayers++;
					if(iSpawnedPlayers == 59) {break;}
				}
			}


			/* sending/setting inventory */
			_Inventory[37]->setItem(ItemID(276,0));
			_Inventory[37]->setStackSize(1);
			_Inventory[38]->setItem(ItemID(2,0));
			_Inventory[38]->setStackSize(5);
			_Inventory[39]->setItem(ItemID(5,0));
			_Inventory[39]->setStackSize(64);
			_Inventory.onLogin();


			/* Welcome message */
			insertChat(_pMinecraftServer->getServerMOTD());
			_timer_Ping.start();
			cout<<_sName<<" joined ("<<_sIP<<") EID:"<<_iEntityID<<endl; 
		}
	}catch(FCRuntimeException& ex) {
		cout<<"Exception catched: "<<ex.getMessage()<<"\n";
		Disconnect(FC_LEAVE_OTHER);
	}
}

void PlayerThread::Packet204_LocalesAndViewDistance() {
	try {
		_sPlayerLocale = _NetworkInRoot.readString();
		_iViewDistance = _NetworkInRoot.readByte();
		_iChatFlags	   = _NetworkInRoot.readByte();
		_iDifficulty   = _NetworkInRoot.readByte();

		if (_sPlayerLocale.empty()) {
			Disconnect("Illegal locale");
			return;
		}

		if (_iViewDistance < 0 || _iViewDistance > 3) {
			Disconnect("Illegal view distance");
			return;
		}
		_fCanSpawnChunks = true;
	}catch(FCRuntimeException& ex) {
		cout<<"Exception catched: "<<ex.getMessage()<<"\n";
		Disconnect(FC_LEAVE_OTHER);
	}
}

char PlayerThread::getViewDistance() {
	return _iViewDistance;
}

void PlayerThread::Packet202_PlayerAbilities() {
	try {
		_NetworkInRoot.readByte();
		_NetworkInRoot.readByte();
		_NetworkInRoot.readByte();

		//NetworkOut Out(&_NetworkOutRoot);
		//Out.addByte(0xCA);
		//Out.addByte(14);
		//Out.addByte(50);
		//Out.addByte(50);
		//Out.Finalize(FC_QUEUE_HIGH);
	}catch(FCRuntimeException& ex) {
		cout<<"Exception catched: "<<ex.getMessage()<<"\n";
		Disconnect(FC_LEAVE_OTHER);
	}
}