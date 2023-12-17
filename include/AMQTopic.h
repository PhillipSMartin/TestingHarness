#pragma once

#include "AMQDestination.h"

class AMQTopic : public AMQDestination
{
    public:
        AMQTopic(std::string const& topicName,
            ActiveMQSession* const& pSession,
            bool synchronous,
            Logger* pLogger);

        virtual std::string get_destination_name() const 
        { 
            return static_cast<cms::Topic*>(pDestination_)->getTopicName(); 
        }
};