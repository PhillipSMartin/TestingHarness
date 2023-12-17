#pragma once

class AMQClient;
class Logger;
class Parameters;
class DestinationMap;

class CLI
{
    private:
        Logger* pLogger_;
        AMQClient* pAMQClient_;
        Parameters* pParms_;

    public:
        CLI(Parameters* pParms, AMQClient* pAMQClient, Logger* pLogger) : 
            pParms_{ pParms }, 
            pAMQClient_{ pAMQClient }, 
            pLogger_{ pLogger } {}
        void run();
};