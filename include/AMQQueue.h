#pragma once

#include "AMQDestination.h"

class AMQQueue : public AMQDestination
{
    public:
        AMQQueue(std::string const& queueName,
            ActiveMQSession* const& pSession,
            bool synchronous,
            Logger* pLogger);

        virtual std::string get_destination_name() const 
        { 
            return static_cast<cms::Queue*>(pDestination_)->getQueueName(); 
        }
};