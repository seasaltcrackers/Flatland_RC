#pragma once

class Constants
{
public:

	
	static const int WindowWidth = 1200;
	static const int WindowHeight = 800;

	static const int WorldWidth = 512;
	static const int WorldHeight = 512;

	// E.g. at 2 this will put a probe every 2 pixels
	// Must be a power of 2 number e.g. 1, 2, 4, 8
	static const int Cascade0ProbeSpacing = 2;

	// The angular resolution of cascade 0 is X * Y
	// Both numbers must be a power of 2 number e.g. 1, 2, 4, 8
	// But both numbers can be different, e.g. 4, 8 gives 32 angles of resoltuion at Cascade 0
	static const int Cascade0AngularResolutionX = 4;
	static const int Cascade0AngularResolutionY = 8;

};
