#include "TestManager.h"

#include <chrono>
#include <thread>

#include "Test.h"
#include "TestCommand.h"

std::mutex TestManager::mutex_;

void TestManager::subscribe( Test* test )
{
    for ( const TestCommand* pCommand : *test )
    {
        if ( pCommand->command_ == TestCommand::RECEIVE )
        {
            if ( pCommand->destination_name_is_incomplete() )
            {
                awaitingLaunchReply = TRUE;
            }
            else
            {
                if (add_destination_if_missing( pCommand->destinationId_.destinationName_ ) )
                {
                    pAMQClient_->subscribe( pCommand->destinationId_, static_cast<AMQListener *>( this ) );
                }
            }
        }
    }
}

gboolean TestManager::add_destination_if_missing(std::string destinationName)
{
    gboolean _added = FALSE;

    mutex_.lock();  
    if ( incomingMessages_.find( destinationName ) == incomingMessages_.end() )
    {
        _added = TRUE;
        incomingMessages_[ destinationName ] = std::vector<nlohmann::json*>{};        
    }
    mutex_.unlock();

    return _added;
}


void TestManager::clear()
{
    mutex_.lock();
    for ( auto& _item : incomingMessages_ )
    {
        for ( auto& _msg : _item.second )
        {
            g_free( _msg );
        }
        _item.second.clear();
    }
    incomingMessages_.clear();
    mutex_.unlock();
}

void TestManager::OnAMQMessage( const DestinationId& destinationId, const char* message )
{
    auto _json = JsonUtilities::try_parse( message, pLogger_ );
    if (_json)
    {
        mutex_.lock();
        if ( incomingMessages_.find( destinationId.destinationName_ ) != incomingMessages_.end() )
        {
            incomingMessages_[ destinationId.destinationName_ ].push_back( _json );

        }
        mutex_.unlock();
    }
 }

gboolean TestManager::wait_for_incoming_message( const TestCommand* pCommand )
{
    auto _it = incomingMessages_.find( pCommand->destinationId_.destinationName_ );
    if ( _it == incomingMessages_.end() )
    {
        pLogger_->Fatal( g_strdup_printf("Not subscribed to %s [%s] (%d)", pCommand->destinationId_.get_destination_type(), pCommand->destinationId_.get_destination_name(), pCommand->number_ ) );
        return FALSE;
    }
    
    gboolean _foundMsg{ FALSE };
    gchar* _waitMsg = 
        g_strdup_printf("WAITING for %s [%s] (%d)", pCommand->comment_, pCommand->get_destination_name(), pCommand->number_ );
    while (1)
    {
        gint _count(0);
        pLogger_->InfoStr( _waitMsg );
        std::this_thread::sleep_for( std::chrono::seconds(30) );
 
        if (_it->second.size() > 0)
        {
            _foundMsg = TRUE;
            break;
        }
    }

    g_free( _waitMsg );

    if (_foundMsg)
    {
        pLogger_->Info( 
            g_strdup_printf("RECEIVED %s [%s] (%d)", pCommand->comment_, pCommand->get_destination_name(), pCommand->number_ ));
    }
    return _foundMsg;
}

gboolean TestManager::verify_incoming_message( const TestCommand* pCommand )
{
    gboolean _success{ TRUE };

    // wait_for_incoming_message has already verified that the vector exists and has at least one element
    auto _it = incomingMessages_.find( pCommand->destinationId_.destinationName_ );  

    mutex_.lock();
    auto _pActual = _it->second[0];
    _it->second.erase( _it->second.begin() );
    mutex_.unlock();
    
    auto _pExpected = pCommand->pJson_ ;
    
    if (_pActual != NULL)
    {
        if ( JsonUtilities::compare_jsons(_pActual, _pExpected, pLogger_ ) )
        {
            pLogger_->Info( g_strdup_printf("SUCCESS %s [%s] (%d): %s", pCommand->comment_, pCommand->get_destination_name(), pCommand->number_, pCommand->get_json_string().c_str(), pCommand->number_) );
            if (awaitingLaunchReply)
            {
                _success = find_tourney_id( _pActual );
            }
        }  
        else
        {
            _success = FALSE;
            pLogger_->Info( g_strdup_printf("FAILURE %s [%s] (%d): %s", pCommand->comment_, pCommand->get_destination_name(), pCommand->number_, pCommand->get_json_string().c_str(),  pCommand->number_ ) );
        } 
 
        g_free( _pActual );
    }
    else
    {
        _success = FALSE;
        pLogger_->Info( g_strdup_printf("FAILURE %s [%s] (%d): Incoming message was NULL", pCommand->comment_, pCommand->get_destination_name(), pCommand->number_) );       
    }

    return _success;
}

gboolean TestManager::find_tourney_id( nlohmann::json* pJson )
{
    if ( pJson->find( "tourney" ) != pJson->end() )
    {
        if ( pJson->at( "tourney" ).find( "trnyid" ) != pJson->at( "tourney" ).end())
        {
            tourneyId = pJson->at( "tourney" )[ "trnyid" ];
            return TRUE;
        }
    }

    pLogger_->InfoStr( "FAILURE - No trnyid in launch reply" );
    return FALSE;
}

void TestManager::inject_tourney_id( Test* test )
{
    for ( TestCommand* _pCommand : *test )
    {
        if (_pCommand->destination_name_is_incomplete())
        {
            _pCommand->update_destination_name( tourneyId.c_str() );
        }
        if (_pCommand->json_is_incomplete())
        {
            _pCommand->update_json( tourneyId.c_str(), pLogger_ );
        }
    }
}

gboolean TestManager::run_test( Test* test )
{
    clear();
    success_ = TRUE;

    subscribe( test );

    pLogger_->InfoStr( "***");
    pLogger_->Info( g_strdup_printf( "*** Test %s begun", test->testName_ ) );
    pLogger_->InfoStr( "***");

    for ( TestCommand* _pCommand : *test )
    {
        switch( _pCommand->command_ )
        {
            case TestCommand::COMMENT:
                pLogger_->Info( g_strdup_printf( "%s (%d)", _pCommand->comment_, _pCommand->number_) );
                break;

            case TestCommand::SEND:
                pAMQClient_->send_text_message( _pCommand->destinationId_, _pCommand->get_json_string() );
                pLogger_->Info( g_strdup_printf( "SENT %s [%s] (%d)", _pCommand->comment_, _pCommand->get_destination_name(), _pCommand->number_ ) );
                break;

            case TestCommand::RECEIVE:
                if ( wait_for_incoming_message( _pCommand ) )
                {
                    success_ = verify_incoming_message( _pCommand );
                    if (success_ && awaitingLaunchReply)
                    {
                        awaitingLaunchReply = FALSE;
                        inject_tourney_id( test );
                        subscribe( test );
                    }
                }
                else
                {
                    success_ = FALSE;
                }
                break;
        }    
        if (!success_ )
        {
            break;
        }
    } 

    return success_;
}