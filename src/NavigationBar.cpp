#include "NavigationBar.h"
#include "NavigationClient.h"

#define NAVIGATION_BAR_GET_PRIVATE( obj ) ( G_TYPE_INSTANCE_GET_PRIVATE( (obj), NAVIGATION_BAR_TYPE, NavigationBarPrivate ) )

typedef struct _NavigationBarPrivate NavigationBarPrivate;

struct _NavigationBarPrivate
{
    Logger*     pLogger_;
    NavigationClient*   pClient_;

    gint        minRecordNumber_;
    gint        currentRecordNumber_;
    gint        maxRecordNumber_;
    gint        displayedRecordNumber_;
    gboolean    autoScroll;

    GtkWidget*  pStartButton_;
    GtkWidget*  pSpinButton_;
    GtkWidget*  pEndButton_;
    GtkWidget*  pAutoScrollCheckBox_;
    gulong      spin_button_handler_id_; // handler id of OnSpinNavigationButtonValueChanged, which we use to block the signal
};

// signals emitted
enum
{
    TEXT_CHANGED_SIGNAL,
    LAST_SIGNAL
};

static guint navigation_bar_signals[LAST_SIGNAL] = { 0 };

// Private function prototypes
static void navigation_bar_class_init( NavigationBarClass* pKlass );
static void navigation_bar_init( NavigationBar* pNavigationBar );

static void navigation_bar_set_sensitive( NavigationBar* pNavigationBar );
static void navigation_bar_set_current_record_number( NavigationBar* pNavigationBar, int recordNumber );

static void OnStartButtonClicked( GtkButton* pButton, NavigationBar* pNavigationBar );
static void OnSpinButtonValueChanged( GtkSpinButton* pButton, NavigationBar* pNavigationBar );
static void OnEndButtonClicked( GtkButton* pButton, NavigationBar* pNavigationBar );
static void OnAutoScrollClicked( GtkToggleButton* pButton, NavigationBar* pNavigationBar ); 

// Type registration
GType navigation_bar_get_type( void )
{
    static GType _navigation_bar_type = 0;

    if ( !_navigation_bar_type )
    {
        static const GTypeInfo _navigation_bar_info =
        {
            sizeof( NavigationBarClass ),
            NULL,
            NULL,
            (GClassInitFunc) navigation_bar_class_init,
            NULL,
            NULL,
            sizeof( NavigationBar ),
            0,
            (GInstanceInitFunc) navigation_bar_init,
        };

        _navigation_bar_type = g_type_register_static( GTK_TYPE_BOX, "NavigationBar", &_navigation_bar_info, (GTypeFlags)0 );
    }

    return _navigation_bar_type;
}

// Initialization functions
static void navigation_bar_class_init( NavigationBarClass* pKlass )
{
    GObjectClass* gobject_class = G_OBJECT_CLASS( pKlass );  

    // add private data
    g_type_class_add_private( pKlass, sizeof( NavigationBarPrivate ) );

    // register the signals
    navigation_bar_signals[TEXT_CHANGED_SIGNAL] =
            g_signal_new( "text-changed", G_TYPE_FROM_CLASS( pKlass ),
            (GSignalFlags)(G_SIGNAL_RUN_FIRST | G_SIGNAL_ACTION),
            G_STRUCT_OFFSET( NavigationBarClass, text_changed ),
            NULL,
            NULL,
            g_cclosure_marshal_VOID__VOID,
            G_TYPE_NONE,
            0 );
}

static void navigation_bar_init( NavigationBar* pNavigationBar )
{
    NavigationBarPrivate* _pPriv = NAVIGATION_BAR_GET_PRIVATE( pNavigationBar );

    gtk_orientable_set_orientation( GTK_ORIENTABLE( pNavigationBar ), GTK_ORIENTATION_HORIZONTAL );
    gtk_box_set_spacing( GTK_BOX( pNavigationBar ), 10 );
    
    _pPriv->pStartButton_= gtk_button_new_with_label( "<<" );
    _pPriv->pSpinButton_= gtk_spin_button_new_with_range( 0, 0, 1 );
    _pPriv->pEndButton_= gtk_button_new_with_label( ">>" );
    _pPriv->pAutoScrollCheckBox_ = gtk_check_button_new_with_label( "Auto-scroll");
 
    gtk_box_pack_start( GTK_BOX( pNavigationBar ), _pPriv->pStartButton_, FALSE, TRUE, 0 );
    gtk_box_pack_start( GTK_BOX( pNavigationBar ), _pPriv->pSpinButton_, TRUE, TRUE, 0 );
    gtk_box_pack_start( GTK_BOX( pNavigationBar ), _pPriv->pEndButton_, FALSE, TRUE, 0 );

    _pPriv->spin_button_handler_id_ = g_signal_connect( G_OBJECT( _pPriv->pSpinButton_ ), "value-changed", 
        G_CALLBACK( OnSpinButtonValueChanged ), gpointer( pNavigationBar ) );
    g_signal_connect( G_OBJECT( _pPriv->pStartButton_ ), "clicked", G_CALLBACK( OnStartButtonClicked ), gpointer( pNavigationBar ) );
    g_signal_connect( G_OBJECT( _pPriv->pEndButton_ ), "clicked", G_CALLBACK( OnEndButtonClicked ), gpointer( pNavigationBar ) );

    navigation_bar_set_client( pNavigationBar, NULL );
}

static void navigation_bar_set_range( NavigationBar* pNavigationBar )
{
    NavigationBarPrivate* _pPriv = NAVIGATION_BAR_GET_PRIVATE( pNavigationBar );

    _pPriv->maxRecordNumber_ = _pPriv->pClient_ ? _pPriv->pClient_->get_max_item_number() : 0; 
    _pPriv->minRecordNumber_ = ( _pPriv->maxRecordNumber_  > 0 ) ? 1 : 0;
 
    // block signal in case current spin button value is out of range - we will set the value ourselves later
    g_signal_handler_block( G_OBJECT( _pPriv->pSpinButton_ ), _pPriv->spin_button_handler_id_ );
    gtk_spin_button_set_range( GTK_SPIN_BUTTON( _pPriv->pSpinButton_ ), _pPriv->minRecordNumber_, _pPriv->maxRecordNumber_ );
    g_signal_handler_unblock( G_OBJECT( _pPriv->pSpinButton_ ), _pPriv->spin_button_handler_id_ ); 

    navigation_bar_set_sensitive( pNavigationBar );
}

static void navigation_bar_set_sensitive( NavigationBar* pNavigationBar )
{
    NavigationBarPrivate* _pPriv = NAVIGATION_BAR_GET_PRIVATE( pNavigationBar );

    gtk_widget_set_sensitive( _pPriv->pStartButton_, _pPriv->currentRecordNumber_ > _pPriv->minRecordNumber_ );
    gtk_widget_set_sensitive( _pPriv->pStartButton_, _pPriv->maxRecordNumber_ > 1 );
    gtk_widget_set_sensitive( _pPriv->pEndButton_, _pPriv->currentRecordNumber_ < _pPriv->maxRecordNumber_ );
}

static void navigation_bar_set_current_record_number( NavigationBar* pNavigationBar, int recordNumber )
{
    NavigationBarPrivate* _pPriv = NAVIGATION_BAR_GET_PRIVATE( pNavigationBar );

    _pPriv->currentRecordNumber_ = recordNumber;
    if ( _pPriv->pClient_ )
    {
       _pPriv->pClient_->set_current_item_number( recordNumber ) ;
    }
    gtk_spin_button_set_value( GTK_SPIN_BUTTON( _pPriv->pSpinButton_ ), recordNumber );
 
    navigation_bar_set_sensitive( pNavigationBar );

    if ( _pPriv->displayedRecordNumber_ != recordNumber )
    {
        _pPriv->displayedRecordNumber_ = recordNumber;
        g_signal_emit_by_name( pNavigationBar, "text-changed" );
    }
}

static void OnStartButtonClicked( GtkButton* pButton, NavigationBar* pNavigationBar )
{
    NavigationBarPrivate* _pPriv = NAVIGATION_BAR_GET_PRIVATE( pNavigationBar );
    navigation_bar_set_current_record_number( pNavigationBar, _pPriv->minRecordNumber_ );  
}

static void OnSpinButtonValueChanged( GtkSpinButton* pButton, NavigationBar* pNavigationBar )
{
    NavigationBarPrivate* _pPriv = NAVIGATION_BAR_GET_PRIVATE( pNavigationBar );

    gint spinButtonValue = gtk_spin_button_get_value_as_int( GTK_SPIN_BUTTON( _pPriv->pSpinButton_ ) );
    if ( _pPriv->currentRecordNumber_ != spinButtonValue )
    {
        navigation_bar_set_current_record_number( pNavigationBar, spinButtonValue );
    }
}

static void OnEndButtonClicked( GtkButton* pButton, NavigationBar* pNavigationBar )
{
    NavigationBarPrivate* _pPriv = NAVIGATION_BAR_GET_PRIVATE( pNavigationBar );
    navigation_bar_set_current_record_number( pNavigationBar, _pPriv->maxRecordNumber_ );  
}

static void OnAutoScrollClicked( GtkToggleButton* pButton, NavigationBar* pNavigationBar )
{
    NavigationBarPrivate* _pPriv = NAVIGATION_BAR_GET_PRIVATE( pNavigationBar );
    _pPriv->autoScroll = gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON( _pPriv->pAutoScrollCheckBox_ ) );

    if ( _pPriv->autoScroll )
    {
        navigation_bar_set_current_record_number( pNavigationBar, _pPriv->maxRecordNumber_ );         
    }
}

// Public functions
void navigation_bar_on_client_item_added( NavigationBar* pNavigationBar )
{
    NavigationBarPrivate* _pPriv = NAVIGATION_BAR_GET_PRIVATE( pNavigationBar );  

    navigation_bar_set_range( pNavigationBar );
    if ( _pPriv->autoScroll )
    {
        navigation_bar_set_current_record_number( pNavigationBar, _pPriv->maxRecordNumber_ );
    }
    else
    {
        if ( _pPriv->currentRecordNumber_ == 0 )
        {
            navigation_bar_set_current_record_number( pNavigationBar, 1 );
        }
    }
}

GtkWidget* navigation_bar_new(  Logger* pLogger, gboolean autoScroll )
{
    GtkWidget* _pBox = GTK_WIDGET( g_object_new( navigation_bar_get_type(), NULL ) );
    NavigationBarPrivate* _pPriv = NAVIGATION_BAR_GET_PRIVATE( _pBox ); 

    if ( autoScroll )
    {
        gtk_box_pack_start( GTK_BOX( _pBox ), _pPriv->pAutoScrollCheckBox_, FALSE, FALSE, 0 );
        g_signal_connect( G_OBJECT( _pPriv->pAutoScrollCheckBox_ ), "toggled", G_CALLBACK( OnAutoScrollClicked ), gpointer( _pBox ) );
        gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON(  _pPriv->pAutoScrollCheckBox_  ), TRUE );
    }

    _pPriv->pLogger_ = pLogger; 
    return _pBox;
} 

void navigation_bar_set_client( NavigationBar* pNavigationBar, NavigationClient* pClient )
{
    NavigationBarPrivate* _pPriv = NAVIGATION_BAR_GET_PRIVATE( pNavigationBar );

    _pPriv ->displayedRecordNumber_ = -1;
    if ( _pPriv->pClient_ )
    {
        _pPriv->pClient_->set_status( NavigationClient::INACTIVE );
    }

    _pPriv->pClient_ = pClient;
    navigation_bar_set_range( pNavigationBar );

    if ( pClient )
    {
        _pPriv->pClient_->set_status( NavigationClient::ACTIVE );      
        navigation_bar_set_current_record_number( pNavigationBar,
            _pPriv->autoScroll ? _pPriv->pClient_->get_max_item_number() : _pPriv->pClient_->get_current_item_number() );  
    } 
    else
    {
        navigation_bar_set_current_record_number( pNavigationBar, 0 );  
    }
}

NavigationClient* navigation_bar_get_client( NavigationBar* pNavigationBar )
{
    NavigationBarPrivate* _pPriv = NAVIGATION_BAR_GET_PRIVATE( pNavigationBar );
    return _pPriv->pClient_;
}

gint navigation_bar_get_current_record_number( NavigationBar* pNavigationBar )
{
     NavigationBarPrivate* _pPriv = NAVIGATION_BAR_GET_PRIVATE( pNavigationBar );
    return _pPriv->currentRecordNumber_;   
}