#include "JsonUtilities.h"
#include "TestCommand.h"

TestCommand::~TestCommand()
{
    if ( pJson_ != NULL )
    {
        delete pJson_;
    }
    if ( comment_ != NULL )
    {
        g_free( comment_ );
    }
    if ( json_str_ != NULL )
    {
        g_free( json_str_ );
    }}

DestinationType TestCommand::parse_destination_type( const gchar* destinationType )
{
    if ( g_strcmp0(destinationType, "TOPIC" ) == 0 ) 
    {
        return DestinationType::TOPIC;
    }
    else if ( g_strcmp0(destinationType, "QUEUE" ) == 0 ) 
    {
        return DestinationType::QUEUE;
    }
    else 
    {
        return DestinationType::NONE;
    }
}

bool TestCommand::destination_name_is_incomplete() const
{
    return destinationId_.destinationName_.find("%s") != std::string::npos;
}
void TestCommand::update_destination_name( const gchar* tourneyId )
{
    gchar* _newName = g_strdup_printf( destinationId_.destinationName_.c_str(), tourneyId );
    destinationId_.destinationName_ = _newName;
    g_free( _newName );
}

bool TestCommand::json_is_incomplete() const
{
    return (pJson_ == NULL) && (json_str_ != NULL);
}
bool TestCommand::update_json( const gchar* tourneyId, Logger* pLogger )
{
    gchar* _new_json_str = g_strdup_printf( json_str_, tourneyId );
    pJson_ = JsonUtilities::try_parse( _new_json_str, pLogger );
    g_free( _new_json_str );
    return (pJson_ != NULL);
}