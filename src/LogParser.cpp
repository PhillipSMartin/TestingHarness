#include "LogParser.h"

#include "DestinationMap.h"
#include "FileUtilities.h"
#include "JsonUtilities.h"
#include "Logger.h"
#include "Test.h"

Test* LogParser::parse_log_file( const gchar* logFileName )
{
    Test* _pTest = new Test{ logFileName };
    lineNo_ = 0;
    commandsImported_ = 0;
    errors_ = 0;

    gboolean _rc;
    GIOChannel* _pChannel = NULL;
    if ( _rc = FileUtilities::file_open( logFileName, &_pChannel, "r", pLogger_ ) )
    {
        gboolean _eof = FALSE;
        while ( _rc && !_eof)
        {
            _rc = read_line( _pChannel, _pTest, &_eof );
        }
       
        FileUtilities::file_close( logFileName, _pChannel, FALSE, pLogger_ );
    }

    if ( _rc )
    {
        pLogger_->Info( g_strdup_printf( "Successfully imported %d commands. %d commmands could not be parsed.", commandsImported_, errors_ ) );
        return _pTest;
    }
    else
    {
        pLogger_->InfoStr( "Import failed" );
        delete _pTest;
        return NULL;
    }
}

gboolean LogParser::read_line( GIOChannel* pChannel, Test* pTest, gboolean* pEOF)
{
    lineNo_++;
    gchar* _line = NULL;
    gboolean _rc = FileUtilities::file_read_line( pTest->testName_, pChannel, &_line, pEOF, pLogger_ );
    if ( _rc & !*pEOF )
    {
         _rc = parse_line( pChannel, pTest, _line );
    }

    g_free( _line );
    return _rc;
}

gboolean LogParser::parse_line( GIOChannel* pChannel, Test* pTest, const gchar* line)
{
    DestinationId _destinationId;
    if ( !get_destinationId( line, _destinationId ) ) 
    {
        return FALSE;
    }
    else
    {
        if ( _destinationId.destinationType_ != DestinationType::NONE )
        {
            auto _pJson = get_json( pChannel, pTest->testName_ );
            if ( !_pJson )
            {
                pLogger_->Error( g_strdup_printf( "Unable to parse line %d", lineNo_) );
                errors_++;
            }
            else
            {
                pTest->push_back( new TestCommand{TestCommand::SEND, NULL, _destinationId, _pJson} );
    //          pLogger_->Debug( g_strdup_printf( "Imported command for %s %s", _destinationId.to_string().c_str(), nlohmann::to_string( *_pJson ).c_str() ) );
                commandsImported_++;
            }
        }
        return TRUE;
    }
}

nlohmann::json* LogParser::get_json( GIOChannel* pChannel, const gchar* fileName )
{
    nlohmann::json* _pJson = NULL;
    gchar* _nextLine = NULL;
    gboolean _EOF = FALSE;
    lineNo_++;
    gboolean _rc = FileUtilities::file_read_line( fileName, pChannel, &_nextLine, &_EOF, pLogger_  );

    if ( _EOF )
    {
        pLogger_->Error( g_strdup_printf( "Error: premature EOF on line %s", lineNo_ ) );
        _rc = FALSE;          
    }

    if ( _rc )
    { 
        std::string _lineStr{_nextLine};
        size_t _startPos = _lineStr.find_first_of( '{' );
        size_t _endPos = _lineStr.find_last_of( '}' );
        if (_startPos == std::string::npos || _endPos == std::string::npos )
        {
            pLogger_->Error( g_strdup_printf( "Error: no json string found on line %s", lineNo_ ) );
        }
        else
        {
            std::string _jsonStr { _nextLine, _startPos, _endPos - _startPos + 1 };
            _pJson = JsonUtilities::try_parse( _jsonStr.c_str(), pLogger_ );
        }
    }

    g_free( _nextLine );
    return _pJson;
}

gboolean LogParser::get_destinationId( const gchar* line, DestinationId& _destinationId ) const
{
    std::string _lineStr{ line };
    size_t _startPos =  _lineStr.find( startToken_ );
    if ( _startPos != std::string::npos )
    {
        size_t _endPos =  _lineStr.find( endToken_ );
        if ( _endPos == std::string::npos )
        {
            pLogger_->Error( g_strdup_printf( "Error parsing line %d: No '%s' found", lineNo_, endToken_ ) );
            return FALSE;
        }

        _startPos += strlen( startToken_ );
        _destinationId.destinationName_ = std::string{ line + _startPos, _endPos - _startPos };
        _destinationId.destinationType_ = pDestinationMap_->get_destination_type( _destinationId.get_destination_name() );
    }

    return TRUE;
}