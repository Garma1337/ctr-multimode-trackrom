#ifndef MATH_H_MODEROM
#define MATH_H_MODEROM

int Math_Clamp(int value, int min, int max);
int Math_Wrap(int value, int count);
int Math_Scale(int value, int total, int range);
int Math_Percent(int value, int total);

#endif
