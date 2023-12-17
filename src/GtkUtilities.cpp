#include "GtkUtilities.h"

namespace GtkUtilities
{
    gint compare_tree_paths( GtkTreePath* pA, GtkTreePath* pB )
    {
        if ( pA == NULL )
        {
            return ( pB == NULL ) ? 0 : -1;
        }
        else
        {
            return ( pB == NULL ) ? 1 : gtk_tree_path_compare( pA, pB );
        }
    }
}