/*******************************************************************************

    get-logging-info
    
    Gather and print information to be used by dbglog library

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

#define _BSD_SOURCE

#define _GNU_SOURCE

#include <stdio.h>

#include <string.h>

#include <stdint.h>

#include <stdarg.h>

#include <time.h>

#include <unistd.h>

#include <sys/types.h>

#include <sys/syscall.h>

#include <errno.h>

extern char *program_invocation_short_name;

/*******************************************************************************

    main()
    
    Invoke with no arguments
    
*******************************************************************************/

int main(int argc, char **argv) {

    {
        struct timespec system_time_ns = { 0, 0 };
        
        clock_gettime(CLOCK_REALTIME, &system_time_ns);
        
        struct tm gmt_time;
        
        memset(&gmt_time, 0, sizeof(struct tm));
        
        gmtime_r(&system_time_ns.tv_sec, &gmt_time);
        
        char s_gmt_time[64];
        
        memset(s_gmt_time, 0, sizeof(s_gmt_time));
        
        // size_t s_gmt_time_len = 
        strftime(s_gmt_time, sizeof(s_gmt_time), "%Y-%m-%d-%T", &gmt_time);
        
        printf("UTC time      = %s-%09lu\n", s_gmt_time, system_time_ns.tv_nsec);
    }
    
    {
        #define HOST_NAME_MAX 64
        
        char host_name[HOST_NAME_MAX+1];
        
        memset(host_name, 0, sizeof(host_name));
        
        gethostname(host_name, sizeof(host_name));
        
        printf("Host name     = %s\n", host_name);
    }
    
    {
        printf("Program name  = %s\n", program_invocation_short_name);
    }
    
    {
        pid_t pid = getpid();
        
        printf("Program lwpid = %d\n", pid);
    }
    
    {
        pid_t tid = (pid_t)syscall(SYS_gettid);
        
        printf("Thread lwpid  = %d\n", tid);
    }
    
	return 0;
}
