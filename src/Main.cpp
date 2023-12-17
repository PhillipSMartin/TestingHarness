#include "CLI.h"
#include "Dashboard.h"
#include "DestinationMap.h"
//#include "Logger.h"
#include "SignalHandler.h"

void print_diagnostics( Parameters* pParms, Logger* pLogger )
{
    pLogger->Debug( g_strdup_printf( "Logfile log level set to %s", Parameters::log_level_to_str( pParms->get_file_log_level() ) ) );
    pLogger->Debug( g_strdup_printf( "Console log level set to %s", Parameters::log_level_to_str( pParms->get_console_log_level() ) ) );
    pLogger->Info( g_strdup_printf( "Tourney id set to %s", pParms->get_tourney_id() ) );
    pLogger->Debug( g_strdup_printf( "Working directory set to %s", pParms->get_working_directory() ) );
    if ( pParms->get_script_file_name() )
    {
        pLogger->Info( g_strdup_printf( "Script file set to %s", pParms->get_script_file_name() ) );   
    }
    if ( pParms->get_log_file_name() )
    {
        pLogger->Info( g_strdup_printf( "Log file to replay set to %s", pParms->get_log_file_name() ) );   
    }   
    if ( pParms->get_noAMQ() )
    {
        pLogger->InfoStr( "Running in test mode - no AMQ session" );      
    }
    else
    {
        pLogger->Info( g_strdup_printf( "AMQ url is set to %s", (pParms->get_AMQurl() == NULL) ? "tcp://localhost:61616" : pParms->get_AMQurl()));
    }
    if ( !pParms->get_usingGUI() )
    {
        pLogger->InfoStr( "No GUI - using command line" );
    }
}

int main(int argc, char** argv) 
{
    // build Parameters instance from command-line arguments
    Parameters _parms{};
    if (!_parms.parse_arguments( argc, argv ) )
    {
        exit( 0 );
    }

    // if using GUI, initialize GTK
    if ( _parms.get_usingGUI() )
    {
        _parms.set_usingGUI( gtk_init_check( &argc, &argv ) ); 
    }

    // start Logger
    Logger _logger{&_parms};
    _logger.InfoStr( "Testing harness started" );  
    print_diagnostics( &_parms, &_logger );

    // import Destinations
    DestinationMap _destinations{ &_logger };
    if ( !_destinations.import_destinations( _parms.get_destinations_file_name() ) )
    {
        exit( 0 );
    }

    // attach SIGTERM handler
    SignalHandler _signalHandler( &_logger );
    signal( SIGTERM, _signalHandler.signal_handler );

    AMQClient* _pAMQClient = NULL;
    if ( !_parms.get_noAMQ() )
    {
        _pAMQClient = new AMQClient{ &_logger, _parms.get_usingGUI() };
        if (! _pAMQClient->start( _parms.get_AMQurl() ) )
        {
            _logger.InfoStr( "Shutting down - unable to connect to ActiveMQ");
            return 8;
        }
    }

    // run app 
    if ( _parms.get_usingGUI() )
    {
        Dashboard _Dashboard{&_parms, &_logger, &_destinations, _pAMQClient}; 
        _Dashboard.run();
    }
    else
    {
        CLI _CLI{&_parms, _pAMQClient, &_logger};
        _CLI.run();
    }

    if ( _pAMQClient )
    {
        _pAMQClient->stop();
    }

    _logger.InfoStr( "Testing harness exited" );
    return 0;
}