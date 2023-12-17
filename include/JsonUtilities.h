#pragma once

#include <map>
#include <set>
#include <string>

#include "gtk/gtk.h"
#include "nlohmann/json.hpp"

class Logger;

namespace JsonUtilities
{
    nlohmann::json* try_parse( const gchar*, Logger* );
    gchar* pretty_format( nlohmann::json* pJson );
    void generate_map( std::map<std::string, std::string>& valuesByKey, nlohmann::json* pBranch, const std::string keyPath = "" );
    gboolean compare_jsons( nlohmann::json* pActual, nlohmann::json* pExpected, Logger* pLogger );
}