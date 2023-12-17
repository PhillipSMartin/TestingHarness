#include <algorithm>
#include <sstream>

#include "activemq.h"
#include "FileUtilities.h"
#include "DestinationListStore.h"
#include "DestinationMap.h"
#include "Logger.h"

#define DESTINATION_LIST_STORE_GET_PRIVATE( obj ) ( G_TYPE_INSTANCE_GET_PRIVATE( (obj), DESTINATION_LIST_STORE_TYPE, DestinationListStorePrivate ) )

typedef struct _DestinationListStorePrivate DestinationListStorePrivate;

struct _DestinationListStorePrivate
{
    Logger* pLogger_;
};

// Private function prototypes
static void destination_list_store_class_init( DestinationListStoreClass* pKlass );
static void destination_list_store_init( DestinationListStore* pDestinationListStore );

static void destination_list_store_add_item( DestinationListStore* pDestinationListStore, const gchar* destinationName, DestinationType destinationType );

// Type registration
GType destination_list_store_get_type( void )
{
    static GType _destination_list_store_type = 0;

    if ( !_destination_list_store_type )
    {
        static const GTypeInfo _destination_list_store_info =
        {
            sizeof( DestinationListStoreClass ),
            NULL,
            NULL,
            (GClassInitFunc) destination_list_store_class_init,
            NULL,
            NULL,
            sizeof( DestinationListStore ),
            0,
            (GInstanceInitFunc) destination_list_store_init,
        };

        _destination_list_store_type = g_type_register_static( GTK_TYPE_LIST_STORE, "DestinationListStore", &_destination_list_store_info, (GTypeFlags)0 );
    }

    return _destination_list_store_type;
}

// Initialization functions
static void destination_list_store_class_init( DestinationListStoreClass* pKlass )
{
    GObjectClass* gobject_class = G_OBJECT_CLASS( pKlass );

    // add private data
    g_type_class_add_private( pKlass, sizeof( DestinationListStorePrivate ) );

    // register the signals
 }

static void destination_list_store_init( DestinationListStore* pDestinationListStore )
{
    GType _gtypes[NUM_COLUMNS] = { G_TYPE_STRING, G_TYPE_INT, G_TYPE_STRING };
    gtk_list_store_set_column_types( GTK_LIST_STORE( pDestinationListStore ), NUM_COLUMNS,  _gtypes );
}

// Private functions

static void destination_list_store_add_item( DestinationListStore* pDestinationListStore, const gchar* destinationName, DestinationType destinationType , const gchar* destinationNameTemplate)
{
    DestinationListStorePrivate* _pPriv = DESTINATION_LIST_STORE_GET_PRIVATE( pDestinationListStore );
    GtkTreeIter _iter;
 
    gtk_list_store_append( GTK_LIST_STORE( pDestinationListStore ), &_iter );
    gtk_list_store_set( GTK_LIST_STORE( pDestinationListStore ), &_iter,
        DESTINATION_NAME, destinationName,
        DESTINATION_TYPE, destinationType,
        DESTINATION_NAME_TEMPLATE, destinationNameTemplate, -1 );
}

// Public functions
DestinationListStore* destination_list_store_new(Logger* pLogger)
{
    DestinationListStore* _pListStore = DESTINATION_LIST_STORE( g_object_new( destination_list_store_get_type(), NULL ) );
    DestinationListStorePrivate* _pPriv = DESTINATION_LIST_STORE_GET_PRIVATE( _pListStore );
    _pPriv->pLogger_ = pLogger;
    return _pListStore;
} 

// returns false if iter is invalid
// pointer (except iter) can be NULL if we are not interested in the value
gboolean destination_list_store_get_values( DestinationListStore* pDestinationListStore, 
    GtkTreeIter *iter, gchar** pDestinationName, gint* destinationType, gchar** pDestinationNameTemplate )
{
    try
    {
        if (pDestinationName != NULL)
        {
            gtk_tree_model_get( GTK_TREE_MODEL( pDestinationListStore ), iter, DESTINATION_NAME, pDestinationName, -1 );
        }
        if (destinationType != NULL)
        {
            gtk_tree_model_get( GTK_TREE_MODEL( pDestinationListStore ), iter, DESTINATION_TYPE, destinationType, -1 );
        }
        if (pDestinationNameTemplate != NULL)
        {
            gtk_tree_model_get( GTK_TREE_MODEL( pDestinationListStore ), iter, DESTINATION_NAME_TEMPLATE, pDestinationNameTemplate, -1 );
        }
   }
    catch(const std::exception& e)
    {
        DestinationListStorePrivate* _pPriv = DESTINATION_LIST_STORE_GET_PRIVATE( pDestinationListStore );
        _pPriv->pLogger_->ErrorStr( "Destination chooser unable to get values: invalid iterator" );
        return FALSE;
    }
      
   return TRUE;
}    

// returns number of lines read
gint destination_list_store_import_map( DestinationListStore* pDestinationListStore, DestinationMap* pDestinationMap, const gchar* defaultDestination, gint* pDefaultIndex )
{
    DestinationListStorePrivate* _pPriv = DESTINATION_LIST_STORE_GET_PRIVATE( pDestinationListStore );
    gtk_list_store_clear( GTK_LIST_STORE( pDestinationListStore ) );

    *pDefaultIndex = -1;
    gint _lineNo = 0;

    for ( auto& _item : *pDestinationMap )
    {
        if ( g_strcmp0( defaultDestination, _item.first.c_str() ) == 0 )
        {
           *pDefaultIndex = _lineNo;
        }

        _lineNo++; 
        destination_list_store_add_item( pDestinationListStore, _item.first.c_str(), _item.second.first, _item.second.second.c_str() );
    } 

    return _lineNo;
}