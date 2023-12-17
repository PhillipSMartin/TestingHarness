#include "DestinationId.h"

const gchar* DestinationId::get_destination_type() const 
{
    switch ( destinationType_ )
    {
        case DestinationType::TOPIC:
            return "topic";
        case DestinationType::QUEUE:
            return "queue";
        default:
            return "<invalid destination type>";
    }
}

std::string DestinationId::to_string() const
{
    gchar* s = g_strdup_printf( "%s [%s]", get_destination_type(), get_destination_name() );
    std::string str{ s };
    g_free( s );
    return str;
}

std::string DestinationId::build_destination_name( const gchar* prefix, const gchar* tourneyId, 
    gint tableNumber, const gchar* destinationNameTemplate  )
{
    std::string _r;
    if ( prefix )
    {
        gchar* _destinationName = g_strdup_printf( destinationNameTemplate, prefix, tourneyId, tableNumber );
        _r = _destinationName;
        g_free( _destinationName );
    }
    else
    {
        _r = "";
    }
    return _r;
}

std::string DestinationId::get_destination_prefix( std::string destinationName )
{
    size_t pos = destinationName.find('=');
    if ( pos != std::string::npos )
    {
        return destinationName.substr( 0, pos );
    }
    else
    {
        return destinationName;
    }
}

gint DestinationId::get_table_number( std::string destinationName )
{
    size_t pos = destinationName.find_last_of( '-' );
    if ( pos != std::string::npos )
    {
        return atoi( destinationName.substr( pos+1 ).c_str() );
    }
    else
    {
        return 0;
    }
}