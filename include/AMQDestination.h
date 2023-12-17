#pragma once

#include "activemq.h"

class Logger;

class AMQDestination : public MessageListener 
{
    protected:
        cms::Destination* pDestination_;
        Logger* pLogger_;
        DestinationType destinationType_;

    private:
        ActiveMQSession* pSession_; // AMQClient is responsible for deletion
        cms::MessageProducer* pProducer_ = nullptr;
        cms::MessageConsumer* pConsumer_ = nullptr;
        int messageCount_  = 0;
        AMQListener* listener_ = nullptr;
        bool synchronous_;
        bool active_{ true };

    public:
        AMQDestination(ActiveMQSession* const& pSession,
            cms::Destination* pDestination,
            DestinationType destinationType,
            bool synchronous,
            Logger* pLogger);

        virtual std::string get_destination_name() const  = 0;
        DestinationId get_destination_id() const { return DestinationId{ get_destination_name(), destinationType_ }; }

        void send_text_message( std::string const& textMessage );
        void subscribe( AMQListener* listener );
        void stop();

        bool receive_message(); // returns true if we received a message
        virtual void onMessage( const Message* pMessage );
        
    private:
        cms::MessageProducer* get_producer_ptr();
};