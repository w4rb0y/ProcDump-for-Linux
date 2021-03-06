// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License

//--------------------------------------------------------------------
//
// A simple logging library for log generation and debugging
// 
//--------------------------------------------------------------------

#include "Logging.h"
#include "ProcDumpConfiguration.h"

static const char *LogLevelStrings[] = { "DEBUG", "INFO", "WARN", "CRITICAL", "ERROR" }; 

void LogFormatter(enum LogLevel logLevel, const char *message, va_list args)
{
    char timeBuff[64];
    time_t rawTime;
    struct tm *timeInfo=NULL;
    char* trace=NULL;

    va_list copy;
    va_copy(copy, args);

    pthread_mutex_lock(&LoggerLock);

    rawTime = time(NULL);
    timeInfo = localtime(&rawTime);
    strftime(timeBuff, 64, "%T", timeInfo);

    int traceLen = snprintf(NULL, 0, "[%s - %s]: ", timeBuff, LogLevelStrings[logLevel]);
    int argsLen = vsnprintf(NULL, 0, message, copy);
    if(!(trace = malloc(traceLen+argsLen+1)))
    {
        pthread_mutex_unlock(&LoggerLock);
        return;
    }
    
    sprintf(trace, "[%s - %s]: ", timeBuff, LogLevelStrings[logLevel]);
    vsprintf(trace+traceLen, message, args);

    // If a log entry is not 'debug' it simply goes to stdout. 
    // If you want an entry to only go to the syslog, use 'debug' 
    if(logLevel != debug)
    {
        printf("%s\n", trace);
    }

    // All log entries also go to the syslog
    syslog(LOG_DEBUG, "%s", trace);

    va_end(copy);
    free(trace);
    pthread_mutex_unlock(&LoggerLock);
}

void Log(enum LogLevel logLevel, const char *message, ...)
{
    va_list args;
    va_start(args, message);
    LogFormatter(logLevel, message, args);
    va_end(args);
}


void DiagTrace(const char *message, ...)
{
    va_list args;
    va_start(args, message);
    if(g_config.DiagnosticsLoggingEnabled) LogFormatter(debug, message, args);
    va_end(args);
}
