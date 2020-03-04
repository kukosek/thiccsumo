#ifndef EnemyDetection_H
#define EnemyDetection_H

#include "mbed.h"
#include "UltrasonicArray.h"
class EnemyDetection {
public:
    EnemyDetection();
    void startDetecting();
private:
    UltrasonicArray* ultraArray;
    static void distCallback(vector<float> distances);
};
#endif