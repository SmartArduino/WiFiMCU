#include <stdint.h>
#include <stdio.h>
#include "cfunctions.h"
#include "cppfunctions.h"


void c_function(int a, int b)
{
    printf("[c_function]: Hello from C\r\n");
    
    // Call C++ function from C.
    // C-linkage on "cpp_function" is required.
    cpp_function(a, b);
}

