#pragma once

int Wrap(int a, int n); 
float Clamp(float Min, float Val, float Max); 

unsigned int MapIndex(unsigned int Dimensions, const int* Position, const unsigned int* Size);
