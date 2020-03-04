#include "EnemyDetection.h"
#include "mbed.h"
#include "UltrasonicArray.h"

#define CENTER_DIST_INDEX 1
#define RIGHT_DIST_DIR 40

#define DIST_GREATERTHAN_CM 0
#define DIST_LESSTHAN_CM 100

EnemyDetection::EnemyDetection(){
    ultraArray = new UltrasonicArray();
    
    PinName triggerPins[1] = {PA_0};
    PinName echoPins[1] = {PA_1};
    ultraArray->setModules(1,triggerPins, echoPins);
}

void EnemyDetection::distCallback(vector<float> distances) {
    /*directionsSum = 0;
    for(int i=0; i<distances.size(); i++) { //for every distance
        if (distances[i] > DIST_GREATERTHAN_CM and distances[i] < DIST_LESSTHAN_CM){
            directionsSum += (distances.size()-1-CENTER_DIST_INDEX)/i * RIGHT_DIST_DIR
        }
    }*/
}

void EnemyDetection::startDetecting(){
    ultraArray->startMeasuring(EnemyDetection::distCallback);
}