#include "AMQDestination.h"
#include "Logger.h"

AMQDestination::AMQDestination(ActiveMQSession* const& pSession, 
        cms::Destination* pDestination,
        DestinationType destinationType,
        bool synchronous,
        Logger* pLogger) : 
    pSession_(pSession),
    pDestination_(pDestination),
    destinationType_(destinationType),
    synchronous_(synchronous),
    pLogger_(pLogger) 
{
}

void AMQDestination::stop() 
{ 
    active_ = false;
    if (pProducer_ != nullptr) 
    {
        delete pProducer_; 
    }
    if (pConsumer_ != nullptr) 
    {
        delete pConsumer_; 
    }
    delete pDestination_; 
}

void AMQDestination::send_text_message( std::string const& textMessage ) 
{
    auto _textMessage = pSession_->createTextMessage();
    _textMessage->setText( textMessage );
    get_producer_ptr()->send( _textMessage );

    pLogger_->Info( g_strdup_printf( "[%s] Message sent: %s", get_destination_name().c_str(), textMessage.c_str() ) );

    delete _textMessage;
}

void AMQDestination::subscribe( AMQListener* listener)
{
    listener_ = listener;
    if ( pConsumer_ == nullptr )
    {
        pConsumer_ = pSession_->createConsumer( pDestination_ );
        if ( !synchronous_ )
        {
            pConsumer_->setMessageListener( static_cast<MessageListener*>(this) );
        }
        
        pLogger_->Info( g_strdup_printf( "Subscribed to [%s]", get_destination_name().c_str() ) );
    }
    else
    {
        pLogger_->Info( g_strdup_printf( "Already subscribed to [%s]", get_destination_name().c_str() ) );
    }
}

cms::MessageProducer* AMQDestination::get_producer_ptr()
{
    if ( pProducer_ == nullptr )
    {
        pProducer_ = pSession_->createProducer( pDestination_ );
        pProducer_->setDeliveryMode( DeliveryMode::NON_PERSISTENT );

        pLogger_->Info( g_strdup_printf( "Created producer for [%s]", get_destination_name().c_str() ) );
    }

    return pProducer_;
}

bool AMQDestination::receive_message()
{
    if ( pConsumer_ )
    {
        auto message( pConsumer_->receiveNoWait() ); 
        if ( message )
        {
            onMessage( message );
            return true;
        }
    }

    return false;
}

void AMQDestination::onMessage( const Message* pMessage ) 
{   
    try        
    {   if ( !active_ )
            return;
            
        messageCount_++;

        const TextMessage* _pTextMessage = static_cast< const TextMessage* >( pMessage );
        if( _pTextMessage != NULL ) 
        {  
            pLogger_->Debug( g_strdup_printf( "[%s] Message #%d received:", get_destination_name().c_str(), messageCount_ ) );
            pLogger_->DebugStr( _pTextMessage->getText().c_str() );
       } 
        else 
        {                
            pLogger_->Error( g_strdup_printf( "[%s] Message #%d is not a text message", get_destination_name().c_str(), messageCount_ ) );          
        }
        
        pMessage->acknowledge(); 

        if ( listener_ )
        {
            listener_->OnAMQMessage( get_destination_id(), _pTextMessage->getText().c_str() );
        } 
    }
    catch (CMSException& e) 
    {
        pLogger_->Error( g_strdup_printf( "[%s] Error reading message #%d: %s", get_destination_name().c_str(), messageCount_, e.what() ) );
    } 
 
}