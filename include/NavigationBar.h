#pragma once

#include "gtk/gtk.h"

class Logger;
class NavigationClient;

G_BEGIN_DECLS

#define NAVIGATION_BAR_TYPE                ( navigation_bar_get_type() )
#define NAVIGATION_BAR( obj )              ( G_TYPE_CHECK_INSTANCE_CAST( ( obj ), NAVIGATION_BAR_TYPE, NavigationBar ) )
#define NAVIGATION_BAR_CLASS( klass )      ( G_TYPE_CHECK_CLASS_CAST( (klass), NAVIGATION_BAR_TYPE, NavigationBarClass ) )
#define IS_NAVIGATION_BAR( obj )           ( G_TYPE_CHECK_INSTANCE_TYPE( (obj), NAVIGATION_BAR_TYPE ) )
#define IS_NAVIGATION_BAR_CLASS( klass )   ( G_TYPE_CHECK_CLASS_TYPE( (klass), NAVIGATION_BAR_TYPE ) )

typedef struct _NavigationBar               NavigationBar;
typedef struct _NavigationBarClass          NavigationBarClass;

struct _NavigationBar
{
    GtkBox box_;
};

struct _NavigationBarClass
{
    GtkBoxClass parentClass_;

    void (* text_changed) ( NavigationBar* NavigationBar );
};

GType navigation_bar_get_type( void ) G_GNUC_CONST;
GtkWidget* navigation_bar_new( Logger* pLogger, gboolean autoScroll = TRUE );

void navigation_bar_set_client( NavigationBar* pNavigationBar, NavigationClient* pClient );
NavigationClient* navigation_bar_get_client( NavigationBar* pNavigationBar );
gint navigation_bar_get_current_record_number( NavigationBar* pNavigationBar );
void navigation_bar_on_client_item_added( NavigationBar* pNavigationBar );

G_END_DECLS
