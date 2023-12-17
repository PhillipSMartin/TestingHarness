#include "TourneyIdChooser.h"

#define TOURNEY_ID_CHOOSER_GET_PRIVATE( obj ) ( G_TYPE_INSTANCE_GET_PRIVATE( (obj), TOURNEY_ID_CHOOSER_TYPE, TourneyIdChooserPrivate ) )

typedef struct _TourneyIdChooserPrivate TourneyIdChooserPrivate;

struct _TourneyIdChooserPrivate
{
    GtkWidget* pTourneyIdEntry_;
    GtkWidget* pUpdateButton_; 
    GtkWidget* pRevertButton_;

    gchar* currentTourneyId_;  
};

// signals emitted
enum
{
    CHANGED_SIGNAL,
    LAST_SIGNAL
};

static guint tourney_id_chooser_signals[LAST_SIGNAL] = { 0 };

// Private function prototypes
static void tourney_id_chooser_class_init( TourneyIdChooserClass* pKlass );
static void tourney_id_chooser_init( TourneyIdChooser* pTourneyIdChooser );
static void tourney_id_chooser_on_changed( GtkCellEditable* pCell, TourneyIdChooser* pTourneyIdChooser  );
static void tourney_id_chooser_on_update_button_clicked( GtkButton* pButton, TourneyIdChooser* pTourneyIdChooser );
static void tourney_id_chooser_on_revert_button_clicked( GtkButton* pButton, TourneyIdChooser* pTourneyIdChooser );

// Type registration
GType tourney_id_chooser_get_type( void )
{
    static GType _tourney_id_chooser_type = 0;

    if ( !_tourney_id_chooser_type )
    {
        static const GTypeInfo _tourney_id_chooser_info =
        {
            sizeof( TourneyIdChooserClass ),
            NULL,
            NULL,
            (GClassInitFunc) tourney_id_chooser_class_init,
            NULL,
            NULL,
            sizeof( TourneyIdChooser ),
            0,
            (GInstanceInitFunc) tourney_id_chooser_init,
        };

        _tourney_id_chooser_type = g_type_register_static( GTK_TYPE_BOX, "TourneyIdChooser", &_tourney_id_chooser_info, (GTypeFlags)0 );
    }

    return _tourney_id_chooser_type;
}

// Initialization functions
static void tourney_id_chooser_class_init( TourneyIdChooserClass* pKlass )
{
    GObjectClass* gobject_class = G_OBJECT_CLASS( pKlass );  

    // add private data
    g_type_class_add_private( pKlass, sizeof( TourneyIdChooserPrivate ) );

    // register the signals
    tourney_id_chooser_signals[CHANGED_SIGNAL] =
            g_signal_new( "changed", G_TYPE_FROM_CLASS( pKlass ),
            (GSignalFlags)(G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION),
            G_STRUCT_OFFSET( TourneyIdChooserClass, changed ),
            NULL,
            NULL,
            g_cclosure_marshal_VOID__VOID,
            G_TYPE_NONE,
            0 );
}

static void tourney_id_chooser_init( TourneyIdChooser* pTourneyIdChooser )
{
    TourneyIdChooserPrivate* _pPriv = TOURNEY_ID_CHOOSER_GET_PRIVATE( pTourneyIdChooser );

    gtk_orientable_set_orientation( GTK_ORIENTABLE( pTourneyIdChooser ), GTK_ORIENTATION_HORIZONTAL );
    gtk_box_set_spacing( GTK_BOX( pTourneyIdChooser ), 10 );
    
    _pPriv->pTourneyIdEntry_ = gtk_entry_new();
    _pPriv->pUpdateButton_ = gtk_button_new_with_label( "Update" );
    _pPriv->pRevertButton_ = gtk_button_new_with_label( "Revert" );

    gtk_box_pack_start( GTK_BOX( pTourneyIdChooser ), gtk_label_new( "Tid:" ), FALSE, TRUE, 0 );
    gtk_box_pack_start( GTK_BOX( pTourneyIdChooser ), _pPriv->pTourneyIdEntry_, TRUE, TRUE, 0);
    gtk_box_pack_start( GTK_BOX( pTourneyIdChooser ), _pPriv->pUpdateButton_, FALSE, TRUE, 0 );
    gtk_box_pack_start( GTK_BOX( pTourneyIdChooser ), _pPriv->pRevertButton_, FALSE, TRUE, 0 );

    g_signal_connect( G_OBJECT( _pPriv->pTourneyIdEntry_ ), "changed", G_CALLBACK( tourney_id_chooser_on_changed ), gpointer( pTourneyIdChooser ) );
    g_signal_connect( G_OBJECT( _pPriv->pUpdateButton_ ), "clicked", G_CALLBACK( tourney_id_chooser_on_update_button_clicked ), gpointer( pTourneyIdChooser ) );
    g_signal_connect( G_OBJECT( _pPriv->pRevertButton_ ), "clicked", G_CALLBACK( tourney_id_chooser_on_revert_button_clicked ), gpointer( pTourneyIdChooser ) );
}

static void tourney_id_chooser_on_changed( GtkCellEditable* pCell, TourneyIdChooser* pTourneyIdChooser )
{
    TourneyIdChooserPrivate* _pPriv = TOURNEY_ID_CHOOSER_GET_PRIVATE( pTourneyIdChooser );  
    gtk_widget_set_sensitive( _pPriv->pUpdateButton_, TRUE );
    gtk_widget_set_sensitive( _pPriv->pRevertButton_, TRUE );
}

static void tourney_id_chooser_on_update_button_clicked( GtkButton* pButton, TourneyIdChooser* pTourneyIdChooser )
{
    TourneyIdChooserPrivate* _pPriv = TOURNEY_ID_CHOOSER_GET_PRIVATE( pTourneyIdChooser );  

    g_free( _pPriv->currentTourneyId_ );
    _pPriv->currentTourneyId_ = g_strdup( gtk_entry_get_text( GTK_ENTRY( _pPriv->pTourneyIdEntry_ ) ) );
    gtk_widget_set_sensitive( _pPriv->pUpdateButton_, FALSE );
    gtk_widget_set_sensitive( _pPriv->pRevertButton_, FALSE );
    g_signal_emit_by_name( G_OBJECT( pTourneyIdChooser ), "changed" );
}


static void tourney_id_chooser_on_revert_button_clicked( GtkButton* pButton, TourneyIdChooser* pTourneyIdChooser )
{
    TourneyIdChooserPrivate* _pPriv = TOURNEY_ID_CHOOSER_GET_PRIVATE( pTourneyIdChooser );  
  
    gtk_entry_set_text( GTK_ENTRY( _pPriv->pTourneyIdEntry_ ), _pPriv->currentTourneyId_ );;
    gtk_widget_set_sensitive( _pPriv->pUpdateButton_, FALSE );
    gtk_widget_set_sensitive( _pPriv->pRevertButton_, FALSE );
}

// Public functions
GtkWidget* tourney_id_chooser_new(  const gchar* tourneyId )
{
    GtkWidget* _pBox = GTK_WIDGET( g_object_new( tourney_id_chooser_get_type(), NULL ) );
    TourneyIdChooserPrivate* _pPriv = TOURNEY_ID_CHOOSER_GET_PRIVATE( _pBox );  

    gtk_entry_set_text( GTK_ENTRY( _pPriv->pTourneyIdEntry_ ), tourneyId );
    _pPriv->currentTourneyId_ = g_strdup( tourneyId );
    gtk_widget_set_sensitive( _pPriv->pUpdateButton_, FALSE );
    gtk_widget_set_sensitive( _pPriv->pRevertButton_, FALSE );

    return _pBox;
} 

const gchar* tourney_id_chooser_get_tourney_id( TourneyIdChooser* pTourneyIdChooser )
{
    TourneyIdChooserPrivate* _pPriv = TOURNEY_ID_CHOOSER_GET_PRIVATE( pTourneyIdChooser );
    return gtk_entry_get_text( GTK_ENTRY( _pPriv->pTourneyIdEntry_ ) );
}

void tourney_id_chooser_set_tourney_id( TourneyIdChooser* pTourneyIdChooser, const gchar* tourneyId )
{
    TourneyIdChooserPrivate* _pPriv = TOURNEY_ID_CHOOSER_GET_PRIVATE( pTourneyIdChooser ); 

    if ( g_strcmp0( gtk_entry_get_text( GTK_ENTRY( _pPriv->pTourneyIdEntry_ ) ), tourneyId )!= 0 )
    {
        gtk_entry_set_text( GTK_ENTRY( _pPriv->pTourneyIdEntry_ ), tourneyId );
        g_signal_emit_by_name( G_OBJECT( pTourneyIdChooser ), "changed" );
    }
}