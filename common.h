#ifndef _COMMON_H_
#define _COMMON_H_

#include <string>
#include <iostream>
#include <stdarg.h>

typedef char int8;
typedef unsigned char uint8;
typedef short int16;
typedef unsigned short uint16;
typedef int int32;
typedef unsigned int uint32;
typedef long long int int64;
typedef unsigned long long int uint64;

using std::string;
using std::cout;

const uint32 LOG_DEFAULT_SIZE = 4096;

//错误代码
const int32 SUCCESS = 0;
const int32 ERR_UNKNOWN = -1;
const int32 ERR_ARG = 10;

enum MsgLevel {
    SUCC = 0,        /**< Log Type Successfully Processed */
    INFO,            /**< Log Type Infomation */
    WARNING,         /**< Log Type Warning */
    ERROR,           /**< Log Type Error */
    SERIOUS,         /**< Log Type Serious */
    UNKNOWN          /**< Log Type UNKNOWN */
};

void write(MsgLevel eMsgLevel, const char* pFile, const int nLine, const char* psFmt, ...);

#define SUCCLOG(psFmt, ...) (write(SUCC, __FILE__, __LINE__, psFmt, ##__VA_ARGS__))
#define INFOLOG(psFmt, ...) (write(INFO, __FILE__, __LINE__, psFmt, ##__VA_ARGS__))
#define WARNINGLOG(psFmt, ...) (write(WARNING, __FILE__, __LINE__, psFmt, ##__VA_ARGS__))
#define ERRORLOG(psFmt, ...) (write(ERROR, __FILE__, __LINE__, psFmt, ##__VA_ARGS__))
#define SERIOUSLOG(psFmt, ...) (write(SERIOUS, __FILE__, __LINE__, psFmt, ##__VA_ARGS__))
#define UNKNOWNLOG(psFmt, ...) (write(UNKNOWN, __FILE__, __LINE__, psFmt, ##__VA_ARGS__))

#endif
