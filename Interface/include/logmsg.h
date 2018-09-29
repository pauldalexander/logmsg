/*******************************************************************************

    logmsg.h - Interface file for debug log facility

    -----------------------------------------------------------------------
    
    Copyright 2018 Paul Alexander

    Redistribution and use in source and binary forms, with or without modi-
    fication, are permitted provided that the following conditions are met:

    1. Redistributions of source code must retain the above copyright notice, 
       this list of conditions and the following disclaimer.

    2. Redistributions in binary form must reproduce the above copyright 
       notice, this list of conditions and the following disclaimer in the 
       documentation and/or other materials provided with the distribution.

    3. Neither the name of the copyright holder nor the names of its con-
       tributors may be used to endorse or promote products derived from this 
       software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
    AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
    IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
    ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE 
    LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CON-
    SEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTI-
    TUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTER-
    RUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, 
    STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN 
    ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY 
    OF SUCH DAMAGE.
    
*******************************************************************************/

#ifndef LOGMSG_H

#define LOGMSG_H

/*******************************************************************************
*                                                                              *
*                           Additional header files                            *
*                                                                              *
*******************************************************************************/

// #include <stdint.h>

// #include <stdarg.h>

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

/*******************************************************************************

    LOGMSG_LEVEL - Note that the ordering is consistent with Apache log4j
    
*******************************************************************************/

typedef enum LOGMSG_LEVEL {

	LOGMSG_LEVEL_NONE   = 0,	// Turn off logging
	
	LOGMSG_LEVEL_FATAL  = 1,	// Fatal error - program cannot continue 
	
	LOGMSG_LEVEL_ERROR  = 2,	// Non-fatal - will cause user-visible problems
	
	LOGMSG_LEVEL_WARN   = 3,	// Non-fatal - may cause user-visible problems
	
	LOGMSG_LEVEL_INFO   = 4,	// Interesting runtime events such as startup,
					            // shutdown
	
	LOGMSG_LEVEL_DEBUG 	= 5,	// More detailed information used to trouble-
					            // shoot problems. For per packet/frame infor-
					            // mation, use TRACE instead
	
	LOGMSG_LEVEL_TRACE 	= 6,	// Most detailed information available
	
	LOGMSG_LEVEL_MIN    = 0,
	
	LOGMSG_LEVEL_MAX    = 6,
	
	LOGMSG_LEVEL_UNDEFINED = -1, // Value not defined
	
} LOGMSG_LEVEL;

/*******************************************************************************
*                                                                              *
*                      Program-wide variable declarations                      *
*                                                                              *
*******************************************************************************/

/*******************************************************************************

    logmsg_level - Program-wide logging level.
    
    Description
    ===========
    
    Set this to the highest numerical log level for which log messages should
    be produced in this program.
    
    Default value is LOGMSG_LEVEL_NONE, which turns off logging.
    
    Note that the log level can also be set using the evironment variable
    LOGMSG_LEVEL, which is queried when the process starts.
    
*******************************************************************************/

extern LOGMSG_LEVEL logmsg_level;

/*******************************************************************************
*                                                                              *
*                           Function declarations                              *
*                                                                              *
*******************************************************************************/

/*******************************************************************************

    logmsg_open_file() - Open log file for concurrent writing.
    
    Return 0 on success, -1 on failure.
    
*******************************************************************************/

int logmsg_open_file(const char* file_spec);

/*******************************************************************************

    logmsg_open_conn() - Open network connection to log recorder server.
    
    Return 0 on success, -1 on failure.
    
*******************************************************************************/

int logmsg_open_conn(const char* server_spec);  

/*******************************************************************************

    logmsg_level_to_string() - Convert log level from binary to text
    
*******************************************************************************/

const char* logmsg_level_to_string(LOGMSG_LEVEL level);

/*******************************************************************************

    logmsg_printf() - Write log entry using printf style formatting
    
    Description
    ===========
    
    Write a single log entry of the form:
    
        <utc-time> <log-level> <host-name>:<program-name>[pid:tid] <message>
        
    where:
   
        <utc-time>     = system UTC time in nanosecond precision, returned
                         by clock_gettime() vDSO library function
        
        <log-level>    = name for the log level associated with this entry 
        
        <host-name>    = value returned by gethostname() library function
        
        <program-name> = value stored in the process-wide global variable
                         program_invocation_short_name
                         
        pid            = Process ID assigned to main thread of program, re-
                         turned by getpid() library function
                         
        tid            = Process ID assigned to calling thread, returned by
                         SYS_gettid system call
                         
        <message>      = text produced by feeding format and its following 
                         arguments to the vsnprintf() library function, and
                         appending a newline character
    
*******************************************************************************/

void logmsg_printf(LOGMSG_LEVEL level, const char* format, ...);

/*******************************************************************************

    LOGMSG_PRINTF() - Convenience macro 
    
    Description
    ===========
    
    Invoke logmsg_printf() but supply __FILE__, __LINE__, and __FUNCTION__ 
    as additional initial arguments to be printed.
    
*******************************************************************************/

#define LOGMSG_PRINTF(level, format, ...)                         \
    logmsg_printf(level,                                          \
                  "%s:%d:%s() " format,                           \
                  __FILE__, __LINE__, __FUNCTION__, __VA_ARGS__)

/*******************************************************************************

    LOGMSG_<LEVEL>_PRINTF() - 
    
    Convenience macros which conditionally generate log entries
    
    Description
    ===========
    
    If logmsg_level is >= <LEVEL>, invoke logmsg_printf() but supply __FILE__, 
    __LINE__, and __FUNCTION__ as additional initial arguments to be printed.
    
*******************************************************************************/

#define LOGMSG_FATAL_PRINTF(format, ...)                    \
if (logmsg_level >= LOGMSG_LEVEL_FATAL) {                   \
    LOGMSG_PRINTF(LOGMSG_LEVEL_FATAL, format, __VA_ARGS__); \
}

#define LOGMSG_ERROR_PRINTF(format, ...)                    \
if (logmsg_level >= LOGMSG_LEVEL_ERROR) {                   \
    LOGMSG_PRINTF(LOGMSG_LEVEL_ERROR, format, __VA_ARGS__); \
}

#define LOGMSG_WARN_PRINTF(format, ...)                    \
if (logmsg_level >= LOGMSG_LEVEL_WARN) {                   \
    LOGMSG_PRINTF(LOGMSG_LEVEL_WARN, format, __VA_ARGS__); \
}

#define LOGMSG_INFO_PRINTF(format, ...)                    \
if (logmsg_level >= LOGMSG_LEVEL_INFO) {                   \
    LOGMSG_PRINTF(LOGMSG_LEVEL_INFO, format, __VA_ARGS__); \
}
    
#define LOGMSG_DEBUG_PRINTF(format, ...)                    \
if (logmsg_level >= LOGMSG_LEVEL_DEBUG) {                   \
    LOGMSG_PRINTF(LOGMSG_LEVEL_DEBUG, format, __VA_ARGS__); \
}
    
#define LOGMSG_TRACE_PRINTF(format, ...)                    \
if (logmsg_level >= LOGMSG_LEVEL_TRACE) {                   \
    LOGMSG_PRINTF(LOGMSG_LEVEL_TRACE, format, __VA_ARGS__); \
}


#ifdef __cplusplus
}
#endif // __cplusplus

#endif // LOGMSG_H


