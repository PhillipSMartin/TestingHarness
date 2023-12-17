#pragma once

#include <gtk/gtk.h>

#include "DestinationId.h"
#include "nlohmann/json.hpp"

struct TestCommand
{
    public:
        enum Command { 
            SEND,    
            RECEIVE,
            COMMENT
        };

    Command command_;
    gchar* comment_ = NULL;
    gchar* json_str_ = NULL;
    
    DestinationId destinationId_;
    nlohmann::json* pJson_ = NULL;
    gint number_;
    static gint nextNumber_;

    TestCommand( Command command, const gchar* comment, const gchar* destinationType, const gchar* destinationName, nlohmann::json* pJson ) : 
        command_{ command }, comment_{ g_strdup(comment) }, destinationId_{ destinationName, parse_destination_type( destinationType ) }, pJson_{ pJson }, number_{ nextNumber_ } {}
    TestCommand( Command command, const gchar* comment, const gchar* destinationType, const gchar* destinationName, gchar* jsonStr ) : 
        command_{ command }, comment_{ g_strdup(comment) }, destinationId_{ destinationName, parse_destination_type( destinationType ) }, json_str_{ g_strdup(jsonStr) }, number_{ nextNumber_ } {}
    TestCommand( Command command, const gchar* comment, DestinationId destinationId, nlohmann::json* pJson ) : 
        command_{ command }, comment_{ g_strdup(comment) }, destinationId_ { destinationId }, pJson_{ pJson }, number_{ nextNumber_ } {}
    TestCommand( Command command, const gchar* comment ) : command_{ command }, comment_{ g_strdup(comment) }, number_{ nextNumber_ } {}

    ~TestCommand();

    static DestinationType parse_destination_type( const gchar* destinationType );
    static uint bump_number() { return ++nextNumber_; }

    std::string get_json_string() const { return (pJson_ == NULL) ? "" : nlohmann::to_string( *pJson_ ); } ;
    const gchar* get_destination_name() const { return destinationId_.destinationName_.c_str(); }

    bool destination_name_is_incomplete() const;
    void update_destination_name( const gchar* tourneyId );
    bool json_is_incomplete() const;
    bool update_json( const gchar* tourneyId, Logger* pLogger );
};