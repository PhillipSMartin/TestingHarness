#include "AMQQueue.h"
#include "Logger.h"

AMQQueue::AMQQueue(std::string const& queueName, 
        ActiveMQSession* const& pSession, 
        bool synchronous,
        Logger* pLogger) : 
    AMQDestination(pSession, pSession->createQueue(queueName), QUEUE, synchronous, pLogger)
{
    pLogger_->Info( g_strdup_printf( "Created queue [%s]", get_destination_name().c_str() ) );
}