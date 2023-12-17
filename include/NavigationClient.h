#pragma once

#include "gtk/gtk.h"

class NavigationClient
{
    public: 
        enum NavigationClientStatus
        {
            NEW,
            INACTIVE,
            ACTIVE
        };

    private:
        gint    currentItemNumber_  = 0;
        NavigationClientStatus  status_;

    public:
        NavigationClient() {}

        NavigationClientStatus get_status() const { return status_; }
        void set_status( NavigationClientStatus status ) { status_ = status; }

        virtual gint get_max_item_number() const = 0;
        gint get_current_item_number() const { return currentItemNumber_; }
        void set_current_item_number(gint messageNumber ) { currentItemNumber_ = messageNumber; }
};
