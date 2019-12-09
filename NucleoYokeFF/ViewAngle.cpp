#include "ViewAngle.h"

ViewAngle::ViewAngle(void)
{
    filteredInput = 0.0f;
    resetRequest = false;
}

/*
* get the angle change value
*/
float ViewAngle::getNewAngle(float currentAngle, float input, float timeElapsed)
{
    // EMA filter strength should depend on the time elapsed (to preserve real time filtering effect)
    float alpha = SpeedFactor * timeElapsed;
    // don't allow alpha > 1 (can happen when time elapsed is huge)
    if (alpha > 1.0f)
    {
        alpha = 1.0f;
    }
    // if reset signal is active, calculate virtual input
    if (resetRequest)
    {
        input = currentAngle / resetAngleThreshold;
        if (input > 1.0f)
        {
            input = 1.0f;
        }
        else if (input < -1.0f)
        {
            input = -1.0f;
        }
    }
    // filter input signal
    filteredInput += alpha * (input - filteredInput);
    // calculate new angle
    currentAngle += filteredInput * AngleChangeRate * timeElapsed;
    // check if reset condition is achieved
    if (resetRequest &&
        (currentAngle > -0.5f) &&
        (currentAngle < 0.5f))
    {
        //reset view and filter
        currentAngle = 0.0f;
        filteredInput = 0.0f;
        resetRequest = false;
    }
    return currentAngle;
}