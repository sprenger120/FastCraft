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
#include "EntityFlags.h"

EntityFlags::EntityFlags() {
	clear();
}

EntityFlags::~EntityFlags() {
}


void EntityFlags::clear() {
	_OnFire = false;
	_Crouched = false;
	_Riding = false;
	_Sprinting = false;
	_RClick = false;
}

bool EntityFlags::isOnFire() {
	return _OnFire;
}


bool EntityFlags::isCrouched() {
	return _Crouched;
}


bool EntityFlags::isRiding() {
	return _Riding;
}


bool EntityFlags::isSprinting() {
	return _Sprinting;
}

bool EntityFlags::isRightClicking() {
	return _RClick;
}

void EntityFlags::setOnFire(bool f) {
	_OnFire = f;
}

void EntityFlags::setCrouched(bool f) {
	_Crouched = f;
}

void EntityFlags::setRiding(bool f) {
	_Riding = f;
}

void EntityFlags::setSprinting(bool f) {
	_Sprinting = f;
}

void EntityFlags::setRightClicking(bool f) {
	_RClick = f;
}