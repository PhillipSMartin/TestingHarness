#pragma once

#include <gtk/gtk.h>
#include <iostream>
#include <string>

class Parameters
{
    private:
        const gchar* stagingUrl = "ssl://b-df142c73-36c6-4867-a5f8-eab7534e9e42-1.mq.us-east-2.amazonaws.com:61617";

        void print_help() const
        {
            std::cout << "Usage:" << std::endl;
            std::cout << "   -t <tourney id>" << std::endl;          
            std::cout << "   -l <log level for log file> (default is INFO)" << std::endl;
            std::cout << "   -c <log level for console> (default is INFO)" << std::endl;
            std::cout << "      valid values are NONE, DEBUG, INFO, WARN, ERROR, and FATAL" << std::endl;
            std::cout << std::endl;
            std::cout << "   -d <working directory> (default is location of executable)" << std::endl;
            std::cout << "   -f <filename> script file (absolute path or relative to working directory)" << std::endl;
            std::cout << "   -D <filename> destinations, flagged as topics or queues (default Destinations.txt)" << std::endl;
            std::cout << std::endl;
            std::cout << "   -n noAMQ (for testing GUI without starting an AMQ session)" << std::endl;
            std::cout << "   -q <url>  (url for activeMQ. default is 'tcp://localhost:61616'";
            std::cout << "   -C command-line execution (no GUI)" << std::endl; 
        }

    public:
        enum LogLevel { DEBUG, INFO, WARN, ERROR, FATAL, NONE, UNKNOWN };

    private:   
        LogLevel    fileLogLevel_ = LogLevel::DEBUG;
        LogLevel    consoleLogLevel_ = LogLevel::INFO;
        gchar*      tourneyId_ = g_strdup( "lh-test" );
        gchar*      workingDirectory_ = NULL;
        gchar*      logFileName_ = NULL;
        gchar*      scriptFileName_ = NULL;
        gchar*      destinationsFileName_ = NULL;
        gchar*      AMQurl = NULL;
        gboolean    noAMQ_ = FALSE;
        gboolean    usingGUI_ = TRUE;

        gchar* get_working_directory( const char* programName ) const; // caller owns string
        gchar* get_absolute_path( const char* relativePath );

    public:
        ~Parameters();
        
        LogLevel get_file_log_level() const { return fileLogLevel_; }
        void set_file_log_level( LogLevel const logLevel ) { fileLogLevel_ = logLevel; }

        LogLevel get_console_log_level() const { return consoleLogLevel_; }
        void set_console_log_level( LogLevel const logLevel ) { consoleLogLevel_ = logLevel; }

        const gchar* get_tourney_id() const { return tourneyId_; }
        void set_tourney_id( const gchar* tourneyId ) { g_free( tourneyId_ ); tourneyId_ = g_strdup( tourneyId ); }

        const gchar* get_working_directory() const { return workingDirectory_; }
        void set_working_directory( const gchar* workingDirectory ) { g_free( workingDirectory_ ); workingDirectory_ = g_strdup( workingDirectory ); }

        const gchar* get_log_file_name() const { return logFileName_; }
        void set_log_file_name( const gchar* logFileName ) 
        { 
            g_free( logFileName_ ); 
            logFileName_ = get_absolute_path( logFileName ); 
        }

        const gchar* get_script_file_name() const { return scriptFileName_; }
        void set_script_file_name( const gchar* scriptFileName ) 
        { 
            g_free( scriptFileName_ ); 
            scriptFileName_ = get_absolute_path( scriptFileName ); 
        }

        const gchar* get_destinations_file_name() const { return destinationsFileName_; }
        void set_destinations_file_name( const gchar* destinationsFileName ) 
        { 
            g_free( destinationsFileName_ ); 
            destinationsFileName_ = get_absolute_path( destinationsFileName ); 
        }

        gboolean get_noAMQ() const { return noAMQ_; }
        void set_noAMQ( gboolean noAMQ ) { noAMQ_ = noAMQ; }

        const gchar* get_AMQurl() const { return AMQurl; }
        void set_AMQurl( const gchar* url )
        {
            g_free( AMQurl );
            AMQurl = g_strdup( url );
        }

        gboolean get_usingGUI() const { return usingGUI_; }
        void set_usingGUI( gboolean noGUI ) { usingGUI_ = noGUI; }
     
        gboolean parse_arguments( int const argc, char** const& argv );

        // converts string to LogLevel enum and back
        static LogLevel str_to_log_level( const gchar* logLevelStr );
        static const gchar* log_level_to_str( LogLevel const logLevel );
};