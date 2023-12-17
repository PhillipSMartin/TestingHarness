#pragma once

#include "gtk/gtk.h"

// HelpViewer
//
//      a scrollwindow containing a GtkTextView object for viewing help mesages
//
//  Public methods:
//      GtkWidget* help_viewer_new( Logger* pLogger )
//          creates a new HelpViewer
//
//       gchar* text help_viewer_get_text( HelpViewer* pHelpViewer, int* id )
//          returns the id (unless pointer is NULL) and txt of the TextView object
//          caller takes ownership of the cstring and must free it via g_free
//
//      void help_tree_store_set_text( HelpViewer* pHelpViewer, int id, gchar* text )
//          sets the text in the TextView object
//          the id is associated with the help object and is returned with the text on a 'get' (to detect race conditions)
//          the caller retains ownership of the passed cstring
//
//  Signals:
//      edited
//          help object was changed via an edit operation
//      replaced
//          help object was replaced  (nobody currently subscribes to this - we relay on the json-replaced signal)

G_BEGIN_DECLS

#define HELP_VIEWER_TYPE                ( help_viewer_get_type() )
#define HELP_VIEWER( obj )              ( G_TYPE_CHECK_INSTANCE_CAST( ( obj ), HELP_VIEWER_TYPE, HelpViewer ) )
#define HELP_VIEWER_CLASS( klass )      ( G_TYPE_CHECK_CLASS_CAST( (klass), HELP_VIEWER_TYPE, HelpViewerClass ) )
#define IS_HELP_VIEWER( obj )           ( G_TYPE_CHECK_INSTANCE_TYPE( (obj), HELP_VIEWER_TYPE ) )
#define IS_HELP_VIEWER_CLASS( klass )   ( G_TYPE_CHECK_CLASS_TYPE( (klass), HELP_VIEWER_TYPE ) )

typedef struct _HelpViewer               HelpViewer;
typedef struct _HelpViewerClass          HelpViewerClass;

struct _HelpViewer
{
    GtkScrolledWindow scrolledWindow_;
};

struct _HelpViewerClass
{
    GtkScrolledWindowClass parentClass_;

    void (* edited) ( HelpViewer* pHelpViewer );
    void (* replaced) ( HelpViewer* pHelpViewer );
};

GType help_viewer_get_type( void ) G_GNUC_CONST;
GtkWidget* help_viewer_new( void );

gchar* help_viewer_get_text( HelpViewer* pHelpViewer );
void help_viewer_set_text( HelpViewer* pHelpViewer, const gchar* text );

G_END_DECLS