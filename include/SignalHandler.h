#pragma once

#include "Logger.h"
#include <csignal>

class SignalHandler
{
    private:
        static Logger* pLogger_;

    public:
        SignalHandler( Logger* pLogger ) 
        {
            pLogger_ = pLogger;
        }
        static void signal_handler( int signum );
};  