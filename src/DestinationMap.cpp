#include <sstream>
#include "DestinationMap.h"
#include "Logger.h"

gboolean DestinationMap::import_destinations( const gchar* fileName )
{
    gboolean _rc = FALSE;
    GIOChannel* _pChannel;
    if ( FileUtilities::file_open( fileName, &_pChannel, "r", pLogger_ ) )
    {
 
        _rc = read_file( _pChannel , fileName);
        FileUtilities::file_close( fileName, _pChannel, FALSE, pLogger_ );
    }

    return _rc;   
}

gboolean DestinationMap::read_file( GIOChannel* pChannel, const gchar* fileName )
{

    gchar* _line = NULL;
    gint _lineNo = 0;
    gboolean _done = FALSE;

    // returns FALSE on an error, TRUE if line is read or on EOF, sets _done to TRUE on EOF
    gboolean _rc = FileUtilities::file_read_line( fileName, pChannel, &_line,  &_done, pLogger_ );
    while ( _rc && !_done )
    {  
        _lineNo++; 
        _rc = add_item( _line );
        if ( !_rc )
        {
            pLogger_->Error( g_strdup_printf("Format error in %s, line %d: %s", fileName, _lineNo, _line ) );
        }
        else
        {
            g_free( _line );
            _line = NULL;

            _rc = FileUtilities::file_read_line( fileName, pChannel, &_line,  &_done, pLogger_ );
        }
     } 

    g_free( _line );
    return _rc;
}

gboolean DestinationMap::add_item( const gchar* line )
{
    std::string _destinationName, _destinationType, _destinationNameTemplate;
    gint _destinationTypeInt;
    std::stringstream _sstream{ line };

    if ( !std::getline( _sstream, _destinationName, '|' ) )
    {
        return FALSE;
    }

    std::getline( _sstream, _destinationType, '|' );
    _destinationTypeInt = atoi( _destinationType.c_str() );
    if ( _destinationTypeInt >= NUM_DESTINATION_TYPES )
    {
        return FALSE;
    }

    if ( !std::getline( _sstream, _destinationNameTemplate, '|' ) )
    {
        return FALSE;
    }

    push_back( { _destinationName, { static_cast<DestinationType>(_destinationTypeInt ), _destinationNameTemplate } } );

    return TRUE;
}

DestinationType DestinationMap::get_destination_type( const char* destinationName ) const
{
    for ( auto& _item : *this )
    {
        if ( g_str_has_prefix( destinationName, _item.first.c_str() ) ) 
        {
            return _item.second.first;
        }
    }
    return DestinationType::NONE;
}


gchar* DestinationMap::get_destination_name_template( const char* destinationName ) const
{
    for ( auto& _item : *this )
    {
        if ( g_str_has_prefix( destinationName, _item.first.c_str() ) ) 
        {
            return g_strdup( _item.second.second.c_str() );
        }
    }
    return g_strdup( "" );
}