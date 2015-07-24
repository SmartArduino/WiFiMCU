#ifndef _CPPFUNCTIONS_H_
#define _CPPFUNCTIONS_H_


/*---------------------------------------------------
 *
 * To be able to call the following C++ functions
 * from C, the C++ compiler needs to be instructed
 * that the functions should be have C linkage.
 * I.e. extern "C" is needed.
 *
 *---------------------------------------------------
 */

#ifdef __cplusplus
extern "C" {
#endif
    
    void cpp_function(int a, int b);
    void cpp_main(void);
#ifdef __cplusplus
}
#endif

void print_my_global(void);

#endif
