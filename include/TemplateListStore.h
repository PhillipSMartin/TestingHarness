#pragma once

#include "gtk/gtk.h"

class Logger;

// TemplateListStore
//
//      a model for storing a collection of templates and info about them
//
//  Public methods:
//      TemplateListStore* template_list_store_new( Logger* pLogger )
//          creates a new TemplateListStore
//
//      gboolean template_list_store_import_file( TemplateListStore* pTemplateListStore, std::string const& fileName );
//          reads a file containing template info and saves the info in the model
//
//      gboolean template_list_store_export_file( TemplateListStore* pTemplateListStore );
//          saves the model in the previously imported file
//
//  Signals:

G_BEGIN_DECLS

#define TEMPLATE_LIST_STORE_TYPE                ( template_list_store_get_type() )
#define TEMPLATE_LIST_STORE( obj )              ( G_TYPE_CHECK_INSTANCE_CAST( ( obj ), TEMPLATE_LIST_STORE_TYPE, TemplateListStore ) )
#define TEMPLATE_LIST_STORE_CLASS( klass )      ( G_TYPE_CHECK_CLASS_CAST( (klass), TEMPLATE_LIST_STORE_TYPE, TemplateListStoreClass ) )
#define IS_TEMPLATE_LIST_STORE( obj )           ( G_TYPE_CHECK_INSTANCE_TYPE( (obj), TEMPLATE_LIST_STORE_TYPE ) )
#define IS_TEMPLATE_LIST_STORE_CLASS( klass )   ( G_TYPE_CHECK_CLASS_TYPE( (klass), TEMPLATE_LIST_STORE_TYPE ) )

typedef struct _TemplateListStore               TemplateListStore;
typedef struct _TemplateListStoreClass          TemplateListStoreClass;

struct _TemplateListStore
{
    GtkListStore listStore_;
};

struct _TemplateListStoreClass
{
    GtkListStoreClass parentClass_;
};

// tree model columns
enum 
{
    KEY = 0,
    TEMPLATE,
    HELP,
    NUM_COLUMNS
};

GType template_list_store_get_type( void ) G_GNUC_CONST;
TemplateListStore*  template_list_store_new( Logger* pLogger );

// returns false if iter is invalid
// pointer (except iter) can be NULL if we are not interested in the value
gboolean template_list_store_get_values( TemplateListStore* pTemplateListStore, GtkTreeIter *pIter, gchar** pTemplateName, gchar** pTemplateText, gchar** pHelpText );

gint template_list_store_import_file( TemplateListStore* pTemplateListStore, const gchar* fileName  );
gboolean template_list_store_export_file( TemplateListStore* pTemplateListStore );

G_END_DECLS