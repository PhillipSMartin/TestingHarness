#include "JsonTreeStore.h"
#include "JsonViewer.h"
#include "nlohmann/json.hpp"

#define JSON_VIEWER_GET_PRIVATE( obj ) ( G_TYPE_INSTANCE_GET_PRIVATE( (obj), JSON_VIEWER_TYPE, JsonViewerPrivate ) )

typedef struct _JsonViewerPrivate JsonViewerPrivate;

struct _JsonViewerPrivate
{
    Logger* pLogger_;
    GtkWidget* pTreeView_;
    JsonTreeStore* pTreeStore_;
    gboolean expandFirstRow_;
};

// signals emitted
enum
{
    EDITED_SIGNAL,
    REPLACED_SIGNAL,
    LAST_SIGNAL
};

static guint json_viewer_signals[LAST_SIGNAL] = { 0 };

// Private function prototypes
static void json_viewer_class_init( JsonViewerClass* pKlass );
static void json_viewer_init( JsonViewer* pJsonViewer );
static void json_viewer_on_cell_edited( GtkCellRendererText* pRenderer, gchar* path, gchar* newText, JsonViewer* pFsonViewer );
static void json_viewer_on_json_edited( JsonTreeStore* pJsonTreeStore, JsonViewer* pJsonViewer );
static void json_viewer_on_json_replaced( JsonTreeStore* pJsonTreeStore, JsonViewer* pJsonViewer );

// Type registration
GType json_viewer_get_type( void )
{
    static GType _json_viewer_type = 0;

    if ( !_json_viewer_type )
    {
        static const GTypeInfo _json_viewer_info =
        {
            sizeof( JsonViewerClass ),
            NULL,
            NULL,
            (GClassInitFunc) json_viewer_class_init,
            NULL,
            NULL,
            sizeof( JsonViewer ),
            0,
            (GInstanceInitFunc) json_viewer_init,
        };

        _json_viewer_type = g_type_register_static( GTK_TYPE_SCROLLED_WINDOW, "JsonViewer", &_json_viewer_info, (GTypeFlags)0 );
    }

    return _json_viewer_type;
}

static void json_viewer_on_cell_edited( GtkCellRendererText* pRenderer, gchar* path, gchar* newText, JsonViewer* pJsonViewer )
{
    JsonViewerPrivate* _pPriv = JSON_VIEWER_GET_PRIVATE( pJsonViewer );
    json_tree_store_edit_json_string( _pPriv->pTreeStore_, path, newText );
}

static void json_viewer_on_json_edited( JsonTreeStore* pJsonTreeStore, JsonViewer* pJsonViewer )
{
    g_signal_emit_by_name( (gpointer)pJsonViewer, "json-edited" );
}

static void json_viewer_on_json_replaced( JsonTreeStore* pJsonTreeStore, JsonViewer* pJsonViewer )
{
    g_signal_emit_by_name( (gpointer)pJsonViewer, "json-replaced" );
}

// Initialization functions
static void json_viewer_class_init( JsonViewerClass* pKlass )
{
    GObjectClass* gobject_class = G_OBJECT_CLASS( pKlass );

    // add private data
    g_type_class_add_private( pKlass, sizeof( JsonViewerPrivate ) );


    // register the signals
    json_viewer_signals[EDITED_SIGNAL] =
            g_signal_new( "json-edited", G_TYPE_FROM_CLASS( pKlass ),
            (GSignalFlags)(G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION),
            G_STRUCT_OFFSET( JsonViewerClass, json_edited ),
            NULL,
            NULL,
            g_cclosure_marshal_VOID__VOID,
            G_TYPE_NONE,
            0 );
    json_viewer_signals[REPLACED_SIGNAL] = 
        g_signal_new( "json-replaced", G_TYPE_FROM_CLASS( pKlass ),
            (GSignalFlags)(G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION),
            G_STRUCT_OFFSET( JsonViewerClass, json_replaced ),
            NULL,
            NULL,
            g_cclosure_marshal_VOID__VOID,
            G_TYPE_NONE,
            0 );
}

static void json_viewer_init( JsonViewer* pJsonViewer )
{
    JsonViewerPrivate* _pPriv = JSON_VIEWER_GET_PRIVATE( pJsonViewer );

    gtk_scrolled_window_set_policy( GTK_SCROLLED_WINDOW( pJsonViewer ), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC );

    // add tree view
    _pPriv->pTreeView_ = gtk_tree_view_new();
    gtk_container_add( GTK_CONTAINER( pJsonViewer ), _pPriv->pTreeView_ );

    // add columns to tree view
    GtkCellRenderer* _pRenderer = gtk_cell_renderer_text_new();
    gtk_tree_view_append_column( GTK_TREE_VIEW( _pPriv->pTreeView_ ), 
        gtk_tree_view_column_new_with_attributes( "Key", _pRenderer, "text", KEY, NULL ) );

    _pRenderer = gtk_cell_renderer_text_new();
    gtk_tree_view_append_column( GTK_TREE_VIEW( _pPriv->pTreeView_ ),     
        gtk_tree_view_column_new_with_attributes( "Value", _pRenderer, "text", VALUE, "editable", EDITABLE, NULL ) );

    // wire signals
    g_signal_connect( G_OBJECT( _pRenderer ), "edited", G_CALLBACK( json_viewer_on_cell_edited ), gpointer(pJsonViewer) );
}

// Public functions
GtkWidget* json_viewer_new( Logger* pLogger, gboolean expandFirstRow )
{
    GtkWidget* _pViewer = GTK_WIDGET( g_object_new( json_viewer_get_type(), NULL ) );
    JsonViewerPrivate* _pPriv = JSON_VIEWER_GET_PRIVATE( _pViewer );
    _pPriv->pLogger_ = pLogger;
    _pPriv->expandFirstRow_ = expandFirstRow;

    // add model to tree view - must be done here because we did not have pLogger during json_viewer_init
    _pPriv->pTreeStore_ = json_tree_store_new( _pPriv->pLogger_ );
    gtk_tree_view_set_model( GTK_TREE_VIEW( _pPriv->pTreeView_ ), GTK_TREE_MODEL(  _pPriv->pTreeStore_ ) );
    g_object_unref(  _pPriv->pTreeStore_ );

    g_signal_connect( G_OBJECT( _pPriv->pTreeStore_ ), "json-edited", G_CALLBACK( json_viewer_on_json_edited ), gpointer(_pViewer) );
    g_signal_connect( G_OBJECT( _pPriv->pTreeStore_ ), "json-replaced", G_CALLBACK( json_viewer_on_json_replaced ), gpointer(_pViewer) );

    return _pViewer;
} 

gchar* json_viewer_get_json_string( JsonViewer* pJsonViewer )
{
     JsonViewerPrivate* _pPriv = JSON_VIEWER_GET_PRIVATE( pJsonViewer ); 
     return json_tree_store_get_json_string( _pPriv->pTreeStore_ );
}

void json_viewer_set_json_string( JsonViewer* pJsonViewer, const gchar* text )
{
    JsonViewerPrivate* _pPriv = JSON_VIEWER_GET_PRIVATE( pJsonViewer ); 
    json_tree_store_set_json_string( _pPriv->pTreeStore_, text );  

    if ( _pPriv->expandFirstRow_ )
    {
        GtkTreePath* _pFirstRow = gtk_tree_path_new_first();
        gtk_tree_view_expand_row( GTK_TREE_VIEW( _pPriv->pTreeView_ ) , _pFirstRow, FALSE );
        gtk_tree_path_free( _pFirstRow );
    } 
}

