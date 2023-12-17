#pragma once
#include <gtk/gtk.h>

class Logger;

// TemplateChooser
//
//      a combo box for selecting pre-defined templates
//
//  Public methods:
//
//  Signals:
//      changed
//          inherited from combo box


G_BEGIN_DECLS

#define TEMPLATE_CHOOSER_TYPE                ( template_chooser_get_type() )
#define TEMPLATE_CHOOSER( obj )              ( G_TYPE_CHECK_INSTANCE_CAST( ( obj ), TEMPLATE_CHOOSER_TYPE, TemplateChooser ) )
#define TEMPLATE_CHOOSER_CLASS( klass )      ( G_TYPE_CHECK_CLASS_CAST( (klass), TEMPLATE_CHOOSER_TYPE, TemplateChooserClass ) )
#define IS_TEMPLATE_CHOOSER( obj )           ( G_TYPE_CHECK_INSTANCE_TYPE( (obj), TEMPLATE_CHOOSER_TYPE ) )
#define IS_TEMPLATE_CHOOSER_CLASS( klass )   ( G_TYPE_CHECK_CLASS_TYPE( (klass), TEMPLATE_CHOOSER_TYPE ) )

#define TEMPLATE_FILE_NAME( topic )          ( g_strdup_printf( "%s.templates.txt", (topic) ) )

typedef struct _TemplateChooser               TemplateChooser;
typedef struct _TemplateChooserClass          TemplateChooserClass;

struct _TemplateChooser
{
    GtkComboBox comboBox_;
};

struct _TemplateChooserClass
{
    GtkComboBoxClass parentClass_;
};

GType template_chooser_get_type( void ) G_GNUC_CONST;
GtkWidget* template_chooser_new( Logger* pLogger, const char* workingDirectory );

// retrieves fields from active selection
// caller assumes ownership of the string and must free it
gchar* template_chooser_get_current_template_name( TemplateChooser* pTemplateChooser );
gchar* template_chooser_get_current_template( TemplateChooser* pTemplateChooser );
gchar* template_chooser_get_current_help_text( TemplateChooser* pTemplateChooser );

gboolean template_chooser_save_changes( TemplateChooser* pTemplateChooser );

// updates record at specified path with specified data - does not change current combo box selection
void template_chooser_update_template( TemplateChooser* pTemplateChooser, const gchar* text, GtkTreePath* pPath );
void template_chooser_update_help_text( TemplateChooser* pTemplateChooser, const gchar* text, GtkTreePath* pPath );
void template_chooser_delete_template( TemplateChooser* pTemplateChooser, GtkTreePath* pPath );

// adds a new record wih the specified name and returns the path - does not change current combo box selection
GtkTreePath* template_chooser_template_new( TemplateChooser* pTemplateChooser, const gchar* templateName );

// gets the path of the given template name - returns NULL if the name does not exist
GtkTreePath* template_chooser_get_path_by_name( TemplateChooser* pTemplateChooser, const gchar* templateName );
GtkTreePath* template_chooser_get_current_path( TemplateChooser* pTemplateChooser );
void template_chooser_set_current_path( TemplateChooser* pTemplateChooser, GtkTreePath* pPath );

G_END_DECLS