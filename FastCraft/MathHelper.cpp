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
#include "MathHelper.h"
#include <cmath>

MathHelper::MathHelper() {
}

MathHelper::~MathHelper() {
}

double MathHelper::distance2D(double x1,double y1,double x2,double y2) {
		return  sqrt (  (x1-x2) * (x1-x2) + 
					    (y1-y2) * (y1-y2)
				     );
}

double MathHelper::distance2D(EntityCoordinates s,EntityCoordinates t) {
	return  sqrt (  (s.X-t.X) * (s.X-t.X) + 
					(s.Z-t.Z) * (s.Z-t.Z)
				 );
}


double MathHelper::distance3D(double x1,double y1,double z1,double x2,double y2,double z2) {
			return  sqrt (  (x1-x2) * (x1-x2) + 
					        (y1-y2) * (y1-y2) +
							(z1-z2) * (z1-z2)
				     );
}



double MathHelper::distance3D(EntityCoordinates s,EntityCoordinates t) {
		return  sqrt (  (s.X-t.X) * (s.X-t.X) + 
					    (s.Z-t.Z) * (s.Z-t.Z) + 
						(s.Y-t.Y) * (s.Y-t.Y)  
				     );
}

double MathHelper::distance3D(BlockCoordinates s,BlockCoordinates t) {
	return sqrt( double((s.X-t.X) * (s.X-t.X) + 
						(s.Z-t.Z) * (s.Z-t.Z) + 
						(s.Y-t.Y) * (s.Y-t.Y)
					   )
			   );
}

double MathHelper::distance2D(ChunkCoordinates s,ChunkCoordinates t) {
	return sqrt( double((s.X-t.X) * (s.X-t.X) + 
						(s.Z-t.Z) * (s.Z-t.Z) 
					   )
	);
}