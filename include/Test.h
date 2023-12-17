#pragma once

#include <vector>

#include "NavigationClient.h"
#include "TestCommand.h"

class Test : public std::vector<TestCommand*>, public NavigationClient
{
    public:
        gchar* testName_;

        Test( const gchar* testName ) : testName_{ g_strdup( testName ) } {}
        ~Test()
        {
            g_free( testName_ );
            for ( auto& _pCommand : *this )
            {
                delete _pCommand;
            }
        }

        virtual gint get_max_item_number() const { return size(); }  
        TestCommand* get_current_command() const { return ( get_current_item_number() == 0) ? NULL : at( get_current_item_number() - 1 ); }
};

class Tests : public std::vector<Test*>
{
    public:
        Tests() {}
        ~Tests()
        {
            for ( auto& _pTest : *this )
            {
                delete _pTest;
            }
        }       
};