#pragma once

#include <string>
#include <utility>
#include <vector>

#include "DestinationId.h"
#include "FileUtilities.h"

class Logger;

// a destination map entry consists of a key and a DestinationMapItem
//
// the DestinationMapItem consists of a DestinationType (0 for QUEUE, 1 for TOPIC)
//    and and a destination name template
// the template is used to construct the destinatation name and is passed the parameters
//    destination key, tournament name, table number
// so if the template is %s=%s-%d, the key is Event.tb, the tournament name is lh-test,
//    and the table number is 1, the name will be Event.tb=lhtest-1
// the template may not use all the parameters. In fact, it may be simply %s, in which case
//    the key and name will be the same
// the Map entries are loaded from a pipe-delimited file, eg:
//
//  Event.tb|1|%s=%s-%d|
//  Event.tm|1|%s=%s-0-0|
//  State.tb|0|%s=%s-%d|
//  State.tm|0|%s=%s-0-0|
//

typedef std::pair<DestinationType, std::string> DestinationMapItem;

class DestinationMap : public std::vector<std::pair<std::string, DestinationMapItem>>
{
    private:
        Logger* pLogger_;
        gboolean read_file( GIOChannel* pChannel, const gchar* fileName );
        gboolean add_item( const gchar* line );

    public:
        DestinationMap(Logger* pLogger) : pLogger_{pLogger} {}
        gboolean import_destinations( const gchar* fileName );
        DestinationType get_destination_type( const gchar* destinatiionName ) const;
        gchar* get_destination_name_template( const gchar* destinationName ) const; // caller owns string
};