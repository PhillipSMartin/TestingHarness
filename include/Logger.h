#pragma once

#include "Parameters.h"

class ConsoleLogger
{
    public:
        virtual void add_console_msg( const gchar* msg ) = 0;
};

class Logger
{
    private:
        const char* logFileBase_ = "TestingHarness";
        Parameters* pParms_;
        ConsoleLogger* pConsoleLogger_ = NULL;
        gboolean muted_ = FALSE;

    public:
        bool started_ = false;

        Logger(Parameters* pParms);

        // for g_strdup_printf output
        // we take owndership of the string
        void Debug(gchar* message) const;
        void Info(gchar* message) const;
        void Warn(gchar* message) const;
        void Error(gchar* message) const;
        void Fatal(gchar* message) const;

        // alternative functions for literals
        void DebugStr(std::string const& message) const;
        void InfoStr(std::string const& message) const;
        void WarnStr(std::string const& message) const;
        void ErrorStr(std::string const& message) const;
        void FatalStr(std::string const& message) const;

        void set_console_logger( ConsoleLogger* pConsoleLogger ) { pConsoleLogger_ = pConsoleLogger; };

        // temporarily mute Debug or Info messages
        void set_muted( gboolean muted ) { muted_ = muted; }

    private:
        void PrintConsoleMessage(const gchar* message, Parameters::LogLevel const logLevel) const;
};