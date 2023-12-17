#!/bin/bash

if [ "$1" = "local" ]; then
  ENDPOINT="failover:tcp://localhost:61616"
else
  ENDPOINT="failover:tcp://activemq.services.swangames.com:61616"  
fi

./TestingHarness -t lh-test -q $ENDPOINT 