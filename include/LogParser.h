#pragma once

#include "DestinationId.h"
#include "nlohmann/json.hpp"

class DestinationMap;
class Logger;
class Test;

class LogParser
{
    private:
        const gchar* startToken_ = "TextMessage [";
        const gchar* endToken_ = "] received";

        Logger* pLogger_;
        DestinationMap* pDestinationMap_;

        gint lineNo_;
        gint commandsImported_;
        gint errors_;

        gboolean read_line( GIOChannel*, Test*, gboolean* pEOF );
        gboolean parse_line( GIOChannel*, Test*, const gchar* line );
        nlohmann::json* get_json( GIOChannel*, const gchar* fileName );

        gboolean get_destinationId( const gchar* line, DestinationId& ) const;
 
    public:
        LogParser( Logger* pLogger, DestinationMap* pDestinationMap ) : pLogger_{ pLogger } , pDestinationMap_{ pDestinationMap } {}      
        Test* parse_log_file( const gchar* logFileName );
};