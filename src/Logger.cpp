#include <iostream>

#include "Logger.h"
#include "utility/Logging.h"

Logger::Logger(Parameters* pParms) : pParms_(pParms)
{
    Parameters::LogLevel _currentLogLevel = pParms_->get_file_log_level();
    if ( _currentLogLevel < Parameters::LogLevel::NONE )
    {
        if ( started_ = Utility::Log::set_unique_log( logFileBase_ ) )
        {
            started_ = Utility::Log::set_log_level( Parameters::log_level_to_str( _currentLogLevel ) ); 
        }
        if ( !started_ )
        {
            pParms_->set_file_log_level( Parameters::LogLevel::NONE );
            std::cout << "WARN: Unable to open logfile" << std:: endl;

            if ( _currentLogLevel < pParms_->get_console_log_level() )
            {
                pParms_->set_console_log_level( _currentLogLevel );
            }
        }
    }
}

void Logger::PrintConsoleMessage( const gchar* message, Parameters::LogLevel const logLevel ) const
{
   if ( pParms_->get_console_log_level() <= logLevel )
    {
        gchar* _msg = g_strdup_printf( "%s: %s\n",  Parameters::log_level_to_str( logLevel ), message );

        if ( pConsoleLogger_ != NULL)
        {
            pConsoleLogger_->add_console_msg( _msg );
        }
        else
        {
            std::cout << _msg;
        }

        g_free( _msg );
    }
}

void Logger::Debug( gchar* message ) const
{
    DebugStr( message );
    g_free( message );
}

void Logger::Info( gchar* message ) const
{
    InfoStr( message );
    g_free( message );
}

void Logger::Warn( gchar* message ) const
{
    WarnStr( message );
    g_free( message );
}

void Logger::Error( gchar* message ) const
{
    ErrorStr( message );
    g_free( message );
}

void Logger::Fatal( gchar* message ) const
{
    FatalStr( message );
    g_free( message );
}

void Logger::DebugStr(std::string const& message) const
{
    if ( !muted_ )
    {
        if ( pParms_->get_file_log_level() <= Parameters::LogLevel::DEBUG )
        {
            LOG_STRM_DEBUG( nullptr, message );
        }
        PrintConsoleMessage( message.c_str(), Parameters::LogLevel::DEBUG );
    }
}

void Logger::InfoStr(std::string const& message) const
{
    if ( !muted_ )
    {
        if ( pParms_->get_file_log_level() <= Parameters::LogLevel::INFO )
        {
            LOG_STRM_INFO( nullptr, message );
        }
        PrintConsoleMessage( message.c_str(), Parameters::LogLevel::INFO );
    }
}

void Logger::WarnStr(std::string const& message) const
{
    if ( pParms_->get_file_log_level() <= Parameters::LogLevel::WARN )
    {
        LOG_STRM_WARN( nullptr, message );
    }
    PrintConsoleMessage( message.c_str(), Parameters::LogLevel::WARN );
}

void Logger::ErrorStr(std::string const& message) const
{
    if ( pParms_->get_file_log_level() <= Parameters::LogLevel::ERROR )
    {
        LOG_STRM_ERROR( nullptr, message );
    }
    PrintConsoleMessage( message.c_str(), Parameters::LogLevel::ERROR );
}

void Logger::FatalStr(std::string const& message) const
{
     if ( pParms_->get_file_log_level() <= Parameters::LogLevel::FATAL )
    {
        LOG_STRM_FATAL( nullptr, message );
    }
    PrintConsoleMessage( message.c_str(), Parameters::LogLevel::FATAL ); 
}
