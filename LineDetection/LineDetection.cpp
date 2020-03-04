#include "LineDetection.h"
#include "mbed.h"

LineDetection::LineDetection(){
    leftSensor= new DigitalIn(PB_5);
    rightSensor= new DigitalIn(PA_0);
}

bool LineDetection::setGroundColor(){
    if (leftSensor->read()==rightSensor->read()){
        groundColor = leftSensor->read();
        return true;
    }else{
        return false;
    }
}

bool LineDetection::isOnLine(){
    if (leftSensor->read()!=groundColor or rightSensor->read()!=groundColor){
        return true;
    }else{
        return false;
    }
}

bool LineDetection::lineInBack(){
    return false;
}

int8_t LineDetection::getLinePos(){
    bool leftLine=leftSensor->read()==groundColor;
    bool rightLine=rightSensor->read()==groundColor;
    if (leftLine and rightLine){
        return 0;
    }else if (leftLine){
        return -100;
    }else if (rightLine){
        return 100;
    }else{
        return NULL;   
    }
}