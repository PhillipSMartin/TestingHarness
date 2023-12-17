#pragma once

#include "DestinationId.h"

class DestinationMap;
class Logger;
class Parameters;

// DestinationChooser
//
//      a collection of widgets for determining a destination
//
//  Public methods:
//      gchar* destination_chooser_get_destination()
//          caller takes ownership of the string
//      gchar* destination_chooser_get_destination_prefix()
//          caller takes ownership of the string
//
//  Signals:
//      changed
//          destination has been changed


G_BEGIN_DECLS

#define DESTINATION_CHOOSER_TYPE                ( destination_chooser_get_type() )
#define DESTINATION_CHOOSER( obj )              ( G_TYPE_CHECK_INSTANCE_CAST( ( obj ), DESTINATION_CHOOSER_TYPE, DestinationChooser ) )
#define DESTINATION_CHOOSER_CLASS( klass )      ( G_TYPE_CHECK_CLASS_CAST( (klass), DESTINATION_CHOOSER_TYPE, DestinationChooserClass ) )
#define IS_DESTINATION_CHOOSER( obj )           ( G_TYPE_CHECK_INSTANCE_TYPE( (obj), DESTINATION_CHOOSER_TYPE ) )
#define IS_DESTINATION_CHOOSER_CLASS( klass )   ( G_TYPE_CHECK_CLASS_TYPE( (klass), DESTINATION_CHOOSER_TYPE ) )

typedef struct _DestinationChooser               DestinationChooser;
typedef struct _DestinationChooserClass          DestinationChooserClass;

struct _DestinationChooser
{
    GtkBox box_;
};

struct _DestinationChooserClass
{
    GtkBoxClass parentClass_;

    void (* prefix_changed) ( DestinationChooser* destinationChooser );
    void (* destination_changed) ( DestinationChooser* destinationChooser );
};

GType destination_chooser_get_type( void ) G_GNUC_CONST;
GtkWidget* destination_chooser_new( Parameters* pParms, Logger* pLogger, DestinationMap* pDestinationMap, const gchar* defaultDestination );

// caller assumes ownership of the cstring and must free it
gchar* destination_chooser_get_prefix( DestinationChooser* pDestinationchooser );
DestinationId destination_chooser_get_destination_id( DestinationChooser* pDestinationchooser );

gint destination_chooser_get_prefix_id( DestinationChooser* pDestinationchooser );
void destination_chooser_set_prefix_id( DestinationChooser* pDestinationchooser, gint newPrefix );

G_END_DECLS
