#include "FileUtilities.h"
#include "Logger.h"

namespace FileUtilities
{
    // returns TRUE if there was an error, FALSE if not
    gboolean static handle_error( const gchar* errorMsg, GError* pError, Logger* pLogger ); 

    static gboolean handle_error( const gchar* errorMsg, GError* pError, Logger* pLogger ) 
    {
        if ( pError != NULL )
        {
            pLogger->Error( g_strdup_printf( "%s: %s", errorMsg, pError->message ) );
            g_clear_error( &pError );
            return TRUE;
        }

        return FALSE;
    }

    gboolean file_open( const gchar* fileName, GIOChannel** ppChannel, const gchar* mode, Logger* pLogger )
    {
        GError* _pError = NULL;
        *ppChannel = g_io_channel_new_file( fileName, mode, &_pError );
        if ( _pError )
        {
            gchar* _errorMsg = g_strdup_printf( "Unable to open file %s", fileName );
            handle_error( _errorMsg, _pError, pLogger );
            g_free( _errorMsg );
            return FALSE;
        }

        return TRUE;
    }

    gboolean file_close( const gchar* fileName, GIOChannel* pChannel, gboolean flush, Logger* pLogger )
    {
        GError* _pError = NULL;
        g_io_channel_shutdown( pChannel, flush, &_pError );
        if ( _pError )
        {
            gchar* _errorMsg = g_strdup_printf( "Unable to close file %s", fileName );
            handle_error( _errorMsg, _pError, pLogger );
            g_free( _errorMsg );
            return FALSE;
        }
        return TRUE;
    }

    gboolean file_read_line( const gchar* fileName, GIOChannel* pChannel, gchar** pLine,  gboolean* pDone, Logger* pLogger )
    {
        GError* _pError = NULL;
        GIOStatus _status = g_io_channel_read_line( pChannel, pLine, NULL, NULL, &_pError );
        if ( _pError )
        {
            gchar* _errorMsg = g_strdup_printf( "Unable to read file %s", fileName );
            handle_error( _errorMsg, _pError, pLogger );
            g_free( _errorMsg );
            return FALSE;
        }

        *pDone = (_status == G_IO_STATUS_EOF);
        return TRUE;
    }

    gboolean file_write_line( const gchar* fileName, GIOChannel* pChannel, gchar* line,  Logger* pLogger ) 
    {
        GError* _pError = NULL;
        g_io_channel_write_chars( pChannel, line, -1, NULL, &_pError ); 
        if ( _pError )
        {
            gchar* _errorMsg = g_strdup_printf( "Unable to write to file %s",  fileName );
            handle_error( _errorMsg, _pError, pLogger );
            g_free( _errorMsg );
            return FALSE;
        }

        if ( !g_str_has_suffix( line, "\n") )
        {
            g_io_channel_write_chars( pChannel, "\n", -1, NULL, &_pError ); 
            if ( _pError )
            {
                gchar* _errorMsg = g_strdup_printf( "Unable to write to file %s",  fileName );
                handle_error( _errorMsg, _pError, pLogger );
                g_free( _errorMsg );
                return FALSE;
            }
        }
        
        return TRUE;
    }

    gboolean file_move( const gchar* sourceFileName, const gchar* destFileName, Logger* pLogger )
    {
        if ( 0 > g_remove( destFileName ) )
        {
            pLogger->Error( g_strdup_printf( "Unable to delete file %s", destFileName ) );
            return FALSE;
        }

        if ( 0 < g_rename( sourceFileName, destFileName ) )
        {
            pLogger->Error( g_strdup_printf( "Unable to rename file %s to %s", sourceFileName, destFileName ) );
            return FALSE;
        }

        return TRUE;
    }
}