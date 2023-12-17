#pragma once

#include <gtk/gtk.h>

// TourneyIdChooser
//
//      a collection of widgets for specifying tourney_id
//
//  Public methods:
//      const gchar* tourney_id_chooser_get_tourney_id()
//          we retain ownership of the string
//
//  Signals:



G_BEGIN_DECLS

#define TOURNEY_ID_CHOOSER_TYPE                ( tourney_id_chooser_get_type() )
#define TOURNEY_ID_CHOOSER( obj )              ( G_TYPE_CHECK_INSTANCE_CAST( ( obj ), TOURNEY_ID_CHOOSER_TYPE, TourneyIdChooser ) )
#define TOURNEY_ID_CHOOSER_CLASS( klass )      ( G_TYPE_CHECK_CLASS_CAST( (klass), TOURNEY_ID_CHOOSER_TYPE, TourneyIdChooserClass ) )
#define IS_TOURNEY_ID_CHOOSER( obj )           ( G_TYPE_CHECK_INSTANCE_TYPE( (obj), TOURNEY_ID_CHOOSER_TYPE ) )
#define IS_TOURNEY_ID_CHOOSER_CLASS( klass )   ( G_TYPE_CHECK_CLASS_TYPE( (klass), TOURNEY_ID_CHOOSER_TYPE ) )

typedef struct _TourneyIdChooser               TourneyIdChooser;
typedef struct _TourneyIdChooserClass          TourneyIdChooserClass;

struct _TourneyIdChooser
{
    GtkBox box_;
};

struct _TourneyIdChooserClass
{
    GtkBoxClass parentClass_;

    void (* changed) ( TourneyIdChooser* pTourneyIdchooser );
};

GType tourney_id_chooser_get_type( void ) G_GNUC_CONST;
GtkWidget* tourney_id_chooser_new( const gchar* tourneyId );

const gchar* tourney_id_chooser_get_tourney_id( TourneyIdChooser* pTourneyIdChooser );
void tourney_id_chooser_set_tourney_id( TourneyIdChooser* pTourneyIdChooser, const gchar* tourneyId );

G_END_DECLS
