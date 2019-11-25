#include "ViewAngle.h"

ViewAngle::ViewAngle(void)
{
    currentAngle = 0.0f;
    requestedAngle = 0.0f;
}

/*
* get angle of pilot's view
    input:
    -2 set -90 degrees
    -1 decrease angle (left / down)
    0
    1 increase angle (right / up)
    2 set +90 degrees
    3 set 0 degrees
    timeElapsed: time elapsed since the last call in seconds
    returns view angle in degrees
*/
float ViewAngle::getAngle(int movement, float timeElapsed)
{
    switch (movement)
    {
    case -1:
        requestedAngle -= timeElapsed * AngleChangeRate;
        if (requestedAngle > 180.0f)
        {
            requestedAngle = 180.0f;
        }
        break;
    case 1:
        requestedAngle += timeElapsed * AngleChangeRate;
        if (requestedAngle < -180.0f)
        {
            requestedAngle = -180.0f;
        }
        break;
    case -2:
        requestedAngle = -90.0f;
        break;
    case 2:
        requestedAngle = 90.0f;
        break;
    case 3:
        requestedAngle = 0.0f;
        break;
    case 0:
    default:
        break;
    }
    currentAngle += (requestedAngle - currentAngle) * timeElapsed * SpeedFactor;
    if (currentAngle > 180.0f)
    {
        currentAngle = 180.0f;
    }
    else if (currentAngle < -180.0f)
    {
        currentAngle = -180.0f;
    }
    return currentAngle;
}