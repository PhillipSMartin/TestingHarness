#pragma once

#include "gtk/gtk.h"

class Logger;

// JsonViewer
//
//      a scrollwindow containing a GtkTreeView object for viewing json objects
//      most functionality is handled the associated JsonTreeStore 
//
//  Public methods:
//      GtkWidget* json_viewer_new( Logger* pLogger )
//          creates a new JsonViewer
//
//       gchar* text json_viewer_get_json_string( JsonViewer* pJsonViewer, int* id )
//          returns the id (unless pointer is NULL) and cstring representation of the internal json object
//          caller takes ownership of the cstring and must free it via g_free
//
//      void json_tree_store_set_json_string( JsonViewer* pJsonViewer, int id, gchar* text )
//          uses the passed cstring to set the internal json object
//          the id is associated with the json object and is returned with the object on a 'get' (to detect race conditions)
//          the caller retains ownership of the passed cstring
//
//  Signals:
//      json-edited
//          json object was changed via an edit operation
//      json-replaced
//          json object was replaced 

G_BEGIN_DECLS

#define JSON_VIEWER_TYPE                ( json_viewer_get_type() )
#define JSON_VIEWER( obj )              ( G_TYPE_CHECK_INSTANCE_CAST( ( obj ), JSON_VIEWER_TYPE, JsonViewer ) )
#define JSON_VIEWER_CLASS( klass )      ( G_TYPE_CHECK_CLASS_CAST( (klass), JSON_VIEWER_TYPE, JsonViewerClass ) )
#define IS_JSON_VIEWER( obj )           ( G_TYPE_CHECK_INSTANCE_TYPE( (obj), JSON_VIEWER_TYPE ) )
#define IS_JSON_VIEWER_CLASS( klass )   ( G_TYPE_CHECK_CLASS_TYPE( (klass), JSON_VIEWER_TYPE ) )

typedef struct _JsonViewer               JsonViewer;
typedef struct _JsonViewerClass          JsonViewerClass;

struct _JsonViewer
{
    GtkScrolledWindow scrolledWindow_;
};

struct _JsonViewerClass
{
    GtkScrolledWindowClass parentClass_;

    void (* json_edited) ( JsonViewer* pJsonViewer );
    void (* json_replaced) ( JsonViewer* pJsonViewer );
};

GType json_viewer_get_type( void ) G_GNUC_CONST;
GtkWidget* json_viewer_new( Logger* pLogger, gboolean expandFirstRow = FALSE );

gchar* json_viewer_get_json_string( JsonViewer* pJsonViewer );
void json_viewer_set_json_string( JsonViewer* pJsonViewer, const gchar* text );

G_END_DECLS