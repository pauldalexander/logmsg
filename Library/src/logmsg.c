/*******************************************************************************

    logmsg.c - Implementation file for debug log facility

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

/*******************************************************************************

    Header files
    
*******************************************************************************/

#define _BSD_SOURCE

#define _GNU_SOURCE

#include <stdio.h>

#include <stdlib.h>

#include <string.h>

#include <stdint.h>

#include <stdarg.h>

#include <time.h>

#include <unistd.h>

#include <sys/types.h>

#include <sys/syscall.h>

#include <errno.h>

#include <fcntl.h>

#include <logmsg.h>

/*******************************************************************************

    Program-wide variable declarations
    
*******************************************************************************/

extern char *program_invocation_short_name;

/*******************************************************************************

    Program-wide variable definitions
    
*******************************************************************************/

// logmsg_level - Process wide logging level - Part of public API

LOGMSG_LEVEL logmsg_level = LOGMSG_LEVEL_NONE;

/*******************************************************************************

    Private variable definitions
    
*******************************************************************************/

// Log file or socket FD

static int logger_fd = -1;

// # open log file failures

uint64_t num_open_failures = 0;

// # log server connect failures

uint64_t num_conn_failures = 0;

// # write log file failures

uint64_t num_write_failures = 0;

/*******************************************************************************
*                                                                              *
*                           Private functions                                  *
*                                                                              *
*******************************************************************************/

/*******************************************************************************

    get_utc_time() - Get system time in printable UTC time format, with 
                     nanosecond precision.
                     
    On success, *p_buffer will be written with a NULL terminated string
    containing the formatted UTC time. The return value will be the num-
    ber of characters written, excluding the terminal NULL character.
    
    On failure, a negative value is returned, and the buffer contents is
    undefined.
                     
*******************************************************************************/

static ssize_t get_utc_time(char* p_buffer, size_t buffer_len) {

    /*
     *  Get system (wall clock) high precision time since the Epoch
     */
     
    struct timespec system_time_ns;
    
    { 
        int status = clock_gettime(CLOCK_REALTIME, &system_time_ns);
        
        if (status < 0) {
        
            return -1;
        }
    }
    
    /*
     *  Convert time in seconds since the Epoch to calendar time
     */
     
    struct tm gmt_time;
    
    {
        struct tm* p_tm = gmtime_r(&system_time_ns.tv_sec, &gmt_time);
        
        if (p_tm == NULL) {
        
            return -2;
        }
    }
    
    /*
     *  Write formatted UTC time string
     */    
    
    size_t utc_time_len = 0;
    
    {
        utc_time_len = 
            strftime(p_buffer, buffer_len, "%Y-%m-%d-%T", &gmt_time);
        
        if (utc_time_len == 0) {
        
            return -3;
        }
    }
    
    /*
     *  Append nanosecond portion of system time
     */
     
    {
        if (utc_time_len + 1 + 9 >= buffer_len) {
        
            return -4;
        }
        
        snprintf(p_buffer + utc_time_len, 
                 1 + 9 + 1, 
                 "-%09lu", 
                 system_time_ns.tv_nsec);
        
        utc_time_len += 1 + 9;
    }
    
    return (ssize_t)utc_time_len;
}

/*******************************************************************************

    get_hostname() - Return host name of system
                     
    On success, *p_buffer will be written with a NULL terminated string
    containing the system host name. The return value will be the num-
    ber of characters written, excluding the terminal NULL character.
    
    On failure, a negative value is returned, and the buffer contents is
    undefined.
                     
*******************************************************************************/

static ssize_t get_hostname(char* p_buffer, size_t buffer_len) {

    /*
     *  Get hostname
     */
     
    {
        int status = gethostname(p_buffer, buffer_len);
        
        if (status != 0) {
        
            return -1;
        }
    }
    
    /*
     *  Return number of characters written
     */
     
    return (ssize_t)strlen(p_buffer);
}

/*******************************************************************************

    get_process_and_thread_ids()
                     
    On success, *p_buffer will be written with a NULL terminated string
    containing the system assigned process and thread IDs for the exe-
    cuting process and thread, respectively. The values will be written
    in decimal notation, and separated by a colon. The return value will 
    be the number of characters written, excluding the terminal NULL 
    character.
    
    On failure, a negative value is returned, and the buffer contents is
    undefined.
                     
*******************************************************************************/

static ssize_t get_process_and_thread_ids(char* p_buffer, size_t buffer_len) {

    /*
     *  Get process ID
     */
     
    pid_t pid = getpid();
    
    /*
     *  Get thread ID
     */
     
    pid_t tid = (pid_t)syscall(SYS_gettid);
    
    /*
     *  Format the process and thread IDs
     */

    int copied_len = 0;
    
    {
        copied_len = snprintf(p_buffer, buffer_len, "%d:%d", pid, tid);
        
        if (copied_len < 0 || copied_len >= buffer_len) {
        
            return -1;
        }
    }
    
    /*
     *  Return number of characters written
     */
     
    return copied_len;
}

/*******************************************************************************
*                                                                              *
*                            API functions                                     *
*                                                                              *
*******************************************************************************/

/*******************************************************************************

    logmsg_open_file() - Open log file for concurrent writing.
    
    Return 0 on success, -1 on failure.
    
*******************************************************************************/

int logmsg_open_file(const char* file_spec) {

    /*
     *  Check for file or connection already open
     */
     
    if (logger_fd >= 0) {
    
        num_open_failures++;
        
        return -1;
    }

    /*
     *  Open existing file or create new file.
     */
     
    mode_t mode = S_IRWXU | S_IRWXG | S_IROTH;

    logger_fd = open(file_spec, O_CREAT | O_APPEND | O_WRONLY, mode);

    /*
     *  Check for open failure
     */
     
    if (logger_fd < 0) {
    
        num_open_failures++;
        
        return -1;
    }

    return 0;
}

/*******************************************************************************

    logmsg_open_conn() - Open network connection to log recorder server.
    
    Return 0 on success, -1 on failure.
    
*******************************************************************************/

int logmsg_open_conn(const char* server_spec) {

    // FIXME - To be supplied
    
    return 0;
}

/*******************************************************************************

    logmsg_level_to_string() - Convert log level from binary to text
    
*******************************************************************************/

const char* logmsg_level_to_string(LOGMSG_LEVEL level) {

    static const char* text[] = {
    
        "NONE", "FATAL", "ERROR", "WARN", "INFO", "DEBUG", "TRACE" 
    };
    
    static const char* undef = "UNDEFINED";

    if (level >= LOGMSG_LEVEL_MIN && level <= LOGMSG_LEVEL_MAX) {
    
        return text[level];
        
    } else {
    
        return undef;
    }
}

/*******************************************************************************

    logmsg_printf - Write log entry using printf style formatting
    
    See logmsg.h for more details.
    
*******************************************************************************/

void logmsg_printf(LOGMSG_LEVEL level, const char* format, ...) {

    /*
     *  Get current UTC time with nsec precision as formatted string
     *
     *  sample: 2018-09-22-22:08:42-086858743
     */

    char utc_time_buf[32+1];
    
    ssize_t utc_time_len = 0;
    
    {
        utc_time_len = get_utc_time(utc_time_buf, sizeof(utc_time_buf));
    
        if (utc_time_len < 0) {
        
            const char* error_text = "**** unknown time ****";
            
            utc_time_len = strlen(error_text);
        
            strncpy(utc_time_buf, error_text, utc_time_len + 1);
        }
    }
    
    /*
     *  Convert supplied log level to text
     */
     
    const char* log_level_text = logmsg_level_to_string(level);
    
    size_t log_level_text_len = strlen(log_level_text);
    
    /*
     *  Get hostname of running system
     */

    char hostname_buf[64+1];
    
    ssize_t hostname_len = 0;
    
    {
        hostname_len = get_hostname(hostname_buf, sizeof(hostname_buf));
    
        if (hostname_len < 0) {
        
            const char* error_text = "**** unknown hostname ****";
            
            hostname_len = strlen(error_text);
        
            strncpy(hostname_buf, error_text, hostname_len + 1);
        }
    }
    
    /*
     *  Get name of running program
     */

    const char* program_name = program_invocation_short_name;
    
    size_t program_name_len = strlen(program_name);
    
    /*
     *  Get system assigned process and thread IDs of running process
     *  and calling thread, respectively in formatted decimal
     */

    char pid_tid_buf[32+1];
    
    ssize_t pid_tid_len = 0;
    
    {
        pid_tid_len = 
            get_process_and_thread_ids(pid_tid_buf, sizeof(pid_tid_buf));
    
        if (pid_tid_len < 0) {
        
            const char* error_text = "**** unknown pid and tid ****";
            
            pid_tid_len = strlen(error_text);
        
            strncpy(pid_tid_buf, error_text, pid_tid_len + 1);
        }
    }
    
    /*
     *  Determine the length of the formatted message, not including the
     *  terminal null character.
     */    
    
    char* p_message = NULL;

    char* format_error_text = "**** message formatting error ****";
            
    int message_len = 0;
    
    {
        va_list ap;
        
        va_start(ap, format);
        
        message_len = vsnprintf(p_message, message_len, format, ap);
        
        if (message_len < 0) {
        
            message_len = strlen(format_error_text);
            
            p_message = format_error_text;
        }
    }

    /*
     *  Allocate memory for complete log entry, including terminal NULL
     */
    
    char* p_log_message = NULL;
     
    char* malloc_error_text = "**** heap memory exhausted ****";
    
    size_t log_message_len = 
    
        utc_time_len + 1 + 
        
            log_level_text_len + 1 +
            
                hostname_len + 1 +
            
                    program_name_len +
                    
                        1 + pid_tid_len + 1 + 1 +
                        
                            message_len + 1;

    {                            
        p_log_message = (char*)malloc(log_message_len + 1);
    
        if (p_log_message == NULL) {
        
            log_message_len = strlen(malloc_error_text);
            
            p_log_message = malloc_error_text;
        }
    }
    
    /*
     *  If memory allocation succeeded, construct the complete log message
     */
    
    if (p_log_message != malloc_error_text) {
         
        char* p_write = p_log_message;
        
        /*
         *  Write UTC time string to log message buffer
         */

        {
            strncpy(p_write, utc_time_buf, utc_time_len + 1);
            
            p_write += utc_time_len;
            
            *p_write++ = ' ';
        }

        /*
         *  Write log level text string to log message buffer
         */
         
        {
            strncpy(p_write, log_level_text, log_level_text_len + 1);
            
            p_write += log_level_text_len;
            
            *p_write++ = ' ';
        }

        /*
         *  Write hostname string to log message buffer
         */
         
        {
            strncpy(p_write, hostname_buf, hostname_len + 1);
            
            p_write += hostname_len;
            
            *p_write++ = ':';
        }

        /*
         *  Write program name string to log message buffer
         */
         
        {
            strncpy(p_write, program_name, program_name_len + 1);
            
            p_write += program_name_len;
        }

        /*
         *  Write process and thread ID string to log message buffer
         */
         
        {
            *p_write++ = '[';
            
            strncpy(p_write, pid_tid_buf, pid_tid_len + 1);
            
            p_write += pid_tid_len;
            
            *p_write++ = ']';
            
            *p_write++ = ' ';
        }

       /*
        *   Format caller supplied message and write to log message buffer
        */  

        if (p_message != format_error_text) {
        
            va_list ap;
            
            va_start(ap, format);
            
            vsnprintf(p_write, message_len + 1, format, ap);
            
        } else {
        
            strncpy(p_write, p_message, message_len + 1);
        }
        
        p_write += message_len;
        
        *p_write++ = '\n';
    }
    
    /*
     *  Write to log file or log server connection if open
     */

    if (logger_fd >= 0)    
    {
        ssize_t n_written = 
            write(logger_fd, p_log_message, log_message_len);
        
        if (n_written != log_message_len) {
        
            num_write_failures++;
        }
    }
    
    /*
     *  Release allocated memory
     */
    
    if (p_log_message != malloc_error_text) {
    
        free(p_log_message);
    }
}


