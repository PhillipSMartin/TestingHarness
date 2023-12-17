#include <algorithm>

#include "Parameters.h"

Parameters::~Parameters()
{
    g_free( tourneyId_ );
    g_free( workingDirectory_ );
}

Parameters::LogLevel Parameters::str_to_log_level( const gchar* logLevelStr )
{
    if ( 0 == g_strcmp0( logLevelStr, "DEBUG" ) ) { return LogLevel::DEBUG; }
    if ( 0 == g_strcmp0( logLevelStr, "INFO" ) ) { return LogLevel::INFO; }
    if ( 0 == g_strcmp0( logLevelStr, "WARN" ) ) { return LogLevel::WARN; }
    if ( 0 == g_strcmp0( logLevelStr, "ERROR" ) ) { return LogLevel::ERROR; }
    if ( 0 == g_strcmp0( logLevelStr, "FATAL" ) ) { return LogLevel::FATAL; }
    if ( 0 == g_strcmp0( logLevelStr, "NONE" ) ) { return LogLevel::NONE; }

    return LogLevel::UNKNOWN;
} 

const gchar* Parameters::log_level_to_str( Parameters::LogLevel const logLevel )
{
    switch (logLevel)
    {
        case LogLevel::DEBUG: return "DEBUG";
        case LogLevel::INFO: return "INFO";
        case LogLevel::WARN: return "WARN";
        case LogLevel::ERROR: return "ERROR";
        case LogLevel::FATAL: return "FATAL";
        case LogLevel::NONE: return "NONE";
        default: return "UNKNOWN";
   }
}

gchar* Parameters::get_working_directory( const char* programName ) const
{
    gchar* _executablePath = g_find_program_in_path( programName );
    gchar* _workingDirectory = g_path_get_dirname( _executablePath );
    g_free( _executablePath );
    return _workingDirectory;
}

gchar* Parameters::get_absolute_path( const char* relativePath )
{
    if ( relativePath == NULL )
    {
        return NULL;
    }
    else if ( g_path_is_absolute( relativePath ) )
    {
        return g_strdup( relativePath );
    }
    else
    {
        return g_strdup_printf( "%s/%s", workingDirectory_, relativePath );
    }
}
          
gboolean Parameters::parse_arguments( int const argc, char** const& argv )
{
    set_working_directory( get_working_directory( argv[0] ) );
 
    int _opt;
    gboolean _rc = TRUE;
    std::string _scriptFileName = "";
    std::string _logFileName = "";
    std::string _destinationsFileName = "Destinations.txt";

    while ( _rc && ( -1 != ( _opt = getopt( argc, argv, ":l:q:c:t:d:f:g:D:nC" ) ) ) )
    {
        LogLevel _consoleLogLevel;
        std::string _optargstring = optarg ? optarg : "";
        _optargstring.erase( std::remove_if( _optargstring.begin(), _optargstring.end(), ::isspace ), _optargstring.end() );
        switch (_opt)
        {
            case 'l': 
                fileLogLevel_ = str_to_log_level( _optargstring.c_str() );
                if ( fileLogLevel_ == LogLevel::UNKNOWN )
                {
                    _rc = FALSE;
                }
                break;
            case 'c': 
                _consoleLogLevel = str_to_log_level( _optargstring.c_str() );
                if ( _consoleLogLevel == LogLevel::UNKNOWN )
                {
                   _rc = FALSE;
                }
                else
                {
                    if ( consoleLogLevel_ > _consoleLogLevel )
                    {
                        consoleLogLevel_ = _consoleLogLevel;
                    }
                }
                break;
            case 't':
                set_tourney_id( _optargstring.c_str() );
                break;
            case 'd':
                set_working_directory( _optargstring.c_str() );
                break;
            case 'n':
                set_noAMQ( TRUE );
                break;
            case 'q':
                set_AMQurl( _optargstring.c_str() );
                break;
            case 'C':
                set_usingGUI( FALSE );
                break;

            // we cannot set these file names because we might not have working direcory yet
            case 'f':             
                _scriptFileName = _optargstring;
                break;
            case 'g':             
                _logFileName = _optargstring;
                break;           
            case 'D':
                _destinationsFileName = _optargstring;
                break;
            default: 
               _rc = FALSE;
                break;
         }
    }

    set_script_file_name( _scriptFileName.c_str() );
    set_log_file_name( _logFileName.c_str() );
    set_destinations_file_name( _destinationsFileName.c_str() );

    if ( optind < argc )
    {
        _rc = FALSE;
    }

    if ( !_rc )
    {
        print_help();
    }

    return _rc;
}