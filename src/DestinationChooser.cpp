#include "DestinationChooser.h"
#include "DestinationListStore.h"
#include "DestinationMap.h"
#include "Logger.h"

#define DESTINATION_CHOOSER_GET_PRIVATE( obj ) ( G_TYPE_INSTANCE_GET_PRIVATE( (obj), DESTINATION_CHOOSER_TYPE, DestinationChooserPrivate ) )

typedef struct _DestinationChooserPrivate DestinationChooserPrivate;

struct _DestinationChooserPrivate
{
    Parameters* pParms_;
    Logger* pLogger_;
    DestinationListStore* pListStore_;
    GtkWidget* pDestinationPrefixComboBox_;
    GtkWidget* pTableNumberSpinButton_;   
};

// signals emitted
enum
{
    CHANGED_SIGNAL,
    DESTINATION_CHANGED_SIGNAL,
    LAST_SIGNAL
};

static guint destination_chooser_signals[LAST_SIGNAL] = { 0 };

// Private function prototypes
static void destination_chooser_class_init( DestinationChooserClass* pKlass );
static void destination_chooser_init( DestinationChooser* pDestinationChooser );
static void destination_chooser_on_combo_box_changed( GtkComboBox* pComboBox, DestinationChooser* pDestinationChooser );
static void destination_chooser_on_spin_button_value_changed( GtkSpinButton* pSpinButton, DestinationChooser* pDestinationChooser );
static gboolean destination_chooser_is_table_destination( DestinationChooser* pDestinationChooser );

// Type registration
GType destination_chooser_get_type( void )
{
    static GType _destination_chooser_type = 0;

    if ( !_destination_chooser_type )
    {
        static const GTypeInfo _destination_chooser_info =
        {
            sizeof( DestinationChooserClass ),
            NULL,
            NULL,
            (GClassInitFunc) destination_chooser_class_init,
            NULL,
            NULL,
            sizeof( DestinationChooser ),
            0,
            (GInstanceInitFunc) destination_chooser_init,
        };

        _destination_chooser_type = g_type_register_static( GTK_TYPE_BOX, "DestinationChooser", &_destination_chooser_info, (GTypeFlags)0 );
    }

    return _destination_chooser_type;
}

// Initialization functions
static void destination_chooser_class_init( DestinationChooserClass* pKlass )
{
    GObjectClass* gobject_class = G_OBJECT_CLASS( pKlass );  

    // add private data
    g_type_class_add_private( pKlass, sizeof( DestinationChooserPrivate ) );

    // register the signals
    destination_chooser_signals[CHANGED_SIGNAL] =
            g_signal_new( "prefix-changed", G_TYPE_FROM_CLASS( pKlass ),
            (GSignalFlags)(G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION),
            G_STRUCT_OFFSET( DestinationChooserClass, prefix_changed ),
            NULL,
            NULL,
            g_cclosure_marshal_VOID__VOID,
            G_TYPE_NONE,
            0 );
    destination_chooser_signals[CHANGED_SIGNAL] =
            g_signal_new( "destination-changed", G_TYPE_FROM_CLASS( pKlass ),
            (GSignalFlags)(G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION),
            G_STRUCT_OFFSET( DestinationChooserClass, destination_changed ),
            NULL,
            NULL,
            g_cclosure_marshal_VOID__VOID,
            G_TYPE_NONE,
            0 );
}

static void destination_chooser_init( DestinationChooser* pDestinationChooser )
{
    DestinationChooserPrivate* _pPriv = DESTINATION_CHOOSER_GET_PRIVATE( pDestinationChooser );

    gtk_orientable_set_orientation( GTK_ORIENTABLE( pDestinationChooser ), GTK_ORIENTATION_HORIZONTAL );
    gtk_box_set_spacing( GTK_BOX( pDestinationChooser ), 10 );
    
    _pPriv->pDestinationPrefixComboBox_ = gtk_combo_box_text_new();
    _pPriv->pTableNumberSpinButton_ = gtk_spin_button_new_with_range( 1, 100000, 1 );

    gtk_box_pack_start( GTK_BOX( pDestinationChooser ), _pPriv->pDestinationPrefixComboBox_, FALSE, TRUE, 0 );
    gtk_box_pack_start( GTK_BOX( pDestinationChooser ), gtk_label_new( "Table no: " ), FALSE, TRUE, 0 );
    gtk_box_pack_start( GTK_BOX( pDestinationChooser ), _pPriv->pTableNumberSpinButton_, FALSE, TRUE, 0 );

    g_signal_connect( G_OBJECT( _pPriv->pDestinationPrefixComboBox_ ), "changed", G_CALLBACK( destination_chooser_on_combo_box_changed ), gpointer( pDestinationChooser ) );
    g_signal_connect( G_OBJECT( _pPriv->pTableNumberSpinButton_ ), "value-changed", G_CALLBACK( destination_chooser_on_spin_button_value_changed ), gpointer( pDestinationChooser ) );
}

static gboolean destination_chooser_is_table_destination( DestinationChooser* pDestinationChooser )
{
    gchar* _prefix = destination_chooser_get_prefix( pDestinationChooser );
    gboolean _rc = FALSE;
    if ( _prefix )
    {
        _rc = g_str_has_suffix( _prefix, "tb" );
    }
    g_free( _prefix );
    return _rc;
}

static void destination_chooser_on_combo_box_changed( GtkComboBox* comboBox, DestinationChooser* pDestinationChooser )
{
    g_signal_emit_by_name( G_OBJECT( pDestinationChooser ), "prefix-changed" );
}

static void destination_chooser_on_spin_button_value_changed( GtkSpinButton* spinButton, DestinationChooser* pDestinationChooser )
{
    if ( destination_chooser_is_table_destination( pDestinationChooser ) )
    {
        g_signal_emit_by_name( G_OBJECT( pDestinationChooser ), "destination-changed" );
    }
}

// Public functions
GtkWidget* destination_chooser_new( Parameters* pParms, Logger* pLogger, DestinationMap* pDestinationMap, const char* defaultDestination )
{
    GtkWidget* _pBox = GTK_WIDGET( g_object_new( destination_chooser_get_type(), NULL ) );
    DestinationChooserPrivate* _pPriv = DESTINATION_CHOOSER_GET_PRIVATE( _pBox );
    _pPriv->pParms_ = pParms;
    _pPriv->pLogger_ = pLogger;

    // add model- must be done here because we did not have pLogger during template_chooser_init

    _pPriv->pListStore_ = destination_list_store_new( _pPriv->pLogger_ );
    gtk_combo_box_set_model( GTK_COMBO_BOX( _pPriv->pDestinationPrefixComboBox_ ), GTK_TREE_MODEL( _pPriv->pListStore_ ) );
    gint _defaultIndex = -1;
    destination_list_store_import_map( _pPriv->pListStore_, pDestinationMap, defaultDestination, &_defaultIndex );
    g_object_unref(  _pPriv->pListStore_ );

    if ( _defaultIndex >= 0)
    {
        gtk_combo_box_set_active( GTK_COMBO_BOX( _pPriv->pDestinationPrefixComboBox_ ), _defaultIndex );
    }

    return _pBox;
} 

gchar* destination_chooser_get_prefix( DestinationChooser* pDestinationChooser )
{
    DestinationChooserPrivate* _pPriv = DESTINATION_CHOOSER_GET_PRIVATE( pDestinationChooser );
    return gtk_combo_box_text_get_active_text( GTK_COMBO_BOX_TEXT( _pPriv->pDestinationPrefixComboBox_ ) );
}

DestinationId destination_chooser_get_destination_id( DestinationChooser* pDestinationChooser )
{
    DestinationChooserPrivate* _pPriv = DESTINATION_CHOOSER_GET_PRIVATE( pDestinationChooser );
    GtkTreeIter _iter;
    gchar* _prefix = NULL;
    gint _destinationType;
    gchar* _destinationNameTemplate = NULL;
    DestinationId _destinationId;

    if ( gtk_combo_box_get_active_iter( GTK_COMBO_BOX( _pPriv->pDestinationPrefixComboBox_ ), &_iter ) )
    {
        destination_list_store_get_values( _pPriv->pListStore_, &_iter, &_prefix, &_destinationType, &_destinationNameTemplate );

        _destinationId.destinationName_ =  DestinationId::build_destination_name( 
            _prefix, 
            _pPriv->pParms_->get_tourney_id(), 
            gtk_spin_button_get_value_as_int( GTK_SPIN_BUTTON( _pPriv->pTableNumberSpinButton_ ) ), 
            _destinationNameTemplate );
         _destinationId.destinationType_ = static_cast<DestinationType>( _destinationType );
    }
;
    g_free( _prefix );    
    return _destinationId;
}

gint destination_chooser_get_prefix_id( DestinationChooser* pDestinationChooser )
{
    DestinationChooserPrivate* _pPriv = DESTINATION_CHOOSER_GET_PRIVATE( pDestinationChooser );
    return gtk_combo_box_get_active( GTK_COMBO_BOX( _pPriv->pDestinationPrefixComboBox_ ) );

}

void destination_chooser_set_prefix_id( DestinationChooser* pDestinationChooser, gint newPrefix )
{
    DestinationChooserPrivate* _pPriv = DESTINATION_CHOOSER_GET_PRIVATE( pDestinationChooser );
    gint _currentPrefix = destination_chooser_get_prefix_id( pDestinationChooser );
    if ( _currentPrefix != newPrefix )
    {
        gtk_combo_box_set_active( GTK_COMBO_BOX( _pPriv->pDestinationPrefixComboBox_ ), (guint) newPrefix );
    }
} 