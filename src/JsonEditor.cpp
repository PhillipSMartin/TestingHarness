#include "JsonTreeStore.h"
#include "JsonEditor.h"
#include "JsonUtilities.h"

#define JSON_EDITOR_GET_PRIVATE( obj ) ( G_TYPE_INSTANCE_GET_PRIVATE( (obj), JSON_EDITOR_TYPE, JsonEditorPrivate ) )

typedef struct _JsonEditorPrivate JsonEditorPrivate;

struct _JsonEditorPrivate
{
    Logger* pLogger_;
    GtkWidget* pTextView_;
};


// Private function prototypes
static void json_editor_class_init( JsonEditorClass* pKlass );
static void json_editor_init( JsonEditor* pJsonEditor );
static gboolean json_editor_verify_json_string( JsonEditor* pJsonEditor, const gchar* text );

// Type registration
GType json_editor_get_type( void )
{
    static GType _json_editor_type = 0;

    if ( !_json_editor_type )
    {
        static const GTypeInfo _json_editor_info =
        {
            sizeof( JsonEditorClass ),
            NULL,
            NULL,
            (GClassInitFunc) json_editor_class_init,
            NULL,
            NULL,
            sizeof( JsonEditor ),
            0,
            (GInstanceInitFunc) json_editor_init,
        };

        _json_editor_type = g_type_register_static( GTK_TYPE_DIALOG, "JsonEditor", &_json_editor_info, (GTypeFlags)0 );
    }

    return _json_editor_type;
}

// Initialization functions
static void json_editor_class_init( JsonEditorClass* pKlass )
{
    GObjectClass* gobject_class = G_OBJECT_CLASS( pKlass );

    // add private data
    g_type_class_add_private( pKlass, sizeof( JsonEditorPrivate ) );

}

static void json_editor_init( JsonEditor* pJsonEditor )
{
    JsonEditorPrivate* _pPriv = JSON_EDITOR_GET_PRIVATE( pJsonEditor );
    
    gtk_window_set_title( GTK_WINDOW( pJsonEditor ), "Json Editor" );
    gtk_widget_set_size_request( GTK_WIDGET( pJsonEditor ), 600, 400 ); 
    gtk_window_set_modal( GTK_WINDOW( pJsonEditor ) , TRUE );

    GtkWidget* _pContentArea = gtk_dialog_get_content_area( GTK_DIALOG( pJsonEditor ) );
    GtkWidget* _pScrolled_window = gtk_scrolled_window_new( FALSE, FALSE );
    _pPriv->pTextView_ = gtk_text_view_new();
    gtk_text_view_set_wrap_mode( GTK_TEXT_VIEW( _pPriv->pTextView_ ), GTK_WRAP_WORD_CHAR );
    gtk_container_add( GTK_CONTAINER( _pScrolled_window ), _pPriv->pTextView_ );

    gtk_scrolled_window_set_policy( GTK_SCROLLED_WINDOW( _pScrolled_window ), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC );
    gtk_box_pack_start( GTK_BOX( _pContentArea ), _pScrolled_window, TRUE, TRUE, 10 );

    gtk_dialog_add_buttons( GTK_DIALOG( pJsonEditor ), "OK", GTK_RESPONSE_OK, "Cancel", GTK_RESPONSE_CANCEL, NULL );

}

static gboolean json_editor_verify_json_string( JsonEditor* pJsonEditor, const gchar* text )
{
    gboolean _rc = ( text == NULL );
    if ( !_rc )
    {
        try
        {
            auto _json = nlohmann::json::parse( text );
            _rc = TRUE;
        }
        catch(const std::exception& e)
        {
             GtkWidget* _pDialog = gtk_dialog_new_with_buttons( "Parsing Error", GTK_WINDOW( pJsonEditor ), GTK_DIALOG_MODAL, 
                "_OK", GTK_RESPONSE_OK, NULL );

            gtk_box_pack_start( GTK_BOX( gtk_dialog_get_content_area( GTK_DIALOG( _pDialog ) ) ), 
                gtk_label_new( e.what() ), FALSE, TRUE, 10 );
            gtk_widget_show_all( _pDialog );
            gtk_dialog_run( GTK_DIALOG( _pDialog ) );

            gtk_widget_destroy( _pDialog );
            _rc = FALSE;
        }
    }

    return _rc;
}

// Public functions
GtkWidget* json_editor_new( GtkWindow* pParent, Logger* pLogger )
{
    GtkWidget* _pJsonEditor = GTK_WIDGET( g_object_new( json_editor_get_type(), NULL ) );
    JsonEditorPrivate* _pPriv = JSON_EDITOR_GET_PRIVATE( _pJsonEditor );
    _pPriv->pLogger_ = pLogger;

    gtk_window_set_transient_for( GTK_WINDOW( _pJsonEditor ), pParent );
    gtk_widget_show_all( GTK_WIDGET( _pJsonEditor ) );

    return _pJsonEditor;
} 

gchar* json_editor_run( JsonEditor* pJsonEditor, const gchar* text)
{
    JsonEditorPrivate* _pPriv = JSON_EDITOR_GET_PRIVATE( pJsonEditor ); 
    gchar* _newText = NULL;

    if ( json_editor_verify_json_string( pJsonEditor, text ) )
    {
        auto _json = nlohmann::json::parse( text );
        GtkTextBuffer* _buffer = gtk_text_view_get_buffer( GTK_TEXT_VIEW( _pPriv->pTextView_ ) );
        gchar* _prettyJson = JsonUtilities::pretty_format( &_json );
        
        gtk_text_buffer_set_text( _buffer, _prettyJson, -1 ); 
        g_free( _prettyJson );

        GtkTextIter _start, _end;
        do
        {
            g_free( _newText );
            if (GTK_RESPONSE_CANCEL == gtk_dialog_run( GTK_DIALOG( pJsonEditor ) ))
            {
                _newText = NULL;
            }
            else
            {
                gtk_text_buffer_get_start_iter( _buffer, &_start );     
                gtk_text_buffer_get_end_iter( _buffer, &_end );
                _newText = gtk_text_buffer_get_text( _buffer, &_start, &_end, TRUE );
            }
        }
        while ( !json_editor_verify_json_string( pJsonEditor, _newText ) );
    }

    return _newText;
}


