#pragma once

#include <string>
#include "gtk/gtk.h"

enum DestinationType {
    NONE = -1,
    TOPIC = 0,
    QUEUE,
    NUM_DESTINATION_TYPES
};

struct DestinationId
{
    std::string destinationName_ ;
    DestinationType destinationType_;

    DestinationId(std::string destinationName = "", DestinationType destinationType = DestinationType::NONE) : destinationName_( destinationName ), destinationType_( destinationType ) {}
    DestinationId( const DestinationId& id ) 
    {
        destinationName_ = id.destinationName_;
        destinationType_ = id.destinationType_;
    }
    DestinationId( const DestinationId& id, const gchar* tourneyId, const gchar* destinationNameTemplate )
    {
        destinationName_ = build_destination_name( get_destination_prefix( id.destinationName_ ).c_str(), 
            tourneyId,
            get_table_number( id.destinationName_ ), 
            destinationNameTemplate);
        destinationType_ = id.destinationType_;
    }

    bool operator ==( const DestinationId& id )
    {
        return ( destinationName_ == id.destinationName_ ) && ( destinationType_ == id.destinationType_ );
    }

    bool operator !=( const DestinationId& id )
    {
        return !( *this == id );
    }

    const gchar* get_destination_name() const { return destinationName_.c_str(); }
    const gchar* get_destination_type() const; 

    gboolean isValid() const { return destinationType_ != DestinationType::NONE; } 
    std::string to_string() const;

    static std::string build_destination_name( const gchar* prefix, const gchar* tourneyId, 
        gint tableNumber, const gchar* destinationNameTemplate );
    static std::string get_destination_prefix( std::string destinationName );
    static gint get_table_number( std::string destinationName );
};