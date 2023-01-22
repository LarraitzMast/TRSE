#ifndef CONSTANTS_H
#define CONSTANTS_H

#define FALSE   0
#define TRUE    1

#define DEBUG
#ifdef DEBUG
#include <stdio.h>
#define DEBUG_PRINT printf
#else
#define DEBUG_PRINT(...)
#endif

#define RED     'R'
#define YELLOW  'Y'
#define GREEN   'G'

#define EMERGENCY_TYPE_A    'A'
#define EMERGENCY_TYPE_B    'B'

#endif