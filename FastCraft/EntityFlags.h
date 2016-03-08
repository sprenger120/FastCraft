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

#ifndef _FASTCRAFTHEADER_ENTITYFLAGS	
#define _FASTCRAFTHEADER_ENTITYFLAGS

class EntityFlags {
private:
	bool _OnFire;
	bool _Crouched;
	bool _Riding;
	bool _Sprinting;
	bool _RClick;
public:
	EntityFlags();
	~EntityFlags();

	void clear();

	bool isOnFire();
	bool isCrouched();
	bool isRiding();
	bool isSprinting();
	bool isRightClicking();

	void setOnFire(bool);
	void setCrouched(bool);
	void setRiding(bool);
	void setSprinting(bool);
	void setRightClicking(bool);
};
#endif