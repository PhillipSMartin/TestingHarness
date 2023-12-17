#pragma once

#include "gtk/gtk.h"

class Logger;

G_BEGIN_DECLS

#define JSON_EDITOR_TYPE                ( json_editor_get_type() )
#define JSON_EDITOR( obj )              ( G_TYPE_CHECK_INSTANCE_CAST( ( obj ), JSON_EDITOR_TYPE, JsonEditor ) )
#define JSON_EDITOR_CLASS( klass )      ( G_TYPE_CHECK_CLASS_CAST( (klass), JSON_EDITOR_TYPE, JsonEditorClass ) )
#define IS_JSON_EDITOR( obj )           ( G_TYPE_CHECK_INSTANCE_TYPE( (obj), JSON_EDITOR_TYPE ) )
#define IS_JSON_EDITOR_CLASS( klass )   ( G_TYPE_CHECK_CLASS_TYPE( (klass), JSON_EDITOR_TYPE ) )

typedef struct _JsonEditor               JsonEditor;
typedef struct _JsonEditorClass          JsonEditorClass;

struct _JsonEditor
{
    GtkDialog dialog_;
};

struct _JsonEditorClass
{
    GtkDialogClass parentClass_;
};

GType json_editor_get_type( void ) G_GNUC_CONST;
GtkWidget* json_editor_new( GtkWindow* pParent, Logger* pLogger );

gchar* json_editor_run( JsonEditor* pJsonEditor, const gchar* text ); // returns new string or NULL if user clicked "Cancel"

G_END_DECLS