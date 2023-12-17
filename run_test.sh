#!/bin/bash

if [ "$2" = "local" ]; then
  ENDPOINT="failover:tcp://localhost:61616"
else
  ENDPOINT="failover:tcp://activemq.services.swangames.com:61616"  
fi

./TestingHarness -t lh-test -C  -q $ENDPOINT -f Scripts/$1.txt