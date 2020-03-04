#ifndef LineDetection_H
#define LineDetection_H
 
#include "mbed.h"

//this is class where i define the controls of robot's moves
class LineDetection { 
public:
    LineDetection();
    bool setGroundColor(); //sets color of ground to the color it is standing on. the opposite color should be line color
    bool isOnLine();
    int8_t getLinePos(); //returns position of line
    bool lineInBack();
private:
    //EDIT THESE variables for initMotorIO, setMotorIO
    DigitalIn* leftSensor;
    DigitalIn* rightSensor;
    
    //local class variable where the default color is saved (if color is not default color, i am in line)
    int groundColor;
};

#endif