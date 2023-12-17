#include "CLI.h"
#include "JsonUtilities.h"
#include "ScriptParser.h"
#include "Test.h"
#include "TestManager.h"

class ExecutionStub : public TestExecutor
{

public:
    Logger* pLogger_;
    gboolean ranFirstTest_ = FALSE;
    nlohmann::json* pJson1 = NULL;
    nlohmann::json* pJson2 = NULL;

    ExecutionStub( Logger* pLogger ) : pLogger_{ pLogger } {}

    virtual gboolean run_test( Test* test )
    {
        if ( !ranFirstTest_ )
        {
            ranFirstTest_ = TRUE;
            pJson1 = (*test)[0]->pJson_;
            pJson2 = (*test)[1]->pJson_;
        }
        else
        {
            if ( JsonUtilities::compare_jsons( pJson1, (*test)[0]->pJson_, pLogger_ ) )
            {
                pLogger_->InfoStr( "Json objects 1 and 3 match" );
            }
            else
            {
                 pLogger_->InfoStr( "Json objects 1 and 3 don't match" );             
            }

            if ( JsonUtilities::compare_jsons( pJson2, (*test)[1]->pJson_, pLogger_ ) )
            {
                pLogger_->InfoStr( "Json objects 2 and 4 match" );
            }
            else
            {
                pLogger_->InfoStr( "Json objects 2 and 4 don't match" );   
           
            }        
        }
    }
};

void CLI::run()
{
    TestManager _testManager{ pAMQClient_, pLogger_ };
//  ExecutionStub _testManager{ pLogger_ }; // for testing
    ScriptParser _parser{ pParms_, pLogger_ };
    _parser.run_script( &_testManager );
}


