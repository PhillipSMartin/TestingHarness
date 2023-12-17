#include "TemplateChooser.h"

#include "GtkUtilities.h"
#include "Logger.h"
#include "TemplateListStore.h"

#define TEMPLATE_CHOOSER_GET_PRIVATE( obj ) ( G_TYPE_INSTANCE_GET_PRIVATE( (obj), TEMPLATE_CHOOSER_TYPE, TemplateChooserPrivate ) )

typedef struct _TemplateChooserPrivate TemplateChooserPrivate;

struct _TemplateChooserPrivate
{
    Logger* pLogger_;
    TemplateListStore* pListStore_;
    gchar* topic_ = NULL;
    gchar* workingDirectory_;
};

// signals emitted

// properties
enum
{
    PROP_0,
    PROP_TOPIC_PREFIX
};

// Private function prototypes
static void template_chooser_class_init( TemplateChooserClass* kpKass );
static void template_chooser_init( TemplateChooser* pTemplateChooser );
static void template_chooser_get_property( GObject*, guint, GValue*, GParamSpec* );
static void template_chooser_set_property( GObject*, guint, const GValue*, GParamSpec* );

// returns number of templates imported or -1 on an error
static void template_chooser_update_destination_prefix( TemplateChooser* pTemplateChooser, const gchar* destination );

// Type registration
GType template_chooser_get_type( void )
{
    static GType _template_chooser_type = 0;

    if ( !_template_chooser_type )
    {
        static const GTypeInfo _template_chooser_info =
        {
            sizeof( TemplateChooserClass ),
            NULL,
            NULL,
            (GClassInitFunc) template_chooser_class_init,
            NULL,
            NULL,
            sizeof( TemplateChooser ),
            0,
            (GInstanceInitFunc) template_chooser_init,
        };

        _template_chooser_type = g_type_register_static( GTK_TYPE_COMBO_BOX, "TemplateChooser", &_template_chooser_info, (GTypeFlags)0 );
    }

    return _template_chooser_type;
}

// Initialization functions
static void template_chooser_class_init( TemplateChooserClass* pKlass )
{
    GObjectClass* gobject_class = G_OBJECT_CLASS( pKlass );

    // override property functions
    gobject_class->set_property = template_chooser_set_property;
    gobject_class->get_property = template_chooser_get_property;   

    // add private data
    g_type_class_add_private( pKlass, sizeof( TemplateChooserPrivate ) );

    // register the properties
    g_object_class_install_property( gobject_class, PROP_TOPIC_PREFIX,
        g_param_spec_string("destination-prefix",
            "AMQ Topic prefix",
            "Prefix of AMQ Topic name to publish to",
            "",
            G_PARAM_READWRITE)
    );
}

static void template_chooser_init( TemplateChooser* pTemplateChooser )
{
    TemplateChooserPrivate* _pPriv = TEMPLATE_CHOOSER_GET_PRIVATE( pTemplateChooser );

    GtkCellRenderer* _pRenderer = gtk_cell_renderer_text_new();
    
    gtk_cell_layout_pack_start( GTK_CELL_LAYOUT( pTemplateChooser ), 
        _pRenderer, TRUE);
    gtk_cell_layout_set_attributes( GTK_CELL_LAYOUT( pTemplateChooser ), 
        _pRenderer, "text", KEY, NULL );
}

static void template_chooser_set_property( GObject* object, guint prop_id, const GValue* value, GParamSpec* pspec )
{
    TemplateChooser* _pTemplateChooser = TEMPLATE_CHOOSER( object );

    switch (prop_id)
    {
        case PROP_TOPIC_PREFIX:
            template_chooser_update_destination_prefix( _pTemplateChooser, g_value_get_string( value ) );
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID( object, prop_id, pspec );
            break;
    }
}

static void template_chooser_get_property( GObject* object, guint prop_id, GValue* value, GParamSpec* pspec )
{
    TemplateChooser* _pTemplateChooser = TEMPLATE_CHOOSER( object );
    TemplateChooserPrivate* _pPriv = TEMPLATE_CHOOSER_GET_PRIVATE( _pTemplateChooser );

    switch (prop_id)
    {
        case PROP_TOPIC_PREFIX:
            g_value_set_string( value, _pPriv->topic_ );
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID( object, prop_id, pspec );
            break;
    }
}

// returns number of templates imported or -1 on an error
static void template_chooser_update_destination_prefix( TemplateChooser* pTemplateChooser, const gchar* destination )
{
    TemplateChooserPrivate* _pPriv = TEMPLATE_CHOOSER_GET_PRIVATE( pTemplateChooser );

    g_free( _pPriv->topic_ );
    _pPriv->topic_ = g_strdup( destination );

    gchar* _templateBaseFileName = TEMPLATE_FILE_NAME( destination );
    gchar* _templateFileName = g_build_filename( _pPriv->workingDirectory_, _templateBaseFileName, NULL );
    gint _templatesImported = template_list_store_import_file( _pPriv->pListStore_, _templateFileName );

    if ( _templatesImported < 0 )
    {
         _pPriv->pLogger_->Info( g_strdup_printf( "No template file found for destination %s", destination ) );
    } 

    g_free( _templateBaseFileName );
    g_free( _templateFileName );
}


// Public functions
GtkWidget* template_chooser_new( Logger* pLogger, const gchar* workingDirectory )
{
    GtkWidget* _pComboBox = GTK_WIDGET( g_object_new( template_chooser_get_type(), NULL ) );
    TemplateChooserPrivate* _pPriv = TEMPLATE_CHOOSER_GET_PRIVATE( _pComboBox );
    _pPriv->pLogger_ = pLogger;
    _pPriv->workingDirectory_ = g_strdup( workingDirectory );

    // add model- must be done here because we did not have pLogger during template_chooser_init

    _pPriv->pListStore_ = template_list_store_new( _pPriv->pLogger_ );
    gtk_combo_box_set_model( GTK_COMBO_BOX( _pComboBox ), GTK_TREE_MODEL( _pPriv->pListStore_ ) );
    g_object_unref(  _pPriv->pListStore_ );

    return _pComboBox;
} 

gchar* template_chooser_get_current_template_name( TemplateChooser* pTemplateChooser )
{
    TemplateChooserPrivate* _pPriv = TEMPLATE_CHOOSER_GET_PRIVATE( pTemplateChooser ); 
    GtkTreeIter _iter;
    gchar* _templateName = NULL;
    if ( gtk_combo_box_get_active_iter( GTK_COMBO_BOX( pTemplateChooser ), &_iter ) )
    {
        if ( template_list_store_get_values( _pPriv->pListStore_, &_iter, &_templateName, NULL, NULL ) )
        {
            return _templateName;
        }
    }

    g_free( _templateName );
    return NULL;
}

gchar* template_chooser_get_current_template( TemplateChooser* pTemplateChooser )
{
    TemplateChooserPrivate* _pPriv = TEMPLATE_CHOOSER_GET_PRIVATE( pTemplateChooser ); 
    GtkTreeIter _iter;
    gchar* _templateText = NULL;
    if ( gtk_combo_box_get_active_iter( GTK_COMBO_BOX( pTemplateChooser ), &_iter ) )
    {
        if ( template_list_store_get_values( _pPriv->pListStore_, &_iter, NULL, &_templateText, NULL ) )
        {
            return _templateText;
        }
    }

    g_free( _templateText );
    return NULL;
}

gchar* template_chooser_get_current_help_text( TemplateChooser* pTemplateChooser )
{
    TemplateChooserPrivate* _pPriv = TEMPLATE_CHOOSER_GET_PRIVATE( pTemplateChooser ); 
    GtkTreeIter _iter;
    gchar* _helpText = NULL;
    if ( gtk_combo_box_get_active_iter( GTK_COMBO_BOX( pTemplateChooser ), &_iter ) )
    {
        if ( template_list_store_get_values( _pPriv->pListStore_, &_iter, NULL, NULL, &_helpText ) )
        {
            return _helpText;
        }
    }

    g_free( _helpText );
    return NULL;
 }

gboolean template_chooser_save_changes( TemplateChooser* pTemplateChooser )
{
    TemplateChooserPrivate* _pPriv = TEMPLATE_CHOOSER_GET_PRIVATE( pTemplateChooser );  
    return template_list_store_export_file( _pPriv->pListStore_ ); 
}

void template_chooser_update_template( TemplateChooser* pTemplateChooser, const gchar* text, GtkTreePath* pPath )
{
    TemplateChooserPrivate* _pPriv = TEMPLATE_CHOOSER_GET_PRIVATE( pTemplateChooser );
    gchar* _templateName = NULL;
    GtkTreeIter _iter;

    gtk_tree_model_get_iter( GTK_TREE_MODEL( _pPriv->pListStore_ ), &_iter, pPath );
    template_list_store_get_values( _pPriv->pListStore_, &_iter, &_templateName, NULL, NULL );
    gtk_list_store_set( GTK_LIST_STORE( _pPriv->pListStore_ ), &_iter, TEMPLATE, text, -1 );
    _pPriv->pLogger_->Debug( g_strdup_printf( "Template for '%s' has been updated", _templateName ) );

    g_free( _templateName );
}

void template_chooser_update_help_text( TemplateChooser* pTemplateChooser, const gchar* text, GtkTreePath* pPath )
{
    TemplateChooserPrivate* _pPriv = TEMPLATE_CHOOSER_GET_PRIVATE( pTemplateChooser );
    gchar* _templateName = NULL;
    GtkTreeIter _iter;

    gtk_tree_model_get_iter( GTK_TREE_MODEL( _pPriv->pListStore_ ), &_iter, pPath );
    template_list_store_get_values( _pPriv->pListStore_, &_iter, &_templateName, NULL, NULL );
    gtk_list_store_set( GTK_LIST_STORE( _pPriv->pListStore_ ), &_iter, HELP, text, -1 );
    _pPriv->pLogger_->Debug( g_strdup_printf( "Help text for '%s' has been updated", _templateName ) );

    g_free( _templateName );
}

void template_chooser_delete_template( TemplateChooser* pTemplateChooser, GtkTreePath* pPath )
{
    TemplateChooserPrivate* _pPriv = TEMPLATE_CHOOSER_GET_PRIVATE( pTemplateChooser );
    gchar* _templateName = NULL;
    GtkTreeIter _iter;

    gtk_tree_model_get_iter( GTK_TREE_MODEL( _pPriv->pListStore_ ), &_iter, pPath );
    template_list_store_get_values( _pPriv->pListStore_, &_iter, &_templateName, NULL, NULL ); 

    gtk_list_store_remove( GTK_LIST_STORE( _pPriv->pListStore_ ), &_iter );

    _pPriv->pLogger_->Debug( g_strdup_printf( "Template '%s' has been deleted", _templateName ) );

    g_free( _templateName );
}

// adds a new record wih the specified name and returns the path - does not change current combo box selection
GtkTreePath* template_chooser_template_new( TemplateChooser* pTemplateChooser, const gchar* templateName )
{
    TemplateChooserPrivate* _pPriv = TEMPLATE_CHOOSER_GET_PRIVATE( pTemplateChooser );
    GtkTreeIter _iter;
    gtk_list_store_append( GTK_LIST_STORE( _pPriv->pListStore_ ), &_iter );
    gtk_list_store_set( GTK_LIST_STORE( _pPriv->pListStore_ ), &_iter, KEY, templateName, -1 );
    return gtk_tree_model_get_path( GTK_TREE_MODEL( _pPriv->pListStore_ ), &_iter );
}

// gets the path of the given template name - returns NULL if the name does not exist
GtkTreePath* template_chooser_get_path_by_name( TemplateChooser* pTemplateChooser, const gchar* templateName )
{
    TemplateChooserPrivate* _pPriv = TEMPLATE_CHOOSER_GET_PRIVATE( pTemplateChooser );
    GtkTreeIter _iter;
    gchar* _iterTemplateName = NULL;
    gboolean _found = FALSE;

    gboolean _rc = gtk_tree_model_get_iter_first( GTK_TREE_MODEL( _pPriv->pListStore_ ), &_iter );
    while (_rc)
    {
        g_free( _iterTemplateName );
        template_list_store_get_values( _pPriv->pListStore_, &_iter, &_iterTemplateName, NULL, NULL );
 
        if ( 0 == g_strcmp0( _iterTemplateName, templateName ) )
        {
            _found = TRUE;
            break;
        }

        _rc = gtk_tree_model_iter_next( GTK_TREE_MODEL( _pPriv->pListStore_ ), &_iter );
    }
    g_free( _iterTemplateName );

    if ( _found )
    {
        return gtk_tree_model_get_path( GTK_TREE_MODEL( _pPriv->pListStore_ ), &_iter );
    }
    else
    {
        return NULL;
    }
}

GtkTreePath* template_chooser_get_current_path( TemplateChooser* pTemplateChooser )
{
    TemplateChooserPrivate* _pPriv = TEMPLATE_CHOOSER_GET_PRIVATE( pTemplateChooser );
    GtkTreeIter _iter;

    if ( !gtk_combo_box_get_active_iter( GTK_COMBO_BOX( pTemplateChooser ), &_iter ) )
    {
        return NULL;
    }

    return gtk_tree_model_get_path( GTK_TREE_MODEL( _pPriv->pListStore_ ), &_iter );
}

void template_chooser_set_current_path( TemplateChooser* templateChooser, GtkTreePath* newPath )
{
    if ( newPath )
    {
        TemplateChooserPrivate* _pPriv = TEMPLATE_CHOOSER_GET_PRIVATE( templateChooser );
        GtkTreeIter _iter;  
        GtkTreePath* _currentPath = template_chooser_get_current_path( templateChooser );

        if ( 0 !=  GtkUtilities::compare_tree_paths( newPath, _currentPath ) )
        {
            if ( gtk_tree_model_get_iter( GTK_TREE_MODEL( _pPriv->pListStore_ ), &_iter, newPath ) )
            {
                gtk_combo_box_set_active_iter( GTK_COMBO_BOX( templateChooser ), &_iter );
            }
            else
            {
                gchar* newPathString = gtk_tree_path_to_string( newPath );
                _pPriv->pLogger_->Error( g_strdup_printf( "Unable to find path %s ", newPathString ) );
                g_free( newPathString );
            }
        }

        gtk_tree_path_free( _currentPath );
    }
}