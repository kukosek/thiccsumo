#ifndef EnemyDetection_H
#define EnemyDetection_H

#include "mbed.h"
#include "UltrasonicArray.h"
class EnemyDetection {
public:
    EnemyDetection();
    void startDetecting(void (*callbackFunc)(bool, int8_t));
    UltrasonicArray* ultraArray;
    void stopDetecting();
private:
    
    static void (*mEnemyPosCallback)(bool, int8_t);
    static void distCallback(vector<float> distances);
};
#endif