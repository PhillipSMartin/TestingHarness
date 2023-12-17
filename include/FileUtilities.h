#pragma once

#include <glib/gstdio.h>

class Logger;

namespace FileUtilities
{
    gboolean file_open( const gchar* fileName, GIOChannel** ppChannel, const gchar* mode, Logger* pLogger );
    gboolean file_close( const gchar* fileName, GIOChannel* pChannel, gboolean flush, Logger* pLogger );

    // returns FALSE on an error, TRUE if line is read or on EOF, sets pDone to TRUE on EOF
    gboolean file_read_line( const gchar* fileName, GIOChannel* pChannel, gchar** pLine,  gboolean* pDone, Logger* pLogger );
    gboolean file_write_line( const gchar* fileName, GIOChannel* pChannel, gchar* line,  Logger* pLogger );
    gboolean file_move( const gchar* sourceFileName, const gchar* destFileName, Logger* pLogger );

}