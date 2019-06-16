#include "common.h"

void write(MsgLevel eMsgLevel, const char *pFile, const int nLine, const char *psFmt, ...)
{
    va_list l_args;
    char l_sMsg[LOG_DEFAULT_SIZE + 1] = {0};
    va_start(l_args, psFmt);
    vsnprintf(l_sMsg, LOG_DEFAULT_SIZE, psFmt, l_args);
    va_end(l_args);

    switch (eMsgLevel)
    {
    case SUCC:
        fprintf(stderr, "File=%s.Line=%d. level=SUCC. err message=%s\r\n", pFile, nLine, l_sMsg);
        break;
    case INFO:
        fprintf(stderr, "File=%s.Line=%d. level=INFO. err message=%s\r\n", pFile, nLine, l_sMsg);
        break;
    case WARNING:
        fprintf(stderr, "File=%s.Line=%d. level=WARNING. err message=%s\r\n", pFile, nLine, l_sMsg);
        break;
    case ERROR:
        fprintf(stderr, "File=%s.Line=%d. level=ERROR. err message=%s\r\n", pFile, nLine, l_sMsg);
        break;
    case SERIOUS:
        fprintf(stderr, "File=%s.Line=%d. level=SERIOUS. err message=%s\r\n", pFile, nLine, l_sMsg);
        break;
    case UNKNOWN:
        fprintf(stderr, "File=%s.Line=%d. level=UNKNOWN. err message=%s\r\n", pFile, nLine, l_sMsg);
        break;
    default:
        break;
        fprintf(stderr, "File=%s.Line=%d. Invalid argument level=%d\r\n", pFile, nLine, eMsgLevel);
    }

    fflush(stderr);

    return;
}
