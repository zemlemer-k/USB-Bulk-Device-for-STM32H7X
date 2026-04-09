#ifndef _LIB_TERMINAL__H_
#define _LIB_TERMINAL__H_

// define section

// Maximum terminal string size
#define MAX_TERMINAL_STRING_LNG 200

// Maximum integer value length
#define MAX_INTEGER_STRING_LNG  20

// Maximum float value length
#define MAX_FLOAT_STRING_LNG  20


// Message types
typedef enum
{
    empty = 0,
    error,
    warning,
    info
}tMsgtype;

// Main terminal message function
void setTerminalMessage( tMsgtype mtype, const char* fmt, ... );

#endif // _LIB_TERMINAL__H_
