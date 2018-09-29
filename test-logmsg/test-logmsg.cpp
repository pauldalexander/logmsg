/*******************************************************************************

    test-logmsg.cpp - Test debug logging facility

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

#include <logmsg.h>

#include <stdlib.h>

#include <stdint.h>

#include <iostream>

/*******************************************************************************

    main()
    
*******************************************************************************/

int main(int argc, char *argv[]) {

    if (argc != 2) {
    
        std::cout << "Invocation: ./test-logmsg <log-file>" << std::endl;
        
        return 1;
    }
    
    {
        int status = logmsg_open_file(argv[1]);
        
        if (status != 0) {
        
            std::cout << "Could not create log file" << std::endl;
            
            return 1;
        }
    }
        
    logmsg_level = LOGMSG_LEVEL_INFO;
    
    if (logmsg_level <= LOGMSG_LEVEL_INFO) {
    
        logmsg_printf(LOGMSG_LEVEL_INFO,
                      "%s:%d:%s() %s %f",
                      __FILE__, __LINE__, __FUNCTION__, 
                      "The value of PI is approximately",
                      3.14159);
    
        logmsg_printf(LOGMSG_LEVEL_INFO,
                      "%s:%d:%s() %s %f",
                      __FILE__, __LINE__, __FUNCTION__, 
                      "The value of E is approximately",
                      2.71828);
    }
            
    return 0;
}



