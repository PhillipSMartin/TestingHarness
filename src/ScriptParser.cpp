#include "FileUtilities.h"
#include "JsonUtilities.h"
#include "Logger.h"
#include "ScriptParser.h"
#include "TestExecutor.h"
#include "Test.h"

gint TestCommand::nextNumber_{ 0 };

void ScriptParser::cleanup_from_previous_test()
{
    if ( pCurrentTest_ )
    {
        delete pCurrentTest_;
        pCurrentTest_ = NULL;
    }
    if ( pTests_ )
    {
        delete pTests_;
        pTests_ = NULL;
    }
}

gboolean ScriptParser::run_script( TestExecutor* pExecutor )
{
    GIOChannel* _pChannel = NULL;
    const gchar* _fileName = pParms_->get_script_file_name();
    gboolean _rc = FALSE;
    gint _numSuccesses = 0;
    gint _numFailures = 0;

    TestCommand::nextNumber_ = 0;
    if ( _rc = FileUtilities::file_open( _fileName, &_pChannel, "r", pLogger_ ) )
    {
        try
        {
            _rc = build_tests( _fileName, _pChannel );
            if ( _rc )
            {
                for ( Test* _pTest : *pTests_ )
                {
                    if ( pExecutor->run_test( _pTest ) )
                    {
                        _numSuccesses++;
                        pLogger_->Info( g_strdup_printf( "*** Test %s succeeded ***", _pTest->testName_ ) );
                    }
                    else
                    {
                        _numFailures++;
                        pLogger_->Info( g_strdup_printf( "*** Test %s failed ***", _pTest->testName_ ) );
                    }
                }
            }
        }
        catch( const std::exception& e )
        {
            pLogger_->Error( g_strdup_printf( "Error running script: %s", e.what() ) );
            _rc = FALSE;
        }
        
        FileUtilities::file_close( _fileName, _pChannel, FALSE, pLogger_ );
        pLogger_->Info( g_strdup_printf( "*** %d test%s succeeded; %d failed ***", _numSuccesses, (_numSuccesses == 1) ? "" : "s", _numFailures ) );
    }

    cleanup_from_previous_test();
    return _rc;
}

gboolean ScriptParser::build_tests( const char* fileName, GIOChannel* pChannel )
{
    pTests_ = new Tests();
    gboolean _rc = TRUE;
    gboolean _done = FALSE;
    gchar* _command = NULL;

    while ( _rc )
    {
        _rc = FileUtilities::file_read_line( fileName, pChannel, &_command, &_done, pLogger_  );
        if ( _done )
        {
            break;
        }

        TestCommand::bump_number();
        if ( strlen( _command ) > 1 ) // ignore empty lines
        {
            gchar** _tokens = g_strsplit ( g_strchomp( _command ), "|", 5);

            if ( _command[0] == '#' )
            {
                 _rc = process_comment_command( _command );
            }
            else if ( 0 == g_strcmp0( _tokens[0], "TEST" ) )
            {
                _rc = process_test_command( _tokens );
            }
            else if ( 0 == g_strcmp0( _tokens[0], "SEND" ) )
            {
                _rc = process_send_command( _tokens );
            }
            else if ( 0 == g_strcmp0( _tokens[0], "RECEIVE" ) )
            {
                _rc = process_receive_command( _tokens );
            }
            else
            {
                pLogger_->Error( g_strdup_printf( "Unknown script command '%s' at line %d", _tokens[0], TestCommand::nextNumber_ ) );
                _rc = FALSE;
            }

            g_strfreev( _tokens );
        }

        g_free( _command );
        _command = NULL;
    }

    if ( pCurrentTest_ )
    {
        pTests_->push_back( pCurrentTest_ );
        pCurrentTest_ = NULL;
    }

    return _rc;
}

gboolean ScriptParser::build_test( gchar** tokens )
{
    if ( 0 != g_strcmp0( tokens[0], "TEST" ) )
    {
        pLogger_->Error( g_strdup_printf( "Error: Expected a TEST command on line %d", TestCommand::nextNumber_ ) );
        return FALSE;
    }
    else if ( tokens[1] == NULL )
    {
        pLogger_->Error( g_strdup_printf( "Error: missing test name on line %d", TestCommand::nextNumber_ ) );
        return FALSE;
    }
    else
    {
        pCurrentTest_ =  new Test( tokens[1] );
    }
}

gboolean ScriptParser::process_test_command( gchar** tokens )
{
    if ( pCurrentTest_ )
    {
        pTests_->push_back( pCurrentTest_ );
        pCurrentTest_ = NULL;
    }
    readSend_ = FALSE;
    return build_test( tokens );
}

gboolean ScriptParser::process_comment_command( const gchar* comment )
{
    pCurrentTest_->push_back( new TestCommand( TestCommand::COMMENT, comment ) );
}

gboolean ScriptParser::process_send_command( gchar** tokens )
{
    if ( !pCurrentTest_ )
    {
        pLogger_->ErrorStr( "Error in script file, SEND must be preceded by a TEST command" );
        return FALSE;
    }
    readSend_ = TRUE;
    return build_command( TestCommand::SEND, tokens );
}

gboolean ScriptParser::process_receive_command( gchar** tokens )
{
    return build_command( TestCommand::RECEIVE, tokens );
}

gboolean ScriptParser::build_command( const TestCommand::Command command, gchar** tokens )
{
    if ( tokens[2] == NULL )
    {
         pLogger_->Error( g_strdup_printf( "Missing topic on line %d", TestCommand::nextNumber_ ) );
         return FALSE;
    }
    else
    {
        auto _pJson = JsonUtilities::try_parse( tokens[4] , pLogger_ );
        if ( _pJson == NULL )
        {
            pLogger_->Error( g_strdup_printf( "Unable to parse json on line %d", TestCommand::nextNumber_ ) );
            return FALSE;           
        }
    
        //
        std::string _json_str = tokens[4];
        if ( _json_str.find("%s") == std::string::npos  )
        {
           pCurrentTest_->push_back( new TestCommand( command, tokens[1], tokens[2], tokens[3], _pJson ) );
        }
        else
        {
           pCurrentTest_->push_back( new TestCommand( command, tokens[1], tokens[2], tokens[3], tokens[4] ) );          
        }
    }

    return TRUE;
}
