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
#include "Entity.h"
#include "MinecraftServer.h"
#include "World.h"
#include "NetworkOut.h"
#include "Constants.h"

Entity::Entity(int iEID,MinecraftServer* pServer,World* pWorld) {
	if (iEID < 0) {throw Poco::RuntimeException("Invalid EID");}
	if (pServer == NULL || _pWorld == NULL) {throw Poco::RuntimeException("Nullpointer are not allowed");}
	
	_iEntityID = iEID;
	_iCreation = pServer->getInGameTime();
	_pWorld = pWorld;
	_pMCServer = pServer;
}

Entity::~Entity(){
}

int Entity::getEntityID() {
	return _iEntityID;
}

World* Entity::getWorld() {
	return _pWorld;
}

Tick Entity::getLiveTime() {
	return _pMCServer->getInGameTime() - _iCreation;
}


bool Entity::isAlive() {
	return false;
}

float Entity::getHeight() {
	return 0.0F;
}

float Entity::getWidth() {
	return 0.0F;
}

string Entity::getName() {
	return string("");
}

void Entity::spawn(NetworkOut& rOut) {
	rOut.addByte(0x1E);
	rOut.addInt(_iEntityID);
	rOut.Finalize(FC_QUEUE_HIGH);
}


double Entity::getX() {
	return Coordinates.X;
}

double Entity::getY() {
	return Coordinates.Y;
}

double Entity::getZ() {
	return Coordinates.Z;
}

float Entity::getYaw() {
	return Coordinates.Yaw;
}

float Entity::getPitch() {
	return Coordinates.Pitch;
}

float Entity::getHeadYaw() {
	return Coordinates.HeadYaw;
}

EntityCoordinates Entity::getCoordinates() {
	return Coordinates;
}

void Entity::setX(double n) {
	lastCoordinates.X = Coordinates.X;
	Coordinates.X = n;
}

void Entity::setY(double n) {
	lastCoordinates.Y = Coordinates.Y;
	Coordinates.Y = n;
}

void Entity::setZ(double n) {
	lastCoordinates.Z = Coordinates.Z;
	Coordinates.Z = n;
}

void Entity::setYaw(float n) {
	lastCoordinates.Yaw = Coordinates.Yaw;
	Coordinates.Yaw = n;
}

void Entity::setPitch(float n) {
	lastCoordinates.Pitch = Coordinates.Pitch;
	Coordinates.Pitch = n;
}

void Entity::setHeadYaw(float n) {
	lastCoordinates.HeadYaw = Coordinates.HeadYaw;
	Coordinates.HeadYaw = n;
}

void Entity::setCoordinates(EntityCoordinates& c) {
	lastCoordinates = Coordinates;
	Coordinates = c;
}

EntityCoordinates Entity::getLastCoordinates() {
	return lastCoordinates;
}

void Entity::syncCoordinates(NetworkOut& rOut) {
	double dX =  Coordinates.X - lastCoordinates.X;
	double dY =  Coordinates.Y - lastCoordinates.Y;
	double dZ =  Coordinates.Z - lastCoordinates.Z;

	/*if (lastCoordinates.HeadYaw != Coordinates.HeadYaw) {
		rOut.addByte(0x23);//Head move
		rOut.addInt(_iEntityID);
		rOut.addInt((char) ((Coordinates.Pitch * 256.0F) / 360.0F));
		rOut.Finalize(FC_QUEUE_HIGH);
	}*/

	if(lastCoordinates.LookEqual(Coordinates)) {	//Player just moved around and doesn't change camera 
		if (fabs(dX) <= 4.0 && fabs(dY) <= 4.0 && fabs(dZ) <= 4.0 ) {//Movement under 4 blocks
			rOut.addByte(0x1F);//relative move
			rOut.addInt(_iEntityID);
			rOut.addByte(   (char) (dX*32.0) );
			rOut.addByte(   (char) (dY*32.0) );
			rOut.addByte(   (char) (dZ*32.0) );
			rOut.Finalize(FC_QUEUE_HIGH);
			return;
		}
		/* else: full update */
	}else{ //player moved camera
		if (lastCoordinates.CoordinatesEqual(Coordinates)) { //Just moved camera
			rOut.addByte(0x20); //Entity Look
			rOut.addInt(_iEntityID);
			rOut.addByte( (char) ((Coordinates.Yaw * 256.0F) / 360.0F) );
			rOut.addByte( (char) ((Coordinates.Pitch * 256.0F) / 360.0F) );
			rOut.Finalize(FC_QUEUE_HIGH);
			return;
		}
		if (fabs(dX) <= 4.0 && fabs(dY) <= 4.0 && fabs(dZ) <= 4.0 ) {//Movement under 4 blocks
			rOut.addByte(0x21);//relative move + camera
			rOut.addInt(_iEntityID);
			rOut.addByte((char) (dX*32.0) );
			rOut.addByte((char) (dY*32.0) );
			rOut.addByte((char) (dZ*32.0) );
			rOut.addByte((char) ((Coordinates.Yaw * 256.0F) / 360.0F) );
			rOut.addByte((char) ((Coordinates.Pitch * 256.0F) / 360.0F) );
			rOut.Finalize(FC_QUEUE_HIGH);
			return;
		}
		/* else: full update */
	}

	//Full update
	rOut.addByte(0x22);
	rOut.addInt(_iEntityID);

	rOut.addInt( (int) (Coordinates.X * 32.0));
	rOut.addInt( (int) (Coordinates.Y * 32.0));
	rOut.addInt( (int) (Coordinates.Z * 32.0));

	rOut.addByte( (char) ((Coordinates.Yaw * 256.0F) / 360.0F) );
	rOut.addByte( (char) ((Coordinates.Pitch * 256.0F) / 360.0F) );
	//rOut.addByte( (char) ((Coordinates.HeadYaw * 256.0F) / 360.0F) );

	rOut.Finalize(FC_QUEUE_HIGH);
}