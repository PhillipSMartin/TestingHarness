#include "MessageStore.h"
#include "Logger.h"

void MessageStore::add_message( const gchar* message )
{
    pLogger_->Debug( g_strdup_printf( "Adding message %d to message store for %s", 
        vector_.size() + 1, destinationId_.to_string().c_str() )  );
    vector_.push_back( g_strdup( message ) );

}

const gchar* MessageStore::get_current_message() const
{
    return ( get_current_item_number() == 0) ? NULL : vector_.at( get_current_item_number() - 1 );
}