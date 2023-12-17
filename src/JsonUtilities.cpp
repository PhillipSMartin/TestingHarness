#include <iostream>
#include <iomanip>

#include "JsonUtilities.h"
#include "Logger.h"

namespace JsonUtilities
{
    nlohmann::json* try_parse( const gchar* text, Logger* pLogger )
    {
        if ( text != NULL )
        {
            try
            {
                return new nlohmann::json( nlohmann::json::parse( text ) );
            }
            catch( const std::exception& e )
            {
                pLogger->Error( g_strdup_printf( "Error parsing string %s", text ) );
                pLogger->ErrorStr( e.what() );
            }
        } 

        return NULL;        
    }

    gchar* pretty_format( nlohmann::json* pJson )
    {
        std::ostringstream _os;
        _os << std::setw(5) << *pJson << std::endl;
        return g_strdup( _os.str().c_str() );
    }

    void generate_map( std::map<std::string, std::string>& valuesByKey, nlohmann::json* pBranch, const std::string keyPath )
    {
        for ( auto& _it : pBranch->items() )
        {
            std::string _newKeyPath = keyPath.empty() ? _it.key() : keyPath + ":" + _it.key();           
            if ( _it.value().is_structured() )
            {
                if ( _it.value().size() > 0 )
                {
                    generate_map(valuesByKey, &_it.value(), _newKeyPath );
                }
            }
            else
            {
                valuesByKey[ _newKeyPath ] = nlohmann::to_string( _it.value() );
            }
        }
    }

    gboolean compare_jsons( nlohmann::json* pActual, nlohmann::json* pExpected, Logger* pLogger )
    {
        gboolean _match = TRUE;

        std::map<std::string, std::string> _mapActual;
        std::map<std::string, std::string> _mapExpected;

        try
        {
            generate_map( _mapActual, pActual );
        }
        catch( const std::exception& e)
        {
            pLogger->Error( g_strdup_printf( "Exception creating map for actual message: %s", e.what()));
            pLogger->ErrorStr( nlohmann::to_string( *pActual ) );
            return FALSE;
        }
        try
        {
            generate_map( _mapExpected, pExpected );
        }
        catch( const std::exception& e)
        {
            pLogger->Error( g_strdup_printf( "Exception creating map for expected message: %s\n message=%s", e.what()));
            pLogger->ErrorStr( nlohmann::to_string( *pExpected ) );
            return FALSE;     
        }

        for ( auto& _itemExpected : _mapExpected )
        {
            auto _itActual = _mapActual.find( _itemExpected.first );
            if ( _itActual == _mapActual.end() )
            {
                _match = FALSE;
                if ( pLogger )
                {
                    pLogger->Info( g_strdup_printf( "Key = %s, Expected = %s, Actual = <None>", _itemExpected.first.c_str(), _itemExpected.second.c_str() ) );
                }
            }
            else
            {
                if ( _itemExpected.second != _itActual->second )
                {
                    _match = FALSE;
                    if ( pLogger )
                    {
                        pLogger->Info( g_strdup_printf( "Key = %s, Expected = %s, Actual = %s", _itemExpected.first.c_str(), _itemExpected.second.c_str(), _itActual->second.c_str() ) );
                    }
                }
            }
        }

        return _match;
    }
}