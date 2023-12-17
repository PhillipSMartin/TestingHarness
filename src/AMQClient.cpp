#include "AMQClient.h"
#include "AMQQueue.h"
#include "AMQTopic.h"
#include "Logger.h"

void AMQClient::send_text_message( DestinationId const& destinationId, std::string const& textMessage )
{
    if (started_)
    {
        get_destination_ptr( destinationId )->send_text_message( textMessage );
    }
    else
    {
        pLogger_->ErrorStr( "Cannot send text message before client is started" );
    }
}

void AMQClient::subscribe( DestinationId const& destinationId, AMQListener* listener )
{
    if ( subscribedDestinations_.find( destinationId.destinationName_ ) == subscribedDestinations_.end() )
    {
        get_destination_ptr( destinationId )->subscribe( listener );
        subscribedDestinations_.insert( destinationId.destinationName_ );
    }
}

void AMQClient::receive_messages()
{
    for (auto _it : destinations_ )
    {
        while ( _it.second->receive_message() ) { }
    }
}

AMQDestination* AMQClient::get_destination_ptr( DestinationId const& destinationId )
{
 
    auto _it = destinations_.find( destinationId.destinationName_ );
    if ( _it != destinations_.end() )
    {
        return _it->second;
    }
    else
    {
        switch ( destinationId.destinationType_ )
        {
            case TOPIC:
                destinations_.emplace( destinationId.destinationName_ ,  new AMQTopic(destinationId.destinationName_, pSession_, synchronous_, pLogger_) );
                break;
            case QUEUE:
                destinations_.emplace( destinationId.destinationName_ ,  new AMQQueue(destinationId.destinationName_, pSession_, synchronous_, pLogger_) );
                break;
        }
        return destinations_[destinationId.destinationName_];
    }
}

gboolean AMQClient::start( const char* url )
{
    ActiveMQConnectionFactory* _pConnectionFactory = NULL;

    try
    {
        // initialize the activeMQ Library
        activemq::library::ActiveMQCPP::initializeLibrary();
    
        // create a connection
        _pConnectionFactory = new ActiveMQConnectionFactory( (url == NULL) ? "tcp://localhost:61616" : url );
        pConnection_ = _pConnectionFactory->createConnection();
        delete _pConnectionFactory;
        _pConnectionFactory = NULL;

        pConnection_->addTransportListener(static_cast<TransportListener*>(this));
        pConnection_->setExceptionListener(static_cast<ExceptionListener*>(this));

        // create a session
        pSession_ = pConnection_->createSession( cms::Session::AUTO_ACKNOWLEDGE );
        pLogger_->DebugStr( "Created ActiveMQ session" );        
        
        pConnection_->start();
        pLogger_->InfoStr( "ActiveMQ connection started" );
        started_ = true;
    }
    catch( const std::exception& e )
    {
        pLogger_->Error( g_strdup_printf( "Unable to start AMQClient: %s", e.what() ) );
        if ( _pConnectionFactory )
        {
            delete _pConnectionFactory;
        }
        stop();
    }

    return started_;
}

void AMQClient::stop()
{
    gboolean _wasStarted = started_;
    started_ = false;

    for ( auto _topic : destinations_ )
    {
        _topic.second->stop();
        delete _topic.second;
    }
    destinations_.clear();
    if ( pSession_ )
    {
        delete pSession_;
    }
    if ( pConnection_ )
    {
        delete pConnection_;
    }
    try
    {
       activemq::library::ActiveMQCPP::shutdownLibrary();
    }
    catch(const std::exception& e)
    {
    }   

    if ( _wasStarted )  // message is confusing if ActiveMQ never started
    {
        pLogger_->InfoStr( "ActiveMQ connection stopped" );
    }
}

void AMQClient::onException( const CMSException& e ) 
{     
    pLogger_->Fatal( g_strdup_printf( "Shutting down client. CMS Exception occurred: %s", e.what() ) );      
    exit( 1 );    
}

void AMQClient::transportInterrupted() 
{        
    pLogger_->InfoStr( "The connection's transport has been interrupted" );    
}

void AMQClient::transportResumed() 
{        
    pLogger_->InfoStr( "The connection's transport has been restarted" );    
}
