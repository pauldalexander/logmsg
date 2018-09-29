/*******************************************************************************

    write-test
    
    Test simultaneous writes to file

    ------------------------------------------------------------------------
    
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

extern char *program_invocation_short_name;

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

    sleep_secs() - Sleep with nanosecond precision
    
    Description
    ===========
    
    Sleep for specified number of seconds using nanosecond precision
    system clock.
    
    If supplied value is zero or out of range, or an error occurs,
    return without sleeping.
                     
*******************************************************************************/

static void sleep_secs(double sleep_time_secs) {

    /*
     *  If supplied sleep time is not positive, don't sleep
     */

    if (sleep_time_secs <= 0.0) {
    
        return;
    }

    /*
     *  Convert double to timespec
     */
     
    struct timespec sleep_time_ns;
    
    sleep_time_ns.tv_sec = sleep_time_secs;
    
    sleep_time_ns.tv_nsec = (sleep_time_secs - sleep_time_ns.tv_sec) * 1.0e9;
    
    /*
     *  Sleep for specified interval
     */     
     
    clock_nanosleep(CLOCK_REALTIME, 
                    0 /* relative time */, 
                    &sleep_time_ns, 
                    NULL);   
}

/*******************************************************************************

    open_file() - Open file for concurrent writing.
    
*******************************************************************************/

static int open_file(const char* file_spec) {

    mode_t mode = S_IRWXU | S_IRWXG | S_IROTH;

    int fd = open(file_spec, O_CREAT | O_APPEND | O_WRONLY, mode);

    return fd;
}

/*******************************************************************************

    main()
    
    Command line arguments:

        Output filename - create if not open
        
        Number of iterations - double precision
        
        Number of seconds between writes - double precision
        
    Output:

        <utc-time> <host-name>:<program-name>[pid:tid] <sequence-number>

*******************************************************************************/

int main(int argc, char **argv) {

    /***************************************************************************
    
        Get program arguments
        
    ***************************************************************************/
    
    char* outfile_name = NULL;      // Name of file to write to
    
    uint64_t num_total_writes = 0;  // Number of total writes to make
    
    double delta_secs = 0.0;        // Number of seconds between writes

    {
        const char* invocation_message = 
            "Invocation: ./write-test <out-file> <num-iters> <delta-secs>";

        if (argc != 4) {
        
            printf("\n%s\n", invocation_message);
        
            return 1;
        }   
        
        outfile_name = argv[1];
        
        {
            double num_iters_d = 0.0;
        
            if (sscanf(argv[2], "%lf", &num_iters_d) != 1)
            {
                printf("\n%s\n", invocation_message);
            
                return 1;
            }
            
            num_total_writes = num_iters_d;
        }
        
        {
            if (sscanf(argv[3], "%lf", &delta_secs) != 1)
            {
                printf("\n%s\n", invocation_message);
            
                return 1;
            }
        }
    }

    /***************************************************************************
    
        Get hostname
        
    ***************************************************************************/

    char hostname_buf[64+1];
    
    {
        ssize_t hostname_len = 0;
    
        hostname_len = get_hostname(hostname_buf, sizeof(hostname_buf));
    
        if (hostname_len < 0) {
        
            printf("\nCould not determine hostname\n");
            
            return 1;
        }
    }
    
    /***************************************************************************
    
        Get program name
        
    ***************************************************************************/

    const char* program_name = program_invocation_short_name;
    
    /***************************************************************************
    
        Get process and thread IDs
        
    ***************************************************************************/
    
    char pid_tid_buf[32+1];
    
    {
        ssize_t pid_tid_len = 0;
    
        pid_tid_len = 
            get_process_and_thread_ids(pid_tid_buf, sizeof(pid_tid_buf));
    
        if (pid_tid_len < 0) {
        
            printf("\nCould not determine pid and tid\n");
            
            return 1;
        }
    }
    
    /***************************************************************************
    
        Open output file
        
    ***************************************************************************/
    
    int fd = open_file(outfile_name);
    
    if (fd < 0) {
    
        printf("\nCould not create or open output file for writing\n");
    }
    
    /***************************************************************************
    
        Write entries to output file
        
    ***************************************************************************/
    
    char write_buf[256+1];

    for (uint64_t seq_num = 0; seq_num < num_total_writes; seq_num++) {

        /*
         *  Get UTC time
         */
    
        char utc_time_buf[32+1];
        
        {
            ssize_t utc_time_len = 0;
        
            utc_time_len = get_utc_time(utc_time_buf, sizeof(utc_time_buf));
        
            if (utc_time_len < 0) {
            
                printf("\nCould not determine UTC time\n");
                
                return 1;
            }
        }

        /*
         *  Assemble output message
         */

        int write_len =
            snprintf(write_buf, 
                     sizeof(write_buf), 
                     "%s %s:%s[%s] %lu\n",
                     utc_time_buf, 
                     hostname_buf,
                     program_name,
                     pid_tid_buf,
                     seq_num);
        
        /*
         *  Write message to file
         */

        {
            ssize_t n_written = write(fd, write_buf, write_len);
            
            if (n_written != write_len) {
            
                printf("\nwrite() did not write all bytes\n");
                
                return 1;
            }
        }
        
        /*
         *  Sleep between writes
         */

        sleep_secs(delta_secs);
    }        

	return 0;
}


