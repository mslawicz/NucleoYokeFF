#pragma once

class ViewAngle
{
public:
    ViewAngle(void);
    float getAngle(int movement, float timeElapsed);
private:
    const float AngleChangeRate = 90.0f;		// rate of angle change in deg/s
    const float SpeedFactor = 7.0f;				// how fast angle should follow the requested angle
    float currentAngle;		// current calculated angle in degrees
    float requestedAngle;	// requested angle in degrees
};