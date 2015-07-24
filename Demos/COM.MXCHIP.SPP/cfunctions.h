#ifndef _CFUNCTIONS_H_
#define _CFUNCTIONS_H_


/*---------------------------------------------------
 *
 * To be able to call the following C functions
 * from C++, the C++ compiler needs to be informed 
 * that the functions (already) have C linkage.
 * I.e. extern "C" is needed.
 *
 *---------------------------------------------------
 */

#ifdef __cplusplus
extern "C" {
#endif

    void LCD_DisplayString(uint8_t *text);
    void c_function(int a, int b);
        
#ifdef __cplusplus
}
#endif


#endif
