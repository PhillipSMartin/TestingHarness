#include "HelpViewer.h"

#define HELP_VIEWER_GET_PRIVATE( obj ) ( G_TYPE_INSTANCE_GET_PRIVATE( (obj), HELP_VIEWER_TYPE, HelpViewerPrivate ) )

typedef struct _HelpViewerPrivate HelpViewerPrivate;

struct _HelpViewerPrivate
{
    GtkWidget* pTextView_;
    GtkTextBuffer* pTextBuffer_;
    gboolean setTextInProgress = FALSE; // we avoid sending an "edit" signal if we are replacing the text
};

// signals emitted
enum
{
    EDITED_SIGNAL,
    REPLACED_SIGNAL,
    LAST_SIGNAL
};

static guint help_viewer_signals[LAST_SIGNAL] = { 0 };

// Private function prototypes
static void help_viewer_class_init( HelpViewerClass* pKlass );
static void help_viewer_init( HelpViewer* pHelpViewer );
static void help_viewer_on_text_buffer_changed( GtkTextBuffer* pTextBuffer, HelpViewer* pHelpViewer );

// Type registration
GType help_viewer_get_type( void )
{
    static GType _help_viewer_type = 0;

    if ( !_help_viewer_type )
    {
        static const GTypeInfo _help_viewer_info =
        {
            sizeof( HelpViewerClass ),
            NULL,
            NULL,
            (GClassInitFunc) help_viewer_class_init,
            NULL,
            NULL,
            sizeof( HelpViewer ),
            0,
            (GInstanceInitFunc) help_viewer_init,
        };

        _help_viewer_type = g_type_register_static( GTK_TYPE_SCROLLED_WINDOW, "HelpViewer", &_help_viewer_info, (GTypeFlags)0 );
    }

    return _help_viewer_type;
}


// Initialization functions
static void help_viewer_class_init( HelpViewerClass* pKlass )
{
    GObjectClass* gobject_class = G_OBJECT_CLASS( pKlass );

    // add private data
    g_type_class_add_private( pKlass, sizeof( HelpViewerPrivate ) );


    // register the signals
    help_viewer_signals[EDITED_SIGNAL] =
            g_signal_new( "edited", G_TYPE_FROM_CLASS( pKlass ),
            (GSignalFlags)(G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION),
            G_STRUCT_OFFSET( HelpViewerClass, edited ),
            NULL,
            NULL,
            g_cclosure_marshal_VOID__VOID,
            G_TYPE_NONE,
            0 );
    help_viewer_signals[REPLACED_SIGNAL] = 
        g_signal_new( "replaced", G_TYPE_FROM_CLASS( pKlass ),
            (GSignalFlags)(G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION),
            G_STRUCT_OFFSET( HelpViewerClass, replaced ),
            NULL,
            NULL,
            g_cclosure_marshal_VOID__VOID,
            G_TYPE_NONE,
            0 );
}

static void help_viewer_init( HelpViewer* pHelpViewer )
{
    HelpViewerPrivate* _pPriv = HELP_VIEWER_GET_PRIVATE( pHelpViewer );

    gtk_scrolled_window_set_policy( GTK_SCROLLED_WINDOW( pHelpViewer ), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC );

    // add text view
    _pPriv->pTextView_ = gtk_text_view_new();
    _pPriv->pTextBuffer_ = gtk_text_view_get_buffer( GTK_TEXT_VIEW( _pPriv->pTextView_ ) );
    gtk_container_add( GTK_CONTAINER( pHelpViewer ), _pPriv->pTextView_ );

    // wire signals
    g_signal_connect( G_OBJECT( _pPriv->pTextBuffer_ ), "changed", G_CALLBACK( help_viewer_on_text_buffer_changed ), gpointer(pHelpViewer) );
}

static void  help_viewer_on_text_buffer_changed( GtkTextBuffer* pTextBuffer, HelpViewer* pHelpViewer )
{
    HelpViewerPrivate* _pPriv = HELP_VIEWER_GET_PRIVATE( pHelpViewer ); 

    if ( !_pPriv->setTextInProgress)
    {
        g_signal_emit_by_name( (gpointer)pHelpViewer, "edited" );
    }
}

// Public functions
GtkWidget* help_viewer_new()
{
    return GTK_WIDGET( g_object_new( help_viewer_get_type(), NULL ) );
} 

gchar* help_viewer_get_text( HelpViewer* pHelpViewer )
{
    HelpViewerPrivate* _pPriv = HELP_VIEWER_GET_PRIVATE( pHelpViewer );  

    GtkTextIter _start, _end;
    gtk_text_buffer_get_start_iter( _pPriv->pTextBuffer_, &_start );     
    gtk_text_buffer_get_end_iter( _pPriv->pTextBuffer_, &_end );
    return gtk_text_buffer_get_text( _pPriv->pTextBuffer_, &_start, &_end, TRUE );
}

void help_viewer_set_text( HelpViewer* pHelpViewer, const gchar* text )
{
    HelpViewerPrivate* _pPriv = HELP_VIEWER_GET_PRIVATE( pHelpViewer ); 
    _pPriv->setTextInProgress = TRUE;
    gtk_text_buffer_set_text( _pPriv->pTextBuffer_, text ? text : "", -1 );
    _pPriv->setTextInProgress = FALSE;

    g_signal_emit_by_name( (gpointer)pHelpViewer, "replaced" );
}