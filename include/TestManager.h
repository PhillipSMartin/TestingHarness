#pragma once

#include <mutex>

#include "AMQClient.h"
#include "JsonUtilities.h"
#include "Logger.h"
#include "TestExecutor.h"

class TestCommand;

class TestManager : public TestExecutor, public AMQListener
{
    private:
        AMQClient* pAMQClient_;
        Logger* pLogger_;

        static std::mutex mutex_;
        std::map<std::string, std::vector<nlohmann::json*>> incomingMessages_;  // messages received, partitioned by destination

        gboolean success_ = TRUE;
        gboolean awaitingLaunchReply = FALSE;

        std::string tourneyId;

        void subscribe( Test* test );
        gboolean wait_for_incoming_message( const TestCommand* pCommand );  
        gboolean verify_incoming_message( const TestCommand* pCommand ); 
        gboolean find_tourney_id( nlohmann::json* pJson );
        // replace placeholders in commands with tourney id
        void inject_tourney_id( Test* test );
        // returns true if destination wass added to incomingMessages_ map; false if already present
        gboolean add_destination_if_missing(std::string destinationName);
        void clear();   

    public:
        TestManager( AMQClient* pAMQClient, Logger* pLogger ) : pAMQClient_{ pAMQClient }, pLogger_{ pLogger } {}
        virtual gboolean run_test( Test* test );
        virtual void OnAMQMessage( const DestinationId& destinationId, const char* message );
};