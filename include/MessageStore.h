#pragma once

#include <vector>

#include "DestinationId.h"
#include "NavigationClient.h"

class Logger;

class MessageStore : public NavigationClient
{
    private:
        Logger* pLogger_;
        DestinationId destinationId_;
        std::vector<const gchar*> vector_;

    public:
        MessageStore( DestinationId const& destinationId , Logger* pLogger ) : pLogger_{pLogger}, destinationId_{destinationId} {}

        void add_message( const gchar* message ); 
        const gchar* get_current_message() const;

        virtual gint get_max_item_number() const { return vector_.size(); }
};
