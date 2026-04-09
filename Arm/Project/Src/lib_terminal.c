/*
 ***********************************************************************************************************************
* lib_terminal.c
* Kirill Gribovskiy
* version V2.0.1
* 2-Mar-2025
* terminal output source file
 ***********************************************************************************************************************
*/

// Includes
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctype.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>

#include "lib_terminal.h"
#include "bd_uart.h"


// Typedef 
typedef enum { integer= 0, unsinteger, hexadecimal} tValtype;

// Define
// Macro
// Variables
static char st_TerminalStr[MAX_TERMINAL_STRING_LNG];
static char st_IntererStr[MAX_INTEGER_STRING_LNG];
static char st_FloatStr[MAX_FLOAT_STRING_LNG];

static const char *st_CharTable = "0123456789ABCDEF";  // char search table

// Function prototypes
static char *convDigitToSring(int data, unsigned char minlength, int fillzeros, tValtype type);
static char *convFloatToSring(float value);

// Functions

/************************************************************************************************************************
* Function Name  : convDigitToSring
* Description    : Creates string from integer input digits
* Input          : int data -- input digit from witch the string will be created
*                  unsigned char minlength -- minimum string length
*                  int  fillzeros -- if 0 !=fillzeros fill string with leading zeros
*                                    otherwise - filling with spaces
*                  tValtype type -- data display type: hex, decs,....
* Output         : None.
* Return         : Pointer to the string.
***********************************************************************************************************************
*/
static char *convDigitToSring(int data, unsigned char minlength, int fillzeros, tValtype type)
{
    unsigned char   dimension = 0;  // number of digits
    unsigned char   strlength;      // string length
    unsigned char   radix;          // hex or dec
    int             val;            // additional variable
    int             positive = 1;   // result sign
    char            *ptr;           // additional pointer
    int             dim_data;       // result string dimension

    // Emptying result string
    if(fillzeros)
        memset(st_IntererStr, '0', MAX_INTEGER_STRING_LNG);
    else
        memset(st_IntererStr, ' ', MAX_INTEGER_STRING_LNG);

    // value dimension
    if((integer == type) && (data < 0)) {
        positive = 0;
        val = -1 * data;
    }
    else{
        val = data;
    }

    dim_data = val;

    // Value dimension calculation
    if(hexadecimal != type) {
        radix = 10;
        do {
            dim_data /= radix;
            dimension++;
        } while(dim_data);
    }
    else {
        radix = 16;
        if(0 == minlength)
            dimension = 8;
        else {
            dimension = minlength;
        }
    }

    // string size calculation length + \0
    if((dimension < minlength)/* && (fillzeros)*/)
        strlength = minlength + 1;
    else
        strlength = dimension + 1;

    if(0 == positive)
        strlength++; // sign
    if(hexadecimal == type)
        strlength += 2;

    if((strlength + 1) > MAX_INTEGER_STRING_LNG )
        return NULL;

    ptr = st_IntererStr + strlength - 1; // was - 1
    *ptr-- = '\0';

    //
    if(hexadecimal != type) {
        do {
            *ptr-- = st_CharTable[val%radix];
            val /= radix;
        }while(val);
    }
    else {
        do {
            *ptr-- = st_CharTable[(val & 0x0F)];
            val >>= 4;
        } while(--dimension);
    }

    if(0 == positive)
        st_IntererStr[0] = '-';
    if(hexadecimal == type){
        st_IntererStr[0] = '0';
        st_IntererStr[1] = 'x';
    }
    return st_IntererStr;
}

/*
***********************************************************************************************************************
* Function Name  : convFloatToSring
* Description    : Creates string from float value
* Input          : float value -- input float digit
* Output         : None.
* Return         : Pointer to the string.
***********************************************************************************************************************
*/

static char *convFloatToSring(float value)
{
    const float precision = 0.000001f; // float max precision
    const int   fcractSz = 6; // fractional size - after dot    

    memset(st_FloatStr, 0, MAX_FLOAT_STRING_LNG);

    uint32_t *f_addr = (uint32_t *)&value;
    uint32_t f_val = *f_addr;

    bool neg_sign = (value < 0) ? true : false; // (value >> 31) & 0x01;
    int  exp  = ((f_val >> 23) & 0xFF) - 127;
    int  mant = f_val & 0x7FFFFF;

    if(128 == exp) {
        if(0 == mant)   strcpy(st_FloatStr, "INF");
        else            strcpy(st_FloatStr, "NAN");
        return st_FloatStr;
    }

    char *posPtr = st_FloatStr;
    int  posCnt = 0;
    
    if(neg_sign) {
        *posPtr++ = '-';
        posCnt++;
        value *= (-1.0); // changing sign` 
    } 

    int   int_part = (int)value;
    float fract_part = value - int_part;
    char *intPtr = convDigitToSring(int_part, 0, 0, unsinteger);

    // Checking length, at least one digit after dot
    if(((posCnt + sizeof(intPtr)) >= (MAX_FLOAT_STRING_LNG - 3)) || (NULL == intPtr))
        return st_FloatStr; // returning only integer part

    strcpy(posPtr, intPtr);
    posPtr += strlen(intPtr);
    posCnt += strlen(intPtr);

    *posPtr++ = '.';
    posCnt++;

    for(int i = 0; i < fcractSz; i++) {
        if((i + posCnt) == (MAX_FLOAT_STRING_LNG - 1)) // digit size checking 
            return st_FloatStr;
        if(fract_part < precision) // only zeros left
            return st_FloatStr;
        fract_part *= 10.0;
        int digit = (int)fract_part;

        *posPtr++ = '0' + digit;
        posCnt++;

        fract_part -= (float)digit;
    }

    return st_FloatStr;
}

/*
***********************************************************************************************************************
* Function Name  : setTerminalMessage
* Description    : Creates terminal message string according to input parameters and
*                  sends it.
* Input          : tMsgtype mtype  -- message type
                 : const char* fmt -- message format string
* Output         : None.
* Return         : Nothing.
***********************************************************************************************************************
*/
void setTerminalMessage( tMsgtype mtype, const char* fmt, ... )
{
    char *msgptr = st_TerminalStr;      // pointer to the terminal transmit buffer
    static unsigned char msgnum = 0;    // message number
    unsigned char msglng;               // output message length
    char *cp;                           // pointer to the format string
    int fillzeros;                      // append lead zeros before number in string
    va_list ap;                         // format string stuff
    unsigned char minlength;            // minimum digit string length
    char *valstr;                       // string delivered from the format parser
    char formatcode;                    // value format
    unsigned int intarg;                // integer format string argument
    char chararg;                       // char format string argument
    char *strarg;                       // string char format string argument

    // 1. Header
    // 1.1 Message number
    memset(st_TerminalStr, '\0', MAX_TERMINAL_STRING_LNG);
    msgnum = (msgnum % 1000);
    strcpy(msgptr, convDigitToSring(msgnum, 3, 1, unsinteger));
    msgnum++;
    strcat(msgptr, " ");

    // 1.2 Message type
    switch(mtype)
    {
        case error:
            strcat(msgptr, "ERROR: ");
            break;
        case warning:
            strcat(msgptr, "WARNING: ");
            break;
        case info:
            strcat(msgptr, "INFO: ");
            break;
        default:
            return;
    }

    msglng = strlen(st_TerminalStr) + 3;    // reassurance
    msgptr = st_TerminalStr + strlen(st_TerminalStr);

    // Main message
    va_start(ap, fmt);
    cp = (char*)fmt;

    while(*cp) {
        // message length check
        if( msglng > MAX_TERMINAL_STRING_LNG)
            break;
        // regular data
        if('%' != *cp) {
            *msgptr++ = *cp++;
            msglng++;
            continue;
        }
        // format parser
        cp++;
        fillzeros = 0;
        minlength = 0;
        // checking leading zeros
        if(isdigit((unsigned char)(*cp))) {
            if('0' == *cp) {
                fillzeros = 1;
                cp++;
            }
            if((*cp > '0') && (*cp <= '9')) {
                minlength = *cp - '0';
                cp++;
            }
            else {
                va_end(ap);
                return;
            }
        }
        formatcode = *cp++;
        valstr = NULL;
        switch(formatcode)
        {
            unsigned char fmtlegth;
            float  float_arg;

            case 'd':
            case 'u':
            case 'X':
            case 'x':    // digit value
                intarg = va_arg(ap, int32_t);
                switch(formatcode)
                {
                    case 'd' :  // decimal
                        valstr = convDigitToSring(intarg, minlength, fillzeros, integer);
                        break;
                    case 'u' :  //unsigned
                        valstr = convDigitToSring(intarg, minlength, fillzeros, unsinteger);
                        break;
                    case 'x' :  //hexadecimal
                        valstr = convDigitToSring(intarg, minlength, fillzeros, hexadecimal);
                        break;
                }

                if(NULL == valstr) {
                    va_end(ap);
                    return;
                }

                fmtlegth = strlen(valstr);
                if((msglng + fmtlegth) > (MAX_TERMINAL_STRING_LNG - 4)) {
                    va_end(ap);
                    return;
                }

                strcat(st_TerminalStr, valstr);
                msgptr += fmtlegth;
                break;

            case 'f' :
                float_arg = (float) va_arg(ap, double);
                valstr = convFloatToSring(float_arg);
                if(NULL == valstr) {
                    va_end(ap);
                    return;
                }
                fmtlegth = strlen(valstr);
                if((msglng + fmtlegth) > (MAX_TERMINAL_STRING_LNG - 4)) {
                    va_end(ap);
                    return;
                }
                strcat(st_TerminalStr, valstr);
                msgptr += fmtlegth;
                break;
            
            case 'c' :  // char
                chararg = (char)va_arg(ap, int);
                *msgptr++ = chararg;
                msglng++;

                if(msglng > (MAX_TERMINAL_STRING_LNG - 4)) {
                    va_end(ap);
                    return;
                }
                break;

            case 's' : // string
                strarg = va_arg(ap, char *);
                fmtlegth = strlen(strarg);
                msglng += fmtlegth;
                msgptr += fmtlegth;
                if(msglng > (MAX_TERMINAL_STRING_LNG - 4)) {
                    va_end(ap);
                    return;
                }
                strcat(st_TerminalStr, strarg);
                break;
            default :
                va_end(ap);
                return;
        }
    }
    va_end(ap);
    msglng = strlen(st_TerminalStr);
    st_TerminalStr[msglng++] = '\n';
    st_TerminalStr[msglng++] = '\r';

    trnDebugData(st_TerminalStr, msglng);
}
