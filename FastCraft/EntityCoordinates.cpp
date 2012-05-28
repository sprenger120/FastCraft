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
#include "EntityCoordinates.h"
#include <cmath>
#include "MathHelper.h"
#include<iostream>

EntityCoordinates::EntityCoordinates() {
	X = Y = Z = Stance = 0.0;
	Yaw = Pitch = 0.0F;
	OnGround = false;
}

EntityCoordinates::~EntityCoordinates() { 
}


bool EntityCoordinates::operator == (const EntityCoordinates& other) {
	return MathHelper::distance3D(*this,other) <= 0.00390625 && 
		   fabs(Yaw - other.Yaw) <= 0.00390625 && 
		   fabs(Pitch - other.Pitch) <= 0.00390625;
}

bool EntityCoordinates::CoordinatesEqual(const EntityCoordinates& other) {
	return MathHelper::distance3D(*this,other) <= 0.0390625;
}

bool EntityCoordinates::LookEqual(const EntityCoordinates& other) {
	return fabs(Yaw - other.Yaw) <= 0.00390625 && 
			fabs(Pitch - other.Pitch) <= 0.00390625;
}