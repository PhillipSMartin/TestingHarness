#include "TemplateListStore.h"

#include <algorithm>
#include <glib/gstdio.h>
#include <sstream>

#include "FileUtilities.h"
#include "Logger.h"

#define TEMPLATE_LIST_STORE_GET_PRIVATE( obj ) ( G_TYPE_INSTANCE_GET_PRIVATE( (obj), TEMPLATE_LIST_STORE_TYPE, TemplateListStorePrivate ) )

typedef struct _TemplateListStorePrivate TemplateListStorePrivate;

struct _TemplateListStorePrivate
{
    Logger* pLogger_;
    gchar* readFileName_ = NULL;
    gchar* writeFileName_ = NULL;
};

// Private function prototypes
static void template_list_store_class_init( TemplateListStoreClass* pKlass );
static void template_list_store_init( TemplateListStore* pTemplateListStore );

static int template_list_store_read_file( TemplateListStore* pTemplateListStore, GIOChannel* pChannel );
static int template_list_store_write_file( TemplateListStore* pTemplateListStore, GIOChannel* pChannel );
static gboolean template_list_store_add_item( TemplateListStore* pTemplateListStore, const gchar* line );
static gboolean template_list_store_get_item( TemplateListStore* pTemplateListStore, GtkTreeIter* pIter, gchar** pLine );

// Type registration
GType template_list_store_get_type( void )
{
    static GType _template_list_store_type = 0;

    if ( !_template_list_store_type )
    {
        static const GTypeInfo _template_list_store_info =
        {
            sizeof( TemplateListStoreClass ),
            NULL,
            NULL,
            (GClassInitFunc) template_list_store_class_init,
            NULL,
            NULL,
            sizeof( TemplateListStore ),
            0,
            (GInstanceInitFunc) template_list_store_init,
        };

        _template_list_store_type = g_type_register_static( GTK_TYPE_LIST_STORE, "TemplateListStore", &_template_list_store_info, (GTypeFlags)0 );
    }

    return _template_list_store_type;
}

// Initialization functions
static void template_list_store_class_init( TemplateListStoreClass* pKlass )
{
    GObjectClass* gobject_class = G_OBJECT_CLASS( pKlass );

    // add private data
    g_type_class_add_private( pKlass, sizeof( TemplateListStorePrivate ) );

    // register the signals
 }

static void template_list_store_init( TemplateListStore* pTemplateListStore )
{
    GType _gtypes[NUM_COLUMNS] = { G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING };
    gtk_list_store_set_column_types( GTK_LIST_STORE( pTemplateListStore ), NUM_COLUMNS,  _gtypes );
}

// Private functions
static int generate_id()
{
    static int _id = 0;
    return _id++;
}

// returns number of lines read or -1 on an error
static int template_list_store_read_file( TemplateListStore* pTemplateListStore, GIOChannel* pChannel )
{
    gchar* _line = NULL;
    int _lineNo = 0;
    gboolean _done = FALSE;
    TemplateListStorePrivate* _pPriv = TEMPLATE_LIST_STORE_GET_PRIVATE( pTemplateListStore );

    // returns FALSE on an error, TRUE if line is read or on EOF, sets _done to TRUE on EOF
    if ( !FileUtilities::file_read_line( _pPriv->readFileName_, pChannel, &_line,  &_done, _pPriv->pLogger_ ) )
    {
        _lineNo = -1;
        _done = TRUE;
    }

    while ( !_done )
    {  
        _lineNo++;
        if ( 0 > template_list_store_add_item( pTemplateListStore, _line ) )
        {
            _pPriv->pLogger_->Error( g_strdup_printf("Format error in %s, line %d: %s",  _pPriv->readFileName_, _lineNo, _line ) );
            _lineNo = -1;
            break;
        }

        g_free( _line );
        _line = NULL;

        if ( !FileUtilities::file_read_line( _pPriv->readFileName_, pChannel, &_line,  &_done, _pPriv->pLogger_ ) )
        {
            _lineNo = -1;
            break;
        }
    } 

    g_free( _line );
    return _lineNo;
}

// returns number of lines written or -1 on an error
static int template_list_store_write_file( TemplateListStore* pTemplateListStore, GIOChannel* pChannel )
{
    gchar* _line = NULL;
    GError* _pError = NULL;
    int _lineNo = 0;
    GtkTreeIter _iter;
    TemplateListStorePrivate* _pPriv = TEMPLATE_LIST_STORE_GET_PRIVATE( pTemplateListStore );

    gboolean _rc = gtk_tree_model_get_iter_first( GTK_TREE_MODEL( pTemplateListStore ) , &_iter );
    while ( _rc )
    {
        _lineNo++;
        g_free( _line );
        _line = NULL;
        if ( 0 > template_list_store_get_item( pTemplateListStore, &_iter, &_line ) )
        {
            _pPriv->pLogger_->Error( g_strdup_printf( "Unable to construct line %d for %s", _lineNo, _pPriv->writeFileName_ ));
            _lineNo = -1;
            break;
        }

        if ( !FileUtilities::file_write_line( _pPriv->writeFileName_, pChannel, _line, _pPriv->pLogger_ ) )
        {
            _lineNo = -1;
            break;
        }

        _rc = gtk_tree_model_iter_next( GTK_TREE_MODEL( pTemplateListStore ) , &_iter );
    }

    g_free( _line );
    return _lineNo;
}

static gboolean template_list_store_add_item( TemplateListStore* pTemplateListStore, const gchar* line )
{
    TemplateListStorePrivate* _pPriv = TEMPLATE_LIST_STORE_GET_PRIVATE( pTemplateListStore );
    GtkTreeIter _iter;
    std::string _key, _template, _help;
    std::stringstream _sstream(line);

    if ( !std::getline( _sstream, _key, '|' ) )
    {
        return FALSE;
    }

    if ( !std::getline( _sstream, _template, '|' ) )
    {
        return FALSE;
    }

    std::getline( _sstream, _help, '|' );
    std::replace( _help.begin(), _help.end(), ';', '\n' );

    gtk_list_store_append( GTK_LIST_STORE( pTemplateListStore ), &_iter );
    gtk_list_store_set( GTK_LIST_STORE( pTemplateListStore ), &_iter,
        KEY, _key.c_str(),
        TEMPLATE, _template.c_str(),
        HELP, _help.c_str(), -1 );

    return TRUE;
}

static gboolean template_list_store_get_item( TemplateListStore* pTemplateListStore, GtkTreeIter* iter, gchar** line )
{
    gchar *_key, *_template, *_help;
    gboolean _rc = TRUE;

    try
    {  
        gtk_tree_model_get( GTK_TREE_MODEL( pTemplateListStore ), iter, KEY, &_key, TEMPLATE, &_template, HELP, &_help, -1 );
        std::string _helpStr{ _help };
        std::replace( _helpStr.begin(), _helpStr.end(), '\n', ';' );

        *line = g_strjoin( "|", _key, _template, _helpStr.c_str(), NULL );
    }
    catch(const std::exception& e)
    {
        _rc = FALSE;
    }
 
    g_free( _key );
    g_free( _template );
    g_free( _help );
    return _rc;
}

// Public functions
TemplateListStore* template_list_store_new(Logger* pLogger)
{
    TemplateListStore* _pListStore = TEMPLATE_LIST_STORE( g_object_new( template_list_store_get_type(), NULL ) );
    TemplateListStorePrivate* _pPriv = TEMPLATE_LIST_STORE_GET_PRIVATE( _pListStore );
    _pPriv->pLogger_ = pLogger;
    return _pListStore;
} 

// returns false if iter is invalid
// pointer (except iter) can be NULL if we are not interested in the value
gboolean template_list_store_get_values( TemplateListStore* pTemplateListStore, GtkTreeIter *iter, gchar** templateName, gchar** templateText, gchar** helpText )
{
    try
    {
        if (templateName != NULL)
            {
                gtk_tree_model_get( GTK_TREE_MODEL( pTemplateListStore ), iter, KEY, templateName, -1 );
            }
        if (templateText != NULL)
            {
                gtk_tree_model_get( GTK_TREE_MODEL( pTemplateListStore ), iter, TEMPLATE, templateText, -1 );
            }
        if (helpText != NULL)
            {
                gtk_tree_model_get( GTK_TREE_MODEL( pTemplateListStore ), iter, HELP, helpText, -1 );
            }
        }
    catch(const std::exception& e)
    {
        TemplateListStorePrivate* _pPriv = TEMPLATE_LIST_STORE_GET_PRIVATE( pTemplateListStore );
        _pPriv->pLogger_->ErrorStr( "Template chooser unable to get values: invalid iterator" );
        return FALSE;
    }
      
   return TRUE;
}    

// returns number of templates imported or -1 on an error
gint template_list_store_import_file( TemplateListStore* pTemplateListStore, const gchar* fileName )
{
    TemplateListStorePrivate* _pPriv = TEMPLATE_LIST_STORE_GET_PRIVATE( pTemplateListStore );
    int _linesRead;
    gtk_list_store_clear( GTK_LIST_STORE( pTemplateListStore ) );

    g_free( _pPriv->readFileName_ );
    _pPriv->readFileName_ = g_strdup( fileName );

    GIOChannel* _pChannel;
    if ( FileUtilities::file_open( _pPriv->readFileName_, &_pChannel, "r", _pPriv->pLogger_ ) )
    {
 
        _linesRead = template_list_store_read_file( pTemplateListStore, _pChannel );
        if ( FileUtilities::file_close( _pPriv->readFileName_, _pChannel, FALSE, _pPriv->pLogger_ ) )
        {
            return _linesRead;
        }
    }

    return -1;   
}

gboolean template_list_store_export_file( TemplateListStore* pTemplateListStore )
{
    GError* _pError = NULL;
    TemplateListStorePrivate* _pPriv = TEMPLATE_LIST_STORE_GET_PRIVATE( pTemplateListStore );  

    g_free( _pPriv->writeFileName_ );
    _pPriv->writeFileName_ = g_strdup_printf( "%s.tmp", _pPriv->readFileName_);

    GIOChannel* _channel;
    if ( FileUtilities::file_open( _pPriv->writeFileName_, &_channel, "w", _pPriv->pLogger_ ) )
    {
        template_list_store_write_file( pTemplateListStore, _channel );
        if ( FileUtilities::file_close( _pPriv->writeFileName_, _channel, TRUE, _pPriv->pLogger_ ) )
        {
            _pPriv->pLogger_->Debug( g_strdup_printf( "Wrote temp file %s", _pPriv->writeFileName_ ) );
            if ( FileUtilities::file_move( _pPriv->writeFileName_, _pPriv->readFileName_, _pPriv->pLogger_) )
            {
               _pPriv->pLogger_->Debug( g_strdup_printf( "Saved file %s", _pPriv->readFileName_ ) );  
               return TRUE;
            }
        }
    }

    return FALSE;      
}


