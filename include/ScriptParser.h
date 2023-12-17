#pragma once

#include <set>
#include "TestCommand.h"

class Logger;
class Parameters;
class Test;
class Tests;
class TestExecutor;

class ScriptParser
{
    private:
        Logger* pLogger_;
        Parameters* pParms_;

        Test* pCurrentTest_  = NULL;
        Tests* pTests_ = NULL;
        gboolean readSend_ = FALSE;

        void cleanup_from_previous_test();

        gboolean build_tests( const char* fileName, GIOChannel* pChannel );
        gboolean build_test( gchar** tokens );
        gboolean build_command( const TestCommand::Command command, gchar** tokens );

        gboolean process_test_command( gchar** tokens );
        gboolean process_send_command( gchar** tokens );
        gboolean process_receive_command( gchar** tokens );
        gboolean process_comment_command( const gchar* comment );

    public:
        ScriptParser( Parameters* pParms, Logger* pLogger ) : pParms_ { pParms }, pLogger_{ pLogger } {}
        ~ScriptParser() 
        { 
            cleanup_from_previous_test();
        }
        
        gboolean run_script( TestExecutor* pExecutor );
};