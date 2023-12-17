#include "Logger.h"
#include "SignalHandler.h"

Logger* SignalHandler::pLogger_;

void SignalHandler::signal_handler( int signum )
{

    pLogger_->Info( g_strdup_printf( "Received signal %d", signum ) );
    exit( signum );

}