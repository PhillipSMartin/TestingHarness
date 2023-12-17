#pragma once

class DestinationMap;
class Logger;

G_BEGIN_DECLS

#define DESTINATION_LIST_STORE_TYPE                ( destination_list_store_get_type() )
#define DESTINATION_LIST_STORE( obj )              ( G_TYPE_CHECK_INSTANCE_CAST( ( obj ), DESTINATION_LIST_STORE_TYPE, DestinationListStore ) )
#define DESTINATION_LIST_STORE_CLASS( klass )      ( G_TYPE_CHECK_CLASS_CAST( (klass), DESTINATION_LIST_STORE_TYPE, DestinationListStoreClass ) )
#define IS_DESTINATION_LIST_STORE( obj )           ( G_TYPE_CHECK_INSTANCE_TYPE( (obj), DESTINATION_LIST_STORE_TYPE ) )
#define IS_DESTINATION_LIST_STORE_CLASS( klass )   ( G_TYPE_CHECK_CLASS_TYPE( (klass), DESTINATION_LIST_STORE_TYPE ) )

typedef struct _DestinationListStore               DestinationListStore;
typedef struct _DestinationListStoreClass          DestinationListStoreClass;

struct _DestinationListStore
{
    GtkListStore listStore_;
};

struct _DestinationListStoreClass
{
    GtkListStoreClass parentClass_;
};

// tree model columns
enum 
{
    DESTINATION_NAME = 0,
    DESTINATION_TYPE,
    DESTINATION_NAME_TEMPLATE,
    NUM_COLUMNS
};

GType destination_list_store_get_type( void ) G_GNUC_CONST;
DestinationListStore*  destination_list_store_new( Logger* pLogger );

// returns false if iter is invalid
// pointer (except iter) can be NULL if we are not interested in the value
gboolean destination_list_store_get_values( DestinationListStore* pDestinationListStore, 
    GtkTreeIter *pIter, gchar** pDestinationName, gint* pDestinationType, gchar** pDestinationNameTemplate );

gint destination_list_store_import_map( DestinationListStore* pDestinationListStore, DestinationMap* pDestinationMap, const gchar* defaultDestination, gint* pDefaultIndex );

G_END_DECLS