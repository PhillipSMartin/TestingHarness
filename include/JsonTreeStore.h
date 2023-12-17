#pragma once

#include <gtk/gtk.h>

class Logger;

// JsonTreeStore
//
//      a hierarchical model for storing an nlohmann::json object
//      can be viewed by a GtkTreeView or a JsonTreeView wdiget
//
//  Public methods:
//      JsonTreeStore* json_tree_store_new( Logger* pLogger )
//          creates a new, empty, JsonTreeStore
//
//      gchar* text json_tree_store_get_json_string( JsonTreeStore* pJsonTreeStore )
//          returns the cstring representation of the internal json object
//          caller takes ownership of the cstring and must free it via g_free
//
//      void json_tree_store_set_json_string( JsonTreeStore* pJsonTreeStore, gchar* text )
//          uses the passed cstring to set the internal json object
//          the caller retains ownership of the passed cstring
//
//      void json_tree_store_edit_json_string( JsonTreeStore* pJsonTreeStore,
//              gchar* path, gchar* newText )
//          modifies the text located at the specified path
//
//  Signals:
//      json-edited
//          json object was changed via an edit operation
//      json-replaced
//          json object was replaced via a call to json_tree_store_set_json_string

G_BEGIN_DECLS

#define JSON_TREE_STORE_TYPE                ( json_tree_store_get_type() )
#define JSON_TREE_STORE( obj )              ( G_TYPE_CHECK_INSTANCE_CAST( ( obj ), JSON_TREE_STORE_TYPE, JsonTreeStore ) )
#define JSON_TREE_STORE_CLASS( klass )      ( G_TYPE_CHECK_CLASS_CAST( (klass), JSON_TREE_STORE_TYPE, JsonTreeStoreClass ) )
#define IS_JSON_TREE_STORE( obj )           ( G_TYPE_CHECK_INSTANCE_TYPE( (obj), JSON_TREE_STORE_TYPE ) )
#define IS_JSON_TREE_STORE_CLASS( klass )   ( G_TYPE_CHECK_CLASS_TYPE( (klass), JSON_TREE_STORE_TYPE ) )

typedef struct _JsonTreeStore               JsonTreeStore;
typedef struct _JsonTreeStoreClass          JsonTreeStoreClass;

struct _JsonTreeStore
{
    GtkTreeStore treeStore_;
};

struct _JsonTreeStoreClass
{
    GtkTreeStoreClass parentClass_;

    void (* json_edited) ( JsonTreeStore* pJsonTreeStore );
    void (* json_replaced) ( JsonTreeStore* pJsonTreeStore );
};

// tree model columns
enum 
{
    KEY = 0,
    VALUE,
    KEY_PATH,
    EDITABLE,
    NUM_COLUMNS
};

GType json_tree_store_get_type( void ) G_GNUC_CONST;
JsonTreeStore*  json_tree_store_new( Logger* pLogger );

gchar* json_tree_store_get_json_string( JsonTreeStore* pJsonTreeStore );
void json_tree_store_set_json_string( JsonTreeStore* pJsonTreeStore , const gchar* text );
void json_tree_store_edit_json_string( JsonTreeStore* pJsonTreeStore, const gchar* path, const gchar* newText );

G_END_DECLS