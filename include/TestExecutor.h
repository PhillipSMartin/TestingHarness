#pragma once

#include "gtk/gtk.h"

class Test;

class TestExecutor
{
    public:
        virtual gboolean run_test( Test* pTest ) = 0;
};