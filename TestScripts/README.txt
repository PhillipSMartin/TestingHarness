
Setup for all Tests (needs to be done only once)
------------------------------------------------------
------------------------------------------------------
    run 
        sudo activemq start 
    (if using a remote ActiveMQ broker, omit this step and omit 'local' from all the run_test commands belwo)


Regression Tests
------------------------------------------------------
------------------------------------------------------

1. Launch tests - call SessionLauncher to launch standard or daylong tournaments
------------------------------------------------------

 Runs three tests:
   StartTourney
   LaunchTourney
   LaunchDaylong

 To test
    go to Bridge/backend directory and execute
        bin/run_setup.sh (kills any existing Sections and Tables and clean the /tmp directory of logfiles)
        bin/run_SessionLauncher.sh (starts the Launcher process)
    go to TestingHarness/build directory and execute
        ./run_test.sh AllLaunchTests local


2. Session tests - runs sessions directly without calling launcher
------------------------------------------------------

 Runs five tests:
   FullTourney
   OneBoardDaylong
   TwoBoardDaylong
   FullDaylong
   PauseAndRestart

 To test
   go to Bridge/backend directory and execute
        bin/run_setup.sh (kills any existing Sections and Tables and clean the /tmp directory of logfiles)
   go to TestingHarness/build directory and execute
       ./run_test.sh AllSessionTests local
   go to Bridge/backend directory and execute
       bin/run_all_session_tests.sh 

3. Robot tests - runs daylong sessions using (rather than simulating) robots
------------------------------------------------------

 Runs two tests:
   DaylongWithRobots
   DaylongWithRobotsRestart

 To test
   go to Bridge/backend directory and execute
        bin/run_setup.sh (kills any existing Sections and Tables and clean the /tmp directory of logfiles)
   go to TestingHarness/build directory and execute
       ./run_test.sh AllRobotTests local
   go to Bridge/backend directory and execute
       bin/run_all_robot_tests.sh 

Individual Standard Tourney Tests
------------------------------------------------------
------------------------------------------------------

1. Start a Standard Tournament
------------------------------------------------------

    go to Bridge/backend directory and execute
        bin/run_setup.sh (kills any existing Sections and Tables and clean the /tmp directory of logfiles)
        bin/run_SessionLauncher.sh (starts the Launcher process)
    go to TestingHarness/build directory and execute
        ./run_test.sh StartTourney local 

2. Launch a Standard Tournament and play first board
------------------------------------------------------
 
    go to Bridge/backend directory and execute
        bin/run_setup.sh (kills any existing Sections and Tables and clean the /tmp directory of logfiles)
        bin/run_SessionLauncher.sh (starts the Launcher process)
    go to TestingHarness/build directory and execute
        ./run_test.sh LaunchTourney local 

3. Run a full tournament with pre-selected boards:
------------------------------------------------------

    go to Bridge/backend directory and execute
        bin/run_setup.sh (kills any existing Sections and Tables and clean the /tmp directory of logfiles)
        (do not start SessionLauncher)
    go to TestingHarness/build directory and execute
        ./run_test.sh FullTourney local
    go to Bridge/backend directory and execute
        bin/run_standard_tourney.sh 

Individual Daylong Tournament Tests
------------------------------------------------------
------------------------------------------------------

1. Launch a Daylong Tournament
------------------------------------------------------
 
    go to Bridge/backend directory and execute
        bin/run_setup.sh (kills any existing Sections and Tables and clean the /tmp directory of logfiles)
        bin/run_SessionLauncher.sh (starts the Launcher process)
    go to TestingHarness/build directory and execute
        ./run_test.sh LaunchDaylong local 

2 Run a One-Board Daylong Tournament
------------------------------------------------------

    go to Bridge/backend directory and execute
        bin/run_setup.sh (kills any existing Sections and Tables and clean the /tmp directory of logfiles)
        (do not start SessionLauncher)
    go to TestingHarness/build directory and execute
        ./run_test.sh OneBoardDaylong local
    go to Bridge/backend directory and execute
        bin/run_daylong_tourney.sh 1
        (be sure to specify 1)

3. Run a Two-Board Daylong Tournament
------------------------------------------------------

    go to Bridge/backend directory and execute
        bin/run_setup.sh (kills any existing Sections and Tables and clean the /tmp directory of logfiles)
        (do not start SessionLauncher)
    go to TestingHarness/build directory and execute
        ./run_test.sh TwoBoardDaylong local
    go to Bridge/backend directory and execute
        bin/run_daylong_tourney.sh 2
        (be sure to specify 2)


4. Run a Full Daylong Tournament
------------------------------------------------------

    go to Bridge/backend directory and execute
        bin/run_setup.sh (kills any existing Sections and Tables and clean the /tmp directory of logfiles)
        (do not start SessionLauncher)
    go to TestingHarness/build directory and execute
        ./run_test.sh FullDaylong local
    go to Bridge/backend directory and execute
        bin/run_daylong_tourney.sh

5. Test Pausing and Restarting a Table
------------------------------------------------------

    go to Bridge/backend directory and execute
        bin/run_setup.sh (kills any existing Sections and Tables and clean the /tmp directory of logfiles)
        (do not start SessionLauncher)
    go to TestingHarness/build directory and execute
        ./run_test.sh PauseAndRestart local
    go to Bridge/backend directory and execute
        bin/run_daylong_tourney.sh 2
        (be sure to specify 2)

6. Test Daylong with Robots
------------------------------------------------------

    go to Bridge/backend directory and execute
        bin/run_setup.sh (kills any existing Sections and Tables and clean the /tmp directory of logfiles)
    go to TestingHarness/build directory and execute
        ./run_test.sh DaylongWithRobots local
    go to Bridge/backend directory and execute
        bin/run_daylong_with_robots.sh 1
        (be sure to specify 1)

7. Test Daylong with Robots Restart
------------------------------------------------------

    go to Bridge/backend directory and execute
        bin/run_setup.sh (kills any existing Sections and Tables and clean the /tmp directory of logfiles)
    go to TestingHarness/build directory and execute
        ./run_test.sh DaylongWithRobotsRestart local
    go to Bridge/backend directory and execute
        bin/run_daylong_with_robots.sh 1
        (be sure to specify 1)

Debugging
------------------------------------------------------
------------------------------------------------------

1. Debug a Daylong Table
------------------------------------------------------

    go to Bridge/backend directory and execute
        bin/run_setup.sh (kills any existing Sections and Tables and clean the /tmp directory of logfiles)
        bin/run_debug_daylong_table.sh (creates lh-test.cfg file)
    make sure launch.json file, in "Run Table" section, has the following parameters:
        "args": ["-c/tmp/lh-test.cfg", "-tlh-test-0-1"]
    if you wish to resume from a statedump, create the statedump and specify "-r/tmp/statedump_lh-test-0-1.json"
    go to TestingHarness/build directory and execute
        ./run_test.sh DaylongTable local 
        (after adding any additional messages you may wish to pass for debugging to build/scripts/DaylongTable.txt)   
    in "Run and Debug" window, select "Run Table" and click arrow or press F5 to start debugger
