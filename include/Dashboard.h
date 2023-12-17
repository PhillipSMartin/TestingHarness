#pragma once

#include <unordered_map>
#include <set>

#include "AMQClient.h"
#include "ConsoleViewer.h"
#include "DestinationChooser.h"
#include "HelpViewer.h"
#include "JsonEditor.h"
#include "JsonViewer.h"
#include "NavigationBar.h"
#include "TemplateChooser.h"
#include "TourneyIdChooser.h"

class DestinationMap;
class Logger;
class Parameters;
class MessageStore;
class Test;
class TestCommand;

typedef struct 
{
    public:
        GtkWidget* pMainWindow_;

        // panes
        GtkWidget* pVerticalPane_;
        GtkWidget* pHorizontalPane_;
        GtkWidget* pJsonViewerPane_;
     
        // viewers
        GtkWidget* pSubscribeJsonViewer_;
        GtkWidget* pPublishJsonViewer_;
        GtkWidget* pConsoleViewer_;
        GtkWidget* pHelpViewer_;

        // choosers
        GtkWidget* pSubscribeDestinationChooser_;
        GtkWidget* pPublishDestinationChooser_;
        GtkWidget* pTemplateChooser_;
        GtkWidget* pTourneyIdChooser_;

        // buttons
        GtkWidget* pSubscribeButton_;
        GtkWidget* pPublishButton_;
        GtkWidget* pSaveTemplateButton_;
        GtkWidget* pSaveNewTemplateButton_; 
        GtkWidget* pDeleteTemplateButton_;
        GtkWidget* pEditJsonButton_;
        GtkWidget* pSubscribeNavigationBar_;
        GtkWidget* pPublishNavigationBar_;

        // checkboxes
        GtkWidget* pConsoleAutoScrollCheckBox_;
        GtkWidget* pRunScriptCheckBox_;

        // menu items
        GtkWidget* pAboutMenuItem_;
        GtkWidget* pImportScriptMenuItem_;
        GtkWidget* pImportLogMenuItem_;
        GtkWidget* pExportLogMenuItem_;

        // panels
        GtkWidget* pSubscribePanel_;
        GtkWidget* pPublishPanel_;
        GtkWidget* pControlPanel_;
        
} WidgetVault;


class Dashboard : public AMQListener
{
    private:
        Parameters* pParms_;
        Logger* pLogger_;
        DestinationMap* pDestinationMap_;
        WidgetVault* pWidgets_; 
        AMQClient* pAMQClient_;
        Test* pTest_ = NULL;

        enum PublishSource {
            TEMPLATE,
            SCRIPT
        };
        PublishSource currentPublishSource_ = TEMPLATE;
        DestinationId publishDestinationId_;

        std::set<std::string> subscribedDestinations_;    // a list of destinations we've subscribed to
        std::unordered_map<std::string, MessageStore*> messageMall_;   // a message list for each subscribed topic

        // handler ids, used we use to block the signal
        gulong subscribe_button_handler_id_;        
        gulong run_script_check_box_handler_id_;

        GtkTreePath* pCurrentTemplatePath_ = NULL;
        gint currentPublishPrefixId = -1;
        gboolean templateIsDirty_ = FALSE;

    public:
        Dashboard(Parameters* pParms, Logger* pLogger, DestinationMap* pDestinationMap, AMQClient* pAMQClient);
        ~Dashboard();
 
        virtual void OnAMQMessage( DestinationId const& destinationId, const gchar* message );
        void run(); 
 
    private:
        // initialization
        void instantiate_widgets();
        void arrange_widgets();
        void wire_signals();
        GtkWidget* subscribe_panel_new(); 
        GtkWidget* publish_panel_new(); 
        GtkWidget* control_panel_new(); 
        GtkWidget* menu_bar_new();

        // supply text for viewers
        void set_publish_message() ;
        void set_subscribe_message( const gchar* text ); 
        void set_publish_destination_id();
 
        // helper functions
        gchar* get_new_template_name();
        void set_template_dirty( gboolean dirty );
        TestCommand* get_current_test_command();

        void subscribe_to_destination( DestinationId const& destinationId );
        void unsubscribe_from_destination( DestinationId const& destinationId );
        MessageStore* get_message_store_for_destination( DestinationId const& destinationId) const;
        void manage_subscribe_button_state( DestinationId const& destinationId );
        gchar* get_subscribed_destinations_tooltip_text() const;

        // ask user for verifcation
        gboolean verify_abandoning_template_changes( const gchar* message );
        gboolean verify_modifying_template( const gchar* templateName ) const;

        // callbacks
        static void OnSubscribePrefixChanged( DestinationChooser* pDestinationChooser, Dashboard* pDashboard );
        static void OnPublishPrefixChanged( DestinationChooser* pDestinationChooser, Dashboard* pDashboard );
        static void OnSubscribeDestinationChanged( DestinationChooser* pDestinationChooser, Dashboard* pDashboard );
        static void OnPublishDestinationChanged( DestinationChooser* pDestinationChooser, Dashboard* pDashboard );
        static void OnTemplateChanged( TemplateChooser* pTemplateChooser, Dashboard* pDashboard ); 
        static void OnTourneyIdChanged( TourneyIdChooser* pTourneyIdChooser, Dashboard* pDashboard );
 
        static void OnSubscribeButtonClicked( GtkToggleButton* pButton, Dashboard* pDashboard );
        static bool OnSubscribeButtonToolTip( GtkWidget* pButton, gint x, gint y, gboolean keyboard_mode, GtkTooltip* pTooltip, Dashboard* pDashboard );
        static void OnPublishButtonClicked( GtkButton* pButton, Dashboard* pDashboard );
        static void OnSaveButtonClicked( GtkButton* pButton, Dashboard* pDashboard );
        static void OnSaveAsButtonClicked( GtkButton* pButton, Dashboard* pDashboard );
        static void OnDeleteButtonClicked( GtkButton* pButton, Dashboard* pDashboard );
        static void OnEditJsonButtonClicked( GtkButton* pButton, Dashboard* pDashboard );
        static void OnConsoleAutoScrollClicked( GtkToggleButton* pButton, Dashboard* pDashboard );
        static void OnRunScriptClicked( GtkToggleButton* pButton, Dashboard* pDashboard );

        static void OnImportScriptMenuItemActivated( GtkMenuItem* pMenuItem, Dashboard* pDashboard );
        static void OnImportLogMenuItemActivated( GtkMenuItem* pMenuItem, Dashboard* pDashboard );
        static void OnExportLogMenuItemActivated( GtkMenuItem* pMenuItem, Dashboard* pDashboard );
        static void OnAboutMenuItemActivated( GtkMenuItem* pMenuItem, Dashboard* pDashboard );

        static void OnJsonEdited( JsonViewer* pJsonViewer, Dashboard* pDashboard );
        static void OnJsonReplaced( JsonViewer* pJsonViewer, Dashboard* pDashboard );
        static void OnHelpEdited( HelpViewer* pHelpViewer, Dashboard* pDashboard );
        static void OnSubscribeNavigationBarTextChanged( NavigationBar* pNavigationBar, Dashboard* pDashboard );
        static void OnPublishNavigationBarTextChanged( NavigationBar* pNavigationBar, Dashboard* pDashboard );
     
        static void OnMainWindowDestroy( GtkWidget* pWindow, Dashboard* pDashboard );
        static gboolean OnMainWindowDelete( GtkWidget* pWindow, GdkEvent *pEvent, Dashboard* pDashboard );
        static gboolean OnIdle( AMQClient* pAMQClient );
};