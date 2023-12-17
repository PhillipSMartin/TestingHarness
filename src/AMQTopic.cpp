#include "AMQTopic.h"
#include "Logger.h"

AMQTopic::AMQTopic(std::string const& topicName, 
        ActiveMQSession* const& pSession, 
        bool synchronous,
        Logger* pLogger) : 
    AMQDestination(pSession, pSession->createTopic(topicName), TOPIC, synchronous, pLogger)
{
    pLogger_->Info( g_strdup_printf( "Created topic [%s]", get_destination_name().c_str() ) );
}