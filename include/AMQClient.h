#pragma once

#include <set>
#include <unordered_map>

#include "activemq.h"
class AMQDestination;
class Logger;

class AMQClient : public ExceptionListener, 
    public DefaultTransportListener
{
    private:
        ActiveMQConnection* pConnection_ = NULL;
        ActiveMQSession* pSession_ = NULL;
        bool started_ = false;
        bool synchronous_;

        Logger* pLogger_;
        std::unordered_map<std::string, AMQDestination*> destinations_;
        std::set<std::string> subscribedDestinations_;
    
    public:
        AMQClient(Logger* pLogger, gboolean synchronous) : pLogger_(pLogger), synchronous_(synchronous) {}

        gboolean start( const char* url = NULL );
        void stop();
        void send_text_message( DestinationId const& destinationId, std::string const& textMessage );
        void subscribe( DestinationId const& destinationId, AMQListener* listener );
        void receive_messages();

        virtual void onException( const CMSException& e );
        virtual void transportInterrupted(); 
        virtual void transportResumed(); 

    private:
        AMQDestination* get_destination_ptr( DestinationId const& destinationId );
};