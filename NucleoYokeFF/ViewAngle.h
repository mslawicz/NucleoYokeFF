#pragma once

class ViewAngle
{
public:
    ViewAngle(void);
    float getNewAngle(float currentAngle, float input, float timeElapsed);
    void resetView(void) { resetRequest = true; }
private:
    const float AngleChangeRate = 90.0f;		// rate of angle change in deg/s
    const float SpeedFactor = 6.0f;				// how fast the angle change is applied
    const float resetAngleThreshold = -20.0f;	// used for virtual input in reset mode
    float filteredInput;						// exponentially filtered input
    bool resetRequest;							// if true the view angle is being reset
};