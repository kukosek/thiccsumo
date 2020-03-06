#include "EnemyDetection.h"
#include "mbed.h"
#include "UltrasonicArray.h"

#define NUMBER_OF_MODULES 3

#define CENTER_DIST_INDEX 1
#define RIGHT_DIST_DIR 40

#define DIST_GREATERTHAN_CM 0
#define DIST_LESSTHAN_CM 100

void (*EnemyDetection::mEnemyPosCallback)(bool, int8_t);

EnemyDetection::EnemyDetection(){
    ultraArray = new UltrasonicArray();
    
    PinName triggerPins[NUMBER_OF_MODULES] = {PB_1, PB_0, PA_4};
    PinName echoPins[NUMBER_OF_MODULES] = {PB_4, PA_12, PA_5};
    ultraArray->setModules(NUMBER_OF_MODULES,triggerPins, echoPins);
}

void EnemyDetection::distCallback(vector<float> distances) {
    bool enemyDetected = false;
    float directionsSum = 0;
    printf("%f\r\n",distances[1]);
    for(uint8_t i=0; i<distances.size(); i++) { //for every distance
        
        if (distances[i] > DIST_GREATERTHAN_CM and distances[i] < DIST_LESSTHAN_CM){
            enemyDetected = true;
            directionsSum += (i-CENTER_DIST_INDEX) / (distances.size()-1-CENTER_DIST_INDEX) * RIGHT_DIST_DIR;
        }
    }
     (*EnemyDetection::mEnemyPosCallback)(enemyDetected, int8_t(directionsSum/distances.size())); //return the average direction
}

void EnemyDetection::startDetecting(void (*callbackFunc)(bool, int8_t)){
    ultraArray->startMeasuring(&EnemyDetection::distCallback);
    mEnemyPosCallback = callbackFunc;
}

void EnemyDetection::stopDetecting(){
    ultraArray->stopMeasuring();
}