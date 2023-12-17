#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include "AMQClient.h"
#include "DestinationMap.h"
#include "Logger.h"

class TestListener : public AMQListener
{
   virtual void OnAMQMessage( DestinationId const& destinationId, const char* message ) {}
};

TEST_CASE("ActiveMQ")
{
   Parameters _parms{};
   TestListener _listener{};

   _parms.set_console_log_level(Parameters::LogLevel::DEBUG);
   Logger _logger{&_parms};  

   AMQClient _AMQClient{ &_logger, false };
   REQUIRE(_AMQClient.start( "failover:tcp://localhost:61616" ) );

   DestinationId _queue{ "tournamentLaunched8", DestinationType::QUEUE};
   _AMQClient.subscribe(_queue, &_listener );  

   _AMQClient.stop();
}

TEST_CASE("Build destination names","[.]")
{
   Logger _logger{new Parameters{}};
   DestinationMap _destinations{ &_logger };
   bool _r = _destinations.import_destinations( "Destinations.txt" );
   REQUIRE( _r );

   SECTION("Event.tb returns Event.tb=lh-test-1")
   {
      gchar prefix_[] = "Event.tb";
      gchar* template_ = _destinations.get_destination_name_template(prefix_);
      std::string destinationName_ =  DestinationId::build_destination_name( 
            prefix_, 
            "lh-test", 
            1, 
            template_);

      REQUIRE( destinationName_ == "Event.tb=lh-test-1");
   }

   SECTION("Event.tm returns Event.tm=lh-test-0-0")
   {
      gchar prefix_[] = "Event.tm";
      gchar* template_ = _destinations.get_destination_name_template(prefix_);
      std::string destinationName_ =  DestinationId::build_destination_name( 
            prefix_,
            "lh-test",
            1, 
            template_);

      REQUIRE( destinationName_ == "Event.tm=lh-test-0");
   }

   SECTION("Q.SectionLaunch returns Q.SectionLaunch")
   {
      gchar prefix_[] = "Q.SectionLaunch";
      gchar* template_ = _destinations.get_destination_name_template(prefix_);
      std::string destinationName_ =  DestinationId::build_destination_name( 
            prefix_, 
            "lh-test",
            1, 
            template_);

      REQUIRE( destinationName_ == "Q.SectionLaunch");
   }
}
