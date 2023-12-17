#include <iostream>

#include "ConsoleViewer.h"
#include "Logger.h"

#define CONSOLE_VIEWER_GET_PRIVATE( obj ) ( G_TYPE_INSTANCE_GET_PRIVATE( (obj), CONSOLE_VIEWER_TYPE, ConsoleViewerPrivate ) )

typedef struct _ConsoleViewerPrivate ConsoleViewerPrivate;

class _ConsoleLogger : public ConsoleLogger
{
    private:
        ConsoleViewer* pConsoleViewer_;
        gboolean autoScroll_ = TRUE;

    public:
        _ConsoleLogger( ConsoleViewer* pConsoleViewer) : pConsoleViewer_( pConsoleViewer ) {}

        gboolean get_auto_scroll () { return autoScroll_; }
        void set_auto_scroll ( gboolean autoScroll ) { autoScroll_ = autoScroll; }

        virtual void add_console_msg( const gchar* msg );
};

struct _ConsoleViewerPrivate : ConsoleLogger
{
    GtkWidget* pTextView_;
    _ConsoleLogger* pConsoleLogger_;
};


// Private function prototypes
static void console_viewer_class_init( ConsoleViewerClass* pKlass );
static void console_viewer_init( ConsoleViewer* pConsoleViewer );
static void console_viewer_append_msg( const gchar* msg, gpointer pConsoleViewer );

// Type registration
GType console_viewer_get_type( void )
{
    static GType _console_viewer_type = 0;

    if ( !_console_viewer_type )
    {
        static const GTypeInfo _console_viewer_info =
        {
            sizeof( ConsoleViewerClass ),
            NULL,
            NULL,
            (GClassInitFunc) console_viewer_class_init,
            NULL,
            NULL,
            sizeof( ConsoleViewer ),
            0,
            (GInstanceInitFunc) console_viewer_init,
        };

        _console_viewer_type = g_type_register_static( GTK_TYPE_SCROLLED_WINDOW, "ConsoleViewer", &_console_viewer_info, (GTypeFlags)0 );
    }

    return _console_viewer_type;
}

void _ConsoleLogger::add_console_msg( const gchar* msg )
{
    ConsoleViewerPrivate* _pPriv = CONSOLE_VIEWER_GET_PRIVATE( CONSOLE_VIEWER( pConsoleViewer_ ) );  
    GtkTextIter _iter;
    GtkTextBuffer* _pBuffer = gtk_text_view_get_buffer( GTK_TEXT_VIEW( _pPriv->pTextView_ ) );
    gtk_text_buffer_get_end_iter( _pBuffer, &_iter );
    gtk_text_buffer_insert( _pBuffer, &_iter, msg, -1 );

    if ( autoScroll_ )
    {
        gtk_text_buffer_get_end_iter( _pBuffer, &_iter );
        GtkTextMark* _eof = gtk_text_buffer_create_mark( _pBuffer, "eof", &_iter, FALSE );
        gtk_text_view_scroll_mark_onscreen( GTK_TEXT_VIEW( _pPriv->pTextView_ ), _eof );
        gtk_text_buffer_delete_mark( _pBuffer, _eof );
    }
}


// Initialization functions
static void console_viewer_class_init( ConsoleViewerClass* pKlass )
{
    GObjectClass* gobject_class = G_OBJECT_CLASS( pKlass );

    // add private data
    g_type_class_add_private( pKlass, sizeof( ConsoleViewerPrivate ) );
}

static void console_viewer_init( ConsoleViewer* pConsoleViewer )
{
    ConsoleViewerPrivate* _pPriv = CONSOLE_VIEWER_GET_PRIVATE( pConsoleViewer );

    _pPriv->pTextView_ = gtk_text_view_new();
    gtk_scrolled_window_set_policy ( GTK_SCROLLED_WINDOW( pConsoleViewer ), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC );
    gtk_container_add( GTK_CONTAINER( pConsoleViewer ), _pPriv->pTextView_ );
} 

// Public functions
GtkWidget* console_viewer_new( Logger* pLogger )
{
    GtkWidget* _pViewer = GTK_WIDGET( g_object_new( console_viewer_get_type(), NULL ) );
    ConsoleViewerPrivate* _pPriv = CONSOLE_VIEWER_GET_PRIVATE( _pViewer );

    _pPriv->pConsoleLogger_ = new _ConsoleLogger( CONSOLE_VIEWER( _pViewer ) );
    pLogger->set_console_logger( _pPriv->pConsoleLogger_ );

    return _pViewer;
} 

gboolean console_viewer_get_auto_scroll( ConsoleViewer* pConsoleViewer )
{
    ConsoleViewerPrivate* _pPriv = CONSOLE_VIEWER_GET_PRIVATE( pConsoleViewer );
    return _pPriv->pConsoleLogger_->get_auto_scroll();
}

void console_viewer_set_auto_scroll( ConsoleViewer* pConsoleViewer, gboolean autoScroll )
{
    ConsoleViewerPrivate* _pPriv = CONSOLE_VIEWER_GET_PRIVATE( pConsoleViewer );
    _pPriv->pConsoleLogger_->set_auto_scroll( autoScroll );
}