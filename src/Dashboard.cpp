#include "Dashboard.h"

#include "About.h"
#include "DestinationMap.h"
#include "GtkUtilities.h"
#include "Logger.h"
#include "LogParser.h"
#include "MessageStore.h"
#include "Test.h"

// TODO - don't print message if destination doesn't change in set_publish_destination


Dashboard::Dashboard(Parameters* pParms, Logger* pLogger, DestinationMap* pDestinationMap, AMQClient* pAMQClient) :
    pParms_{ pParms },
    pLogger_{ pLogger },
    pDestinationMap_{ pDestinationMap },
    pWidgets_{ (WidgetVault*) g_malloc (sizeof (WidgetVault)) },
    pAMQClient_{ pAMQClient }
{
    instantiate_widgets();
    arrange_widgets();
    wire_signals();

    // populate template combo box
    g_signal_emit_by_name( G_OBJECT( pWidgets_->pPublishDestinationChooser_ ), "prefix-changed" );

    // turn on auto-scroll
    gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON( pWidgets_->pConsoleAutoScrollCheckBox_ ), TRUE );
}

Dashboard::~Dashboard()
{
    g_free( pWidgets_ );
    for (auto& m : messageMall_ )
    {
        delete m.second;
    }
    messageMall_.clear();
    if ( pTest_ )
    {
        delete pTest_;
    }   
}

// private methods
// initialization
void Dashboard::instantiate_widgets()
{
    // main window
    pWidgets_->pMainWindow_ = gtk_window_new( GTK_WINDOW_TOPLEVEL );

    // panes
    pWidgets_->pVerticalPane_ = gtk_paned_new( GTK_ORIENTATION_VERTICAL );
    pWidgets_->pHorizontalPane_ = gtk_paned_new( GTK_ORIENTATION_HORIZONTAL );
    pWidgets_->pJsonViewerPane_ = gtk_paned_new( GTK_ORIENTATION_HORIZONTAL );
 
    // viewers
    pWidgets_->pSubscribeJsonViewer_ = json_viewer_new( pLogger_, TRUE );
    pWidgets_->pPublishJsonViewer_ = json_viewer_new( pLogger_, FALSE );
    pWidgets_->pConsoleViewer_ = console_viewer_new( pLogger_ ); 
    pWidgets_->pHelpViewer_ = help_viewer_new();

    // choosers
    pWidgets_->pSubscribeDestinationChooser_ = destination_chooser_new( pParms_, pLogger_, pDestinationMap_, "State.tb" );
    pWidgets_->pPublishDestinationChooser_ = destination_chooser_new(pParms_, pLogger_, pDestinationMap_, "Event.tb" );
    pWidgets_->pTemplateChooser_ = template_chooser_new( pLogger_, pParms_->get_working_directory() );
    pWidgets_->pTourneyIdChooser_ = tourney_id_chooser_new( pParms_->get_tourney_id() );

    // buttons
    pWidgets_->pSubscribeButton_ = gtk_toggle_button_new_with_label( "Subscribe" );
    pWidgets_->pPublishButton_ = gtk_button_new_with_label( "Publish" );
    pWidgets_->pSaveTemplateButton_ = gtk_button_new_with_label( "Save" );
    pWidgets_->pSaveNewTemplateButton_ = gtk_button_new_with_label( "Save As..." );
    pWidgets_->pDeleteTemplateButton_ = gtk_button_new_with_label( "Delete" );
    pWidgets_->pEditJsonButton_ = gtk_button_new_with_label( "Edit Json..." );
    pWidgets_->pSubscribeNavigationBar_ = navigation_bar_new( pLogger_ );
    pWidgets_->pPublishNavigationBar_ = navigation_bar_new( pLogger_, FALSE /* no autoscroll checkbox */ );

    // checkboxes
    pWidgets_->pConsoleAutoScrollCheckBox_ = gtk_check_button_new_with_label( "Auto-scroll console");
    pWidgets_->pRunScriptCheckBox_ = gtk_check_button_new_with_label( "Run script");

    // menu items
    pWidgets_->pAboutMenuItem_ = gtk_menu_item_new_with_label( "About" );
    pWidgets_->pImportScriptMenuItem_ = gtk_menu_item_new_with_label( "Import script file..." );
    pWidgets_->pImportLogMenuItem_ = gtk_menu_item_new_with_label( "Import log file..." );
    pWidgets_->pExportLogMenuItem_ = gtk_menu_item_new_with_label( "Export log file as script..." );

    // panels
    pWidgets_->pSubscribePanel_ = subscribe_panel_new();    
    pWidgets_->pPublishPanel_ = publish_panel_new();    
    pWidgets_->pControlPanel_ = control_panel_new();
}

void Dashboard::arrange_widgets()
{
    // main window
    gtk_window_set_title( GTK_WINDOW( pWidgets_->pMainWindow_ ), "Swan Games Testing Harness" );
    gtk_container_set_border_width( GTK_CONTAINER( pWidgets_->pMainWindow_ ), 10 ); 

    // menu bar
    GtkWidget* _pBox = gtk_box_new( GTK_ORIENTATION_VERTICAL, 0 );
    gtk_container_add( GTK_CONTAINER( pWidgets_->pMainWindow_ ), _pBox ); 
    gtk_container_add( GTK_CONTAINER( _pBox ), menu_bar_new() ); 
 
    // vertical pane
    gtk_widget_set_size_request( pWidgets_->pVerticalPane_, 1200, 800 ); 
    gtk_container_add( GTK_CONTAINER( _pBox ), pWidgets_->pVerticalPane_ );
    gtk_paned_add1( GTK_PANED( pWidgets_->pVerticalPane_ ), pWidgets_->pHorizontalPane_ );
    gtk_paned_add2( GTK_PANED( pWidgets_->pVerticalPane_  ), pWidgets_->pConsoleViewer_ );
    gtk_paned_set_position( GTK_PANED( pWidgets_->pVerticalPane_ ) , 647 );

    // two left panes and one right pane
    gtk_paned_add1( GTK_PANED( pWidgets_->pHorizontalPane_ ), pWidgets_->pJsonViewerPane_ );
    gtk_paned_add2( GTK_PANED( pWidgets_->pHorizontalPane_  ), pWidgets_->pControlPanel_ );
    gtk_paned_set_position( GTK_PANED( pWidgets_->pHorizontalPane_ ) , 820 );  
 
    // left panes
    gtk_paned_add1( GTK_PANED( pWidgets_->pJsonViewerPane_ ), pWidgets_->pSubscribePanel_ );
    gtk_paned_add2( GTK_PANED( pWidgets_->pJsonViewerPane_  ), pWidgets_->pPublishPanel_ );
    gtk_paned_set_position( GTK_PANED( pWidgets_->pJsonViewerPane_ ) , 400 );   
}

void Dashboard::wire_signals()
{
    g_signal_connect( G_OBJECT( pWidgets_->pSubscribeDestinationChooser_ ), "prefix-changed", G_CALLBACK( OnSubscribeDestinationChanged ), gpointer( this ) ); //same callback for either event
    g_signal_connect( G_OBJECT( pWidgets_->pSubscribeDestinationChooser_ ), "destination-changed", G_CALLBACK( OnSubscribeDestinationChanged ), gpointer( this ) ); //same callback for either event
    g_signal_connect( G_OBJECT( pWidgets_->pPublishDestinationChooser_ ), "prefix-changed", G_CALLBACK( OnPublishPrefixChanged ), gpointer( this ) );
    g_signal_connect( G_OBJECT( pWidgets_->pPublishDestinationChooser_ ), "destination-changed", G_CALLBACK( OnPublishDestinationChanged ), gpointer( this ) );
    g_signal_connect( G_OBJECT( pWidgets_->pTemplateChooser_ ), "changed", G_CALLBACK( OnTemplateChanged ), gpointer( this ) ); 
    g_signal_connect( G_OBJECT( pWidgets_->pTourneyIdChooser_), "changed", G_CALLBACK( OnTourneyIdChanged ), gpointer( this ) ); 
 
    subscribe_button_handler_id_ = g_signal_connect( G_OBJECT( pWidgets_->pSubscribeButton_ ), "toggled", G_CALLBACK( OnSubscribeButtonClicked ), gpointer( this ) );
    g_signal_connect( G_OBJECT( pWidgets_->pSubscribeButton_ ), "query-tooltip", G_CALLBACK( OnSubscribeButtonToolTip ), gpointer( this ) );
    g_signal_connect( G_OBJECT( pWidgets_->pPublishButton_ ), "clicked", G_CALLBACK( OnPublishButtonClicked ), gpointer( this ) );
    g_signal_connect( G_OBJECT( pWidgets_->pSaveTemplateButton_ ), "clicked", G_CALLBACK( OnSaveButtonClicked ), gpointer( this ) );
    g_signal_connect( G_OBJECT( pWidgets_->pSaveNewTemplateButton_ ), "clicked", G_CALLBACK( OnSaveAsButtonClicked ), gpointer( this ) );
    g_signal_connect( G_OBJECT( pWidgets_->pDeleteTemplateButton_ ), "clicked", G_CALLBACK( OnDeleteButtonClicked ), gpointer( this ) );
    g_signal_connect( G_OBJECT( pWidgets_->pEditJsonButton_ ), "clicked", G_CALLBACK( OnEditJsonButtonClicked ), gpointer( this ) );
    g_signal_connect( G_OBJECT( pWidgets_->pSubscribeNavigationBar_ ), "text-changed", G_CALLBACK( OnSubscribeNavigationBarTextChanged ), gpointer( this ) );
    g_signal_connect( G_OBJECT( pWidgets_->pPublishNavigationBar_ ), "text-changed", G_CALLBACK( OnPublishNavigationBarTextChanged ), gpointer( this ) );

    g_signal_connect( G_OBJECT( pWidgets_->pConsoleAutoScrollCheckBox_ ), "toggled", G_CALLBACK( OnConsoleAutoScrollClicked ), gpointer( this ) );
    run_script_check_box_handler_id_ = g_signal_connect( G_OBJECT( pWidgets_->pRunScriptCheckBox_ ), "toggled", G_CALLBACK( OnRunScriptClicked ), gpointer( this ) );

    g_signal_connect( G_OBJECT( pWidgets_->pImportScriptMenuItem_) , "activate", G_CALLBACK( OnImportScriptMenuItemActivated ), gpointer( this ) );
    g_signal_connect( G_OBJECT( pWidgets_->pImportLogMenuItem_) , "activate", G_CALLBACK( OnImportLogMenuItemActivated ), gpointer( this ) );
    g_signal_connect( G_OBJECT( pWidgets_->pExportLogMenuItem_) , "activate", G_CALLBACK( OnExportLogMenuItemActivated ), gpointer( this ) );
    g_signal_connect( G_OBJECT( pWidgets_->pAboutMenuItem_) , "activate", G_CALLBACK( OnAboutMenuItemActivated ), gpointer( this ) );
 
    g_signal_connect( G_OBJECT( pWidgets_->pPublishJsonViewer_ ), "json-edited", G_CALLBACK( OnJsonEdited ), gpointer( this ) );
    g_signal_connect( G_OBJECT( pWidgets_->pPublishJsonViewer_ ), "json-replaced", G_CALLBACK( OnJsonReplaced ), gpointer( this ) );
    g_signal_connect( G_OBJECT( pWidgets_->pHelpViewer_ ), "edited", G_CALLBACK( OnHelpEdited ), gpointer( this ) );

    g_signal_connect( G_OBJECT( pWidgets_->pMainWindow_ ), "destroy", G_CALLBACK( OnMainWindowDestroy ), gpointer( this ) ); 
    g_signal_connect( G_OBJECT( pWidgets_->pMainWindow_ ), "delete-event", G_CALLBACK( OnMainWindowDelete ), gpointer( this ) ); 
}

GtkWidget* Dashboard::subscribe_panel_new()
{
    GtkWidget* _pSubscribePanel = gtk_box_new( GTK_ORIENTATION_VERTICAL, 10 ); 
    gtk_container_set_border_width( GTK_CONTAINER( _pSubscribePanel ), 10 );
    
    gtk_box_pack_start( GTK_BOX( _pSubscribePanel ), pWidgets_->pSubscribeButton_, FALSE, TRUE, 0 );
    gtk_box_pack_start( GTK_BOX( _pSubscribePanel ), pWidgets_->pSubscribeDestinationChooser_, FALSE, TRUE, 0 );
    gtk_box_pack_start( GTK_BOX( _pSubscribePanel ), pWidgets_->pSubscribeJsonViewer_, TRUE, TRUE, 0 );
    gtk_box_pack_start( GTK_BOX( _pSubscribePanel ), pWidgets_->pSubscribeNavigationBar_, FALSE, TRUE, 0 );

    gtk_widget_set_has_tooltip(  pWidgets_->pSubscribeButton_, TRUE );

    return _pSubscribePanel;
}

GtkWidget* Dashboard::publish_panel_new()
{
    GtkWidget* _pPublishPanel = gtk_box_new( GTK_ORIENTATION_VERTICAL, 10 ); 
    gtk_container_set_border_width( GTK_CONTAINER( _pPublishPanel ), 10 );
;
    gtk_box_pack_start( GTK_BOX( _pPublishPanel ), pWidgets_->pPublishButton_, FALSE, TRUE, 0 );
    gtk_box_pack_start( GTK_BOX( _pPublishPanel ), pWidgets_->pPublishDestinationChooser_, FALSE, TRUE, 0 );
    gtk_box_pack_start( GTK_BOX( _pPublishPanel ), pWidgets_->pPublishJsonViewer_, TRUE, TRUE, 0 );

    GtkWidget* _pBox = gtk_box_new( GTK_ORIENTATION_HORIZONTAL, 10 );
    gtk_box_pack_start( GTK_BOX( _pBox ), pWidgets_->pPublishNavigationBar_, TRUE, TRUE, 0 );
    gtk_box_pack_start( GTK_BOX( _pBox ), pWidgets_->pRunScriptCheckBox_, FALSE, FALSE, 0 );
    gtk_box_pack_start( GTK_BOX( _pPublishPanel ), _pBox, FALSE, TRUE, 0 );

    gtk_widget_set_sensitive( pWidgets_->pPublishButton_, FALSE );
    gtk_widget_set_sensitive( pWidgets_->pPublishNavigationBar_, FALSE ); 
    gtk_widget_set_sensitive( pWidgets_->pRunScriptCheckBox_, FALSE) ;

    return _pPublishPanel;
}

GtkWidget* Dashboard::control_panel_new()
{
    GtkWidget* _pControlPanel = gtk_box_new( GTK_ORIENTATION_VERTICAL, 10 ); 
    gtk_container_set_border_width( GTK_CONTAINER( _pControlPanel ), 10 );

    GtkWidget* _pTemplateChooserBox = gtk_box_new( GTK_ORIENTATION_HORIZONTAL, 10 );
    GtkWidget* _pTemplateChooserButtons1 = gtk_box_new( GTK_ORIENTATION_HORIZONTAL, 10 );
    GtkWidget* _pTemplateChooserButtons2 = gtk_box_new( GTK_ORIENTATION_HORIZONTAL, 10 );
    GtkWidget* _pAutoscrollBox = gtk_box_new( GTK_ORIENTATION_HORIZONTAL, 10 );
 
    gtk_box_pack_start( GTK_BOX( _pTemplateChooserBox ), gtk_label_new( "Template:" ) , FALSE, TRUE, 0 );
    gtk_box_pack_start( GTK_BOX( _pTemplateChooserBox ), pWidgets_->pTemplateChooser_, TRUE, TRUE, 0 );

    gtk_box_pack_start( GTK_BOX( _pTemplateChooserButtons1 ), pWidgets_->pSaveTemplateButton_, TRUE, TRUE, 0 );
    gtk_box_pack_start( GTK_BOX( _pTemplateChooserButtons1 ), pWidgets_->pSaveNewTemplateButton_, TRUE, TRUE, 0 );
    gtk_box_pack_start( GTK_BOX( _pTemplateChooserButtons1 ), pWidgets_->pDeleteTemplateButton_, TRUE, TRUE, 0 );

    gtk_box_pack_start( GTK_BOX( _pTemplateChooserButtons2), gtk_label_new( "Help:" ), FALSE, FALSE, 0 );
    gtk_box_pack_start( GTK_BOX( _pTemplateChooserButtons2), gtk_label_new( "" ), TRUE, TRUE, 0 );
    gtk_box_pack_start( GTK_BOX( _pTemplateChooserButtons2), pWidgets_->pEditJsonButton_, FALSE, FALSE, 0 );
 

    gtk_box_pack_start( GTK_BOX( _pAutoscrollBox ), gtk_label_new(" "), TRUE, TRUE, 0 );
    gtk_box_pack_start( GTK_BOX( _pAutoscrollBox ), pWidgets_->pConsoleAutoScrollCheckBox_, FALSE, TRUE, 0 );

    gtk_box_pack_start( GTK_BOX( _pControlPanel ), pWidgets_->pTourneyIdChooser_, FALSE, TRUE, 0 );
    gtk_box_pack_start( GTK_BOX( _pControlPanel ), _pTemplateChooserBox, FALSE, TRUE, 0 );
    gtk_box_pack_start( GTK_BOX( _pControlPanel ), _pTemplateChooserButtons1, FALSE, TRUE, 0 );

    gtk_box_pack_start( GTK_BOX( _pControlPanel ), _pTemplateChooserButtons2, FALSE, TRUE, 0 );
    gtk_box_pack_start( GTK_BOX( _pControlPanel ), pWidgets_->pHelpViewer_, TRUE, TRUE, 0 ); 

    gtk_box_pack_start( GTK_BOX( _pControlPanel ), _pAutoscrollBox, FALSE, TRUE, 0 );

    gtk_widget_set_sensitive( pWidgets_->pSaveTemplateButton_, FALSE );
    gtk_widget_set_sensitive( pWidgets_->pSaveNewTemplateButton_, FALSE );
    gtk_widget_set_sensitive( pWidgets_->pDeleteTemplateButton_, FALSE );
    gtk_widget_set_sensitive( pWidgets_->pEditJsonButton_, FALSE );

    return _pControlPanel;
}

GtkWidget* Dashboard::menu_bar_new()
{
    GtkWidget* _pMenuBar = gtk_menu_bar_new();
    GtkWidget* _pFileMenuItem = gtk_menu_item_new_with_label( "File" );
    GtkWidget* _pHelpMenuItem = gtk_menu_item_new_with_label( "Help" );
    gtk_menu_shell_append( GTK_MENU_SHELL( _pMenuBar ), _pFileMenuItem );
    gtk_menu_shell_append( GTK_MENU_SHELL( _pMenuBar ), _pHelpMenuItem );
 
    GtkWidget* _pFileSubMenu = gtk_menu_new();
    gtk_menu_item_set_submenu( GTK_MENU_ITEM( _pFileMenuItem ), _pFileSubMenu );
    // gtk_menu_shell_append( GTK_MENU_SHELL( _pFileSubMenu ), pWidgets_->pImportScriptMenuItem_ );
    gtk_menu_shell_append( GTK_MENU_SHELL( _pFileSubMenu ), pWidgets_->pImportLogMenuItem_ );
    // gtk_menu_shell_append( GTK_MENU_SHELL( _pFileSubMenu ), pWidgets_->pExportLogMenuItem_ );
    gtk_widget_set_sensitive( pWidgets_->pImportScriptMenuItem_, FALSE );
    gtk_widget_set_sensitive( pWidgets_->pExportLogMenuItem_, FALSE );

    GtkWidget* _pHelpSubMenu = gtk_menu_new();
    gtk_menu_item_set_submenu( GTK_MENU_ITEM( _pHelpMenuItem ), _pHelpSubMenu );
    gtk_menu_shell_append( GTK_MENU_SHELL( _pHelpSubMenu ), pWidgets_->pAboutMenuItem_ );

    return _pMenuBar;
}

// supply text for viewers
void Dashboard::set_publish_message() 
{ 
    gchar* _templateName = NULL;
    gchar* _jsonText = NULL;
    gchar* _helpText = NULL;

    switch ( currentPublishSource_ )
    {
        case TEMPLATE:
            _templateName = template_chooser_get_current_template_name( TEMPLATE_CHOOSER( pWidgets_->pTemplateChooser_  ) );
            _jsonText = template_chooser_get_current_template( TEMPLATE_CHOOSER( pWidgets_->pTemplateChooser_ ) );
            _helpText = template_chooser_get_current_help_text( TEMPLATE_CHOOSER( pWidgets_->pTemplateChooser_ ) );

            pLogger_->Debug( g_strdup_printf( "Template changed to '%s'", _templateName ) );
            json_viewer_set_json_string( JSON_VIEWER( pWidgets_->pPublishJsonViewer_ ), _jsonText ); 
            help_viewer_set_text( HELP_VIEWER( pWidgets_->pHelpViewer_ ), _helpText ); 
            break;

        case SCRIPT:
            TestCommand* _command = get_current_test_command();
            _jsonText = _command ? g_strdup( _command->get_json_string().c_str() ) : NULL;
   
            pLogger_->Debug( g_strdup_printf( "Using script command %d", 
                navigation_bar_get_current_record_number( NAVIGATION_BAR( pWidgets_->pPublishNavigationBar_) ) ) );
            json_viewer_set_json_string( JSON_VIEWER( pWidgets_->pPublishJsonViewer_ ), _jsonText ); 
            help_viewer_set_text( HELP_VIEWER( pWidgets_->pHelpViewer_ ), NULL ); 
            gtk_widget_set_sensitive( pWidgets_->pPublishButton_, _command != NULL );
            break;
    }

    gtk_widget_set_sensitive( pWidgets_->pPublishButton_, _jsonText != NULL );   
    g_free( _templateName );
    g_free( _jsonText );
    g_free( _helpText );
}

void Dashboard::set_subscribe_message( const gchar* text ) 
{ 
    json_viewer_set_json_string( JSON_VIEWER( pWidgets_->pSubscribeJsonViewer_ ), text ); 
} 

void Dashboard::set_publish_destination_id()
{
    DestinationId _newId;
    switch ( currentPublishSource_ )
    {
        case TEMPLATE:
            _newId = destination_chooser_get_destination_id( DESTINATION_CHOOSER( pWidgets_->pPublishDestinationChooser_ ) );
            break;
        case SCRIPT:
            TestCommand* pCommand = get_current_test_command();
            gchar* destinationNameTemplate = pDestinationMap_->get_destination_name_template( pCommand->destinationId_.destinationName_.c_str());
            _newId = pCommand ? DestinationId{ pCommand->destinationId_, 
                pParms_->get_tourney_id(),
                destinationNameTemplate } : 
                DestinationId{};
            g_free(destinationNameTemplate);
            break;
    }

    if ( _newId != publishDestinationId_)
    {
        publishDestinationId_ = _newId;
        pLogger_->Info( g_strdup_printf( "Publish destination set to %s", publishDestinationId_.to_string().c_str() ) );
    }
}

// helper functions
gchar* Dashboard::get_new_template_name()
{
    GtkWidget* _pDialog = gtk_dialog_new_with_buttons( "Save As...", GTK_WINDOW( pWidgets_->pMainWindow_ ), GTK_DIALOG_MODAL, 
        "_OK", GTK_RESPONSE_OK, "_Cancel", GTK_RESPONSE_CANCEL, NULL );
    GtkWidget* _pEntry = gtk_entry_new();
    gchar* _newTemplateName = NULL;

    gtk_box_pack_start( GTK_BOX( gtk_dialog_get_content_area( GTK_DIALOG( _pDialog ) ) ), 
        gtk_label_new( "Enter template name" ), FALSE, TRUE, 10 );
    gtk_box_pack_start( GTK_BOX( gtk_dialog_get_content_area( GTK_DIALOG( _pDialog ) ) ),  _pEntry, FALSE, TRUE, 10 );
    gtk_widget_show_all( _pDialog );
    gint _response = gtk_dialog_run( GTK_DIALOG( _pDialog ) );

    if ( GTK_RESPONSE_CANCEL != _response )
    {
        _newTemplateName = g_strdup( gtk_entry_get_text( GTK_ENTRY( _pEntry ) ) );
    }

    gtk_widget_destroy( _pDialog );   
    return _newTemplateName;
}

void Dashboard::set_template_dirty( gboolean dirty )
{
    templateIsDirty_ = dirty;
    gtk_widget_set_sensitive( pWidgets_->pSaveTemplateButton_, dirty );
    gtk_widget_set_sensitive( pWidgets_->pSaveNewTemplateButton_, dirty );
}

void Dashboard::subscribe_to_destination( DestinationId const& destinationId )
{
    subscribedDestinations_.insert( destinationId.destinationName_ );
    MessageStore* _pActiveMessageStore = get_message_store_for_destination( destinationId );
    if ( !_pActiveMessageStore )
    {
        _pActiveMessageStore = messageMall_[ destinationId.destinationName_ ] = new MessageStore( destinationId, pLogger_ );
    }
    navigation_bar_set_client( NAVIGATION_BAR( pWidgets_->pSubscribeNavigationBar_), _pActiveMessageStore );

    if ( pParms_->get_noAMQ() )
    {
        pLogger_->Info( g_strdup_printf( "Subscribed to %s", destinationId.get_destination_name() ) );       
    }
    else
    {
        pAMQClient_->subscribe( destinationId, this );
    }
}

void Dashboard::unsubscribe_from_destination( DestinationId const& destinationId )
{
    subscribedDestinations_.erase( destinationId.destinationName_ );
    pLogger_->Info( g_strdup_printf( "Unsubscribed from %s", destinationId.get_destination_name() ) );
}

MessageStore* Dashboard::get_message_store_for_destination( DestinationId const& destinationId ) const
{
    auto it = messageMall_.find( destinationId.destinationName_ );
    return ( it == messageMall_.end() ) ? nullptr : it->second;
}

void Dashboard::manage_subscribe_button_state( DestinationId const& destinationId )
{
    if ( destinationId.isValid() )
    {
        g_signal_handler_block( G_OBJECT( pWidgets_->pSubscribeButton_ ), subscribe_button_handler_id_ );
        if ( subscribedDestinations_.find( destinationId.destinationName_ ) == subscribedDestinations_.end() )
        {
            gtk_button_set_label( GTK_BUTTON( pWidgets_->pSubscribeButton_ ), "Subscribe" );
            gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON( pWidgets_->pSubscribeButton_ ), FALSE );
        }
        else
        {
            gtk_button_set_label( GTK_BUTTON( pWidgets_->pSubscribeButton_ ), "Unsubscribe" );
            gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON( pWidgets_->pSubscribeButton_ ), TRUE );
        }  
        g_signal_handler_unblock( G_OBJECT( pWidgets_->pSubscribeButton_ ), subscribe_button_handler_id_ );    
    }   
}

gchar* Dashboard::get_subscribed_destinations_tooltip_text() const
{
    std::string _tooltipText{ "Subscribed destinations:" };
    if (subscribedDestinations_.size() == 0)
    {
        _tooltipText += "\n   None";
    }
    else
    {
        for ( std::string _destination : subscribedDestinations_ )
        {
           _tooltipText += "\n   " + _destination; 
        }
    }
    return g_strdup( _tooltipText.c_str() );
}

// adk user for verification
gboolean Dashboard::verify_abandoning_template_changes( const gchar* message )
{
    if ( templateIsDirty_ )
    {
        GtkWidget* _pDialog = gtk_dialog_new_with_buttons( "Template management", GTK_WINDOW( pWidgets_->pMainWindow_ ), GTK_DIALOG_MODAL, 
            "_OK", GTK_RESPONSE_OK, "_Cancel", GTK_RESPONSE_CANCEL, NULL );
        gtk_box_pack_start( GTK_BOX( gtk_dialog_get_content_area( GTK_DIALOG( _pDialog ) ) ), 
            gtk_label_new( message ), FALSE, TRUE, 10 );
        gtk_widget_show_all( _pDialog );
        gint _response = gtk_dialog_run( GTK_DIALOG( _pDialog ) );
        gtk_widget_destroy( _pDialog );

        if ( GTK_RESPONSE_CANCEL == _response )
        {
            template_chooser_set_current_path( TEMPLATE_CHOOSER( pWidgets_->pTemplateChooser_ ), pCurrentTemplatePath_ );
            destination_chooser_set_prefix_id( DESTINATION_CHOOSER( pWidgets_->pPublishDestinationChooser_ ), currentPublishPrefixId );
            return FALSE;
        }
        templateIsDirty_ = FALSE;
    }

    return TRUE;
}

gboolean Dashboard::verify_modifying_template( const gchar* message ) const
{
    GtkWidget* _pDialog = gtk_dialog_new_with_buttons( "Template management", GTK_WINDOW( pWidgets_->pMainWindow_ ), GTK_DIALOG_MODAL, 
        "_OK", GTK_RESPONSE_OK, "_Cancel", GTK_RESPONSE_CANCEL, NULL );
 
    gtk_box_pack_start( GTK_BOX( gtk_dialog_get_content_area( GTK_DIALOG( _pDialog ) ) ), 
        gtk_label_new( message ), FALSE, TRUE, 10 );
    gtk_widget_show_all( _pDialog );
    gint _response = gtk_dialog_run( GTK_DIALOG( _pDialog ) );
    gtk_widget_destroy( _pDialog );

    return ( GTK_RESPONSE_OK == _response );
}

TestCommand* Dashboard::get_current_test_command()
{
    return pTest_ ? pTest_->get_current_command() : NULL;
}

// callbacks
void Dashboard::OnPublishPrefixChanged( DestinationChooser* pDestinationChooser, Dashboard* pDashboard )
{
    gint _newPrefixId = destination_chooser_get_prefix_id( pDestinationChooser );
    if ( _newPrefixId != pDashboard->currentPublishPrefixId )
    {
        if ( pDashboard->verify_abandoning_template_changes( "Switch template without saving changes?" ) )
        {
            pDashboard->currentPublishPrefixId = _newPrefixId;
            gchar* _newPrefix = destination_chooser_get_prefix( pDestinationChooser );
            g_object_set( G_OBJECT( pDashboard->pWidgets_->pTemplateChooser_ ), "destination-prefix", _newPrefix, NULL );
            g_free( _newPrefix );

            g_signal_emit_by_name( G_OBJECT( pDashboard->pWidgets_->pPublishDestinationChooser_ ), "destination-changed" );
        }
    }
}

void Dashboard::OnSubscribeDestinationChanged( DestinationChooser* pDestinationChooser, Dashboard* pDashboard )
{
    DestinationId _destinationId = destination_chooser_get_destination_id( pDestinationChooser );
    navigation_bar_set_client( NAVIGATION_BAR( pDashboard->pWidgets_->pSubscribeNavigationBar_ ), pDashboard->get_message_store_for_destination( _destinationId ) );
    pDashboard->manage_subscribe_button_state( _destinationId );

    pDashboard->pLogger_->Info( g_strdup_printf( "Subscribe destination changed to %s", _destinationId.get_destination_name()) );
}

void Dashboard::OnPublishDestinationChanged( DestinationChooser* pDestinationChooser, Dashboard* pDashboard )
{
    pDashboard->set_publish_destination_id();
} 

void Dashboard::OnTemplateChanged( TemplateChooser* pTemplateChooser, Dashboard* pDashboard )
{
    GtkTreePath* _pNewTemplatePath = template_chooser_get_current_path( pTemplateChooser );
    gtk_widget_set_sensitive( pDashboard->pWidgets_->pDeleteTemplateButton_, _pNewTemplatePath != NULL );
    gtk_widget_set_sensitive( pDashboard->pWidgets_->pEditJsonButton_, _pNewTemplatePath != NULL );

    // don't update template if path has not changed
    gboolean _updateTemplate = ( 0 != GtkUtilities::compare_tree_paths( _pNewTemplatePath, pDashboard->pCurrentTemplatePath_ ) );

    // don't update template if user forgot to save changes
    if ( _updateTemplate )  
    {
        _updateTemplate = pDashboard->verify_abandoning_template_changes( "Switch template without saving changes?" );
    }

    if ( _updateTemplate )
    {
        gtk_tree_path_free( pDashboard->pCurrentTemplatePath_ );
        pDashboard->pCurrentTemplatePath_ = _pNewTemplatePath;
        pDashboard->set_publish_message();
    }
    else
    {
        gtk_tree_path_free( _pNewTemplatePath );
    }
}

void Dashboard::OnTourneyIdChanged( TourneyIdChooser* pTourneyIdChooser, Dashboard* pDashboard )
{
    pDashboard->pParms_->set_tourney_id( tourney_id_chooser_get_tourney_id( pTourneyIdChooser ) );;
    pDashboard->pLogger_->Info( g_strdup_printf( "Tourney id changed to %s", pDashboard->pParms_->get_tourney_id() ) );

    g_signal_emit_by_name( G_OBJECT( pDashboard->pWidgets_->pSubscribeDestinationChooser_ ), "destination-changed" );
    g_signal_emit_by_name( G_OBJECT( pDashboard->pWidgets_->pPublishDestinationChooser_ ), "destination-changed" );
}

void Dashboard::OnSubscribeButtonClicked( GtkToggleButton* pButton, Dashboard* pDashboard )
{
    DestinationId _destinationId = destination_chooser_get_destination_id( DESTINATION_CHOOSER( pDashboard->pWidgets_->pSubscribeDestinationChooser_ ) );
    if ( _destinationId.isValid() )
    {
        if ( gtk_toggle_button_get_active( pButton ) )
        {
            pDashboard->subscribe_to_destination( _destinationId );
        }
         else
        {
            pDashboard->unsubscribe_from_destination( _destinationId );
        }

        pDashboard->manage_subscribe_button_state( _destinationId );
    }
}

bool Dashboard::OnSubscribeButtonToolTip( GtkWidget* pButton, gint x, gint y, gboolean keyboard_mode, GtkTooltip* pTooltip, Dashboard* pDashboard )
{
    gchar* _tooltipText = pDashboard->get_subscribed_destinations_tooltip_text();
    gtk_tooltip_set_text( pTooltip,  _tooltipText );
    g_free( _tooltipText );
    return TRUE;
}

void Dashboard::OnPublishButtonClicked( GtkButton* pButton, Dashboard* pDashboard )
{
    if ( pDashboard->publishDestinationId_.isValid() )
    {
        gchar* _jsonString = json_viewer_get_json_string( JSON_VIEWER( pDashboard->pWidgets_->pPublishJsonViewer_ ) );
        if ( pDashboard->pParms_->get_noAMQ() )
        {
            pDashboard->pLogger_->Info( g_strdup_printf( "Publishing to %s: %s", 
                pDashboard->publishDestinationId_.to_string().c_str(), _jsonString ) );
            pDashboard->OnAMQMessage( pDashboard->publishDestinationId_, _jsonString );    // for testing, echo message
        }
        else
        {
            pDashboard->pAMQClient_->send_text_message( pDashboard->publishDestinationId_, _jsonString );
        }
        g_free ( _jsonString );
    }
}

void Dashboard::OnSaveButtonClicked( GtkButton* pButton, Dashboard* pDashboard )
{
    gchar* _text = json_viewer_get_json_string( JSON_VIEWER( pDashboard->pWidgets_->pPublishJsonViewer_ ) );
    template_chooser_update_template( TEMPLATE_CHOOSER( pDashboard->pWidgets_->pTemplateChooser_), _text, pDashboard->pCurrentTemplatePath_ );
    g_free( _text );

    _text = help_viewer_get_text( HELP_VIEWER( pDashboard->pWidgets_->pHelpViewer_ ) );
    template_chooser_update_help_text( TEMPLATE_CHOOSER( pDashboard->pWidgets_->pTemplateChooser_), _text, pDashboard->pCurrentTemplatePath_ );
    g_free( _text );

    if ( template_chooser_save_changes( TEMPLATE_CHOOSER( pDashboard->pWidgets_->pTemplateChooser_ ) ) )
    {
        pDashboard->set_template_dirty( FALSE ); 
    } 
}

void Dashboard::OnSaveAsButtonClicked( GtkButton* pButton, Dashboard* pDashboard )
{
    gchar* _newTemplateName = pDashboard->get_new_template_name();
    GtkTreePath* _pNewTemplatePath = NULL;
    gboolean _saveOK = TRUE;

    if ( _newTemplateName )
    {
        // if name already exists, make sure it's OK to replace it
        // otherwise, create a new entry
        _pNewTemplatePath = template_chooser_get_path_by_name( TEMPLATE_CHOOSER( pDashboard->pWidgets_->pTemplateChooser_ ), _newTemplateName );
        if ( _pNewTemplatePath )
        {
            gchar* _message = g_strdup_printf( "Replace template '%s'?", _newTemplateName );
            _saveOK = pDashboard->verify_modifying_template( _message );
            g_free( _message );
        }
        else
        {
            _pNewTemplatePath = template_chooser_template_new( TEMPLATE_CHOOSER( pDashboard->pWidgets_->pTemplateChooser_ ), _newTemplateName );
        }

        // if it's OK to proceed, save the template and help text, save the file, and select the new template in the combo bo
        if ( _saveOK )
        {
            gchar* _text = json_viewer_get_json_string( JSON_VIEWER( pDashboard->pWidgets_->pPublishJsonViewer_ ) );
            template_chooser_update_template( TEMPLATE_CHOOSER( pDashboard->pWidgets_->pTemplateChooser_), _text, _pNewTemplatePath );
            g_free( _text );

            _text = help_viewer_get_text( HELP_VIEWER( pDashboard->pWidgets_->pHelpViewer_ ) );
            template_chooser_update_help_text( TEMPLATE_CHOOSER( pDashboard->pWidgets_->pTemplateChooser_), _text, _pNewTemplatePath );
            g_free( _text );

            if ( template_chooser_save_changes( TEMPLATE_CHOOSER( pDashboard->pWidgets_->pTemplateChooser_ ) ) )
            {
                pDashboard->set_template_dirty( FALSE ); 
            }        
            template_chooser_set_current_path( TEMPLATE_CHOOSER( pDashboard->pWidgets_->pTemplateChooser_), _pNewTemplatePath ) ;  
        }      

        g_free( _newTemplateName );
        gtk_tree_path_free( _pNewTemplatePath );
    }
}

void Dashboard::OnDeleteButtonClicked( GtkButton* pButton, Dashboard* pDashboard )
{
    gchar* _templateName = template_chooser_get_current_template_name( TEMPLATE_CHOOSER( pDashboard->pWidgets_->pTemplateChooser_ ) );
    gchar* _message = g_strdup_printf( "Delete template '%s'?", _templateName );

    if ( pDashboard->verify_modifying_template( _message ) )
    {
        template_chooser_delete_template( TEMPLATE_CHOOSER( pDashboard->pWidgets_->pTemplateChooser_ ), pDashboard->pCurrentTemplatePath_ );
        template_chooser_save_changes( TEMPLATE_CHOOSER( pDashboard->pWidgets_->pTemplateChooser_ ) );
    }

    g_free( _message );    
    g_free( _templateName );
}
 
void Dashboard::OnEditJsonButtonClicked( GtkButton* pButton, Dashboard* pDashboard )
{
    GtkWidget* _pJsonEditor = json_editor_new( GTK_WINDOW( pDashboard->pWidgets_->pMainWindow_ ), pDashboard->pLogger_ );

    gchar* _oldText = json_viewer_get_json_string( JSON_VIEWER( pDashboard->pWidgets_->pPublishJsonViewer_ ) );
    gchar* _newText = json_editor_run( JSON_EDITOR( _pJsonEditor ), _oldText );

    if ( _newText != NULL )
    {
        json_viewer_set_json_string( JSON_VIEWER( pDashboard->pWidgets_->pPublishJsonViewer_ ), _newText );
        pDashboard->set_template_dirty( TRUE ); 
    }

    g_free( _oldText );
    g_free( _newText );
    gtk_widget_destroy( _pJsonEditor );
}

void Dashboard::OnConsoleAutoScrollClicked( GtkToggleButton* pButton, Dashboard* pDashboard )
{
    console_viewer_set_auto_scroll( CONSOLE_VIEWER( pDashboard->pWidgets_->pConsoleViewer_ ), gtk_toggle_button_get_active( pButton ) );
}


void Dashboard::OnRunScriptClicked( GtkToggleButton* pButton, Dashboard* pDashboard )
{
    gboolean _isActive = gtk_toggle_button_get_active( pButton );
    if ( _isActive )
    {
        if ( !pDashboard->verify_abandoning_template_changes( "Switch to script without saving changes to template?" ) )
        {
            g_signal_handler_block( G_OBJECT( pButton ), pDashboard->run_script_check_box_handler_id_ )  ;
            gtk_toggle_button_set_active( pButton, FALSE );
            g_signal_handler_unblock( G_OBJECT( pButton ), pDashboard->run_script_check_box_handler_id_ );
            return;
        }
    }

    gtk_widget_set_sensitive( pDashboard->pWidgets_->pPublishDestinationChooser_, !_isActive );
    gtk_widget_set_sensitive( pDashboard->pWidgets_->pTemplateChooser_, !_isActive );   
    gtk_widget_set_sensitive( pDashboard->pWidgets_->pTourneyIdChooser_, !_isActive ); 
    gtk_widget_set_sensitive( pDashboard->pWidgets_->pPublishNavigationBar_, _isActive ); 

    pDashboard->currentPublishSource_ =  _isActive ? Dashboard::SCRIPT : Dashboard::TEMPLATE;
    pDashboard->set_publish_destination_id();
    pDashboard->set_publish_message();
}

void Dashboard::OnImportScriptMenuItemActivated( GtkMenuItem* pMenuItem, Dashboard* pDashboard )
{
    GtkWidget* _pFileChooser = gtk_file_chooser_dialog_new("Choose Script File", GTK_WINDOW( pDashboard->pWidgets_->pMainWindow_ ),
        GTK_FILE_CHOOSER_ACTION_OPEN, "Cancel", GTK_RESPONSE_CANCEL,  "OK", GTK_RESPONSE_OK, NULL);
    gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(_pFileChooser), pDashboard->pParms_->get_working_directory());

    if (gtk_dialog_run (GTK_DIALOG (_pFileChooser)) == GTK_RESPONSE_OK)
    {
        gchar* _fileName = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (_pFileChooser) );
        pDashboard->pParms_->set_script_file_name( gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (_pFileChooser) ) );
        pDashboard->pLogger_->Info( g_strdup_printf( "Script file selected: %s", _fileName ) );
        g_free( _fileName );
    }
    gtk_widget_destroy( _pFileChooser );
}

void Dashboard::OnImportLogMenuItemActivated( GtkMenuItem* pMenuItem, Dashboard* pDashboard )
{
    GtkWidget* _pFileChooser = gtk_file_chooser_dialog_new("Choose Log File", GTK_WINDOW( pDashboard->pWidgets_->pMainWindow_ ),
        GTK_FILE_CHOOSER_ACTION_OPEN, "Cancel", GTK_RESPONSE_CANCEL,  "OK", GTK_RESPONSE_OK, NULL);
    gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(_pFileChooser), pDashboard->pParms_->get_working_directory());

    if (gtk_dialog_run (GTK_DIALOG (_pFileChooser)) == GTK_RESPONSE_OK)
    {
        gchar* _fileName = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (_pFileChooser) );
        pDashboard->pParms_->set_log_file_name( gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (_pFileChooser) ) );
        pDashboard->pLogger_->Info( g_strdup_printf( "Log file selected: %s", _fileName ) );

        if ( pDashboard->pTest_ )
        {
            delete pDashboard->pTest_;
        }
        LogParser _parser{ pDashboard->pLogger_, pDashboard->pDestinationMap_ };
        pDashboard->pTest_ = _parser.parse_log_file( _fileName );
        g_free( _fileName );

        gtk_widget_set_sensitive( pDashboard->pWidgets_->pRunScriptCheckBox_, (pDashboard->pTest_ != NULL) );
        navigation_bar_set_client( NAVIGATION_BAR( pDashboard->pWidgets_->pPublishNavigationBar_ ), pDashboard->pTest_ );
        navigation_bar_on_client_item_added( NAVIGATION_BAR( pDashboard->pWidgets_->pPublishNavigationBar_ ) );
    }

    gtk_widget_destroy( _pFileChooser );
}

void Dashboard::OnExportLogMenuItemActivated( GtkMenuItem* pMenuItem, Dashboard* pDashboard )
{


}

void Dashboard::OnAboutMenuItemActivated( GtkMenuItem* pMenuItem, Dashboard* pDashboard )
{
    GtkWidget* _pAboutDialog = gtk_dialog_new_with_buttons("About", GTK_WINDOW( pDashboard->pWidgets_->pMainWindow_ ), GTK_DIALOG_MODAL, 
        "_OK", GTK_RESPONSE_OK, NULL );
    gtk_container_set_border_width( GTK_CONTAINER( _pAboutDialog ), 10 ); 

    gtk_box_pack_start( GTK_BOX( gtk_dialog_get_content_area( GTK_DIALOG( _pAboutDialog ) ) ), 
        gtk_label_new( TESTING_HARNESS_NAME ), FALSE, FALSE, 10 );
    gtk_box_pack_start( GTK_BOX( gtk_dialog_get_content_area( GTK_DIALOG( _pAboutDialog ) ) ), 
        gtk_label_new( TESTING_HARNESS_VERSION ), FALSE, FALSE, 10 );
    gtk_box_pack_start( GTK_BOX( gtk_dialog_get_content_area( GTK_DIALOG( _pAboutDialog ) ) ), 
        gtk_label_new( TESTING_HARNESS_COPYRIGHT ), FALSE, FALSE, 10 );
    gtk_widget_show_all( _pAboutDialog );
    gint _response = gtk_dialog_run( GTK_DIALOG( _pAboutDialog ) );
    gtk_widget_destroy( _pAboutDialog );
}

void Dashboard::OnJsonEdited( JsonViewer* pJsonViewer, Dashboard* pDashboard )
{
    pDashboard->set_template_dirty( TRUE );      
}

void Dashboard::OnJsonReplaced( JsonViewer* pJsonViewer, Dashboard* pDashboard )
{
    pDashboard->set_template_dirty( FALSE );  
}

void Dashboard::OnHelpEdited( HelpViewer* pHelpViewer, Dashboard* pDashboard )
{
    pDashboard->set_template_dirty( TRUE );   
}

void Dashboard::OnSubscribeNavigationBarTextChanged( NavigationBar* pNavigationBar, Dashboard* pDashboard )
{
    MessageStore* _messageStore = static_cast<MessageStore*>( navigation_bar_get_client( pNavigationBar ) );
    pDashboard->set_subscribe_message(  _messageStore ? _messageStore->get_current_message() : NULL );
}

void Dashboard::OnPublishNavigationBarTextChanged( NavigationBar* pNavigationBar, Dashboard* pDashboard )
{
    if ( pDashboard->currentPublishSource_ == Dashboard::SCRIPT )
    {
        pDashboard->set_publish_destination_id();
        pDashboard->set_publish_message();
    }
}

void Dashboard::OnMainWindowDestroy( GtkWidget* pWindow, Dashboard* pDashboard )
{
    pDashboard->pLogger_->set_console_logger( NULL );
    gtk_main_quit();
} 

gboolean Dashboard::OnMainWindowDelete( GtkWidget* pWindow, GdkEvent *pEvent, Dashboard* pDashboard )
{
    if ( pDashboard->templateIsDirty_ )
    {
        return !pDashboard->verify_abandoning_template_changes( "Exit without saving changes to template?" );
    }
    return FALSE;
}

gboolean Dashboard::OnIdle( AMQClient* pAMQClient )
{
    pAMQClient->receive_messages();
    return TRUE;
}

// public methods
void Dashboard::OnAMQMessage( DestinationId const& destinationId, const gchar* message )
{
    if (subscribedDestinations_.find( destinationId.destinationName_ ) != subscribedDestinations_.end())
    {
        if (pParms_->get_noAMQ() )
        {
            pLogger_->Info( g_strdup_printf( "Receiving for %s '%s': %s",
                destinationId.get_destination_type(), destinationId.get_destination_name(), message ) );
        }
        MessageStore* _pMessageStore = get_message_store_for_destination( destinationId );
        if ( _pMessageStore )
        {
            _pMessageStore->add_message( g_strdup( message )  );
            if ( MessageStore::ACTIVE == _pMessageStore->get_status() )
            {
                navigation_bar_on_client_item_added( NAVIGATION_BAR( pWidgets_->pSubscribeNavigationBar_ ) );
            }
        }
    }
}

void Dashboard::run()
{
    gtk_widget_show_all( pWidgets_->pMainWindow_ );  

    if ( !pParms_->get_noAMQ() )
    {
       g_idle_add( (GSourceFunc)OnIdle, gpointer( pAMQClient_ ) );
    }
    gtk_main();
}