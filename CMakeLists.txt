cmake_minimum_required(VERSION 3.0.0)
project(TestingHarness VERSION 0.1.0)

include(CTest)
enable_testing()

add_executable(UnitTests
    src/UnitTests.cpp
    src/AMQClient.cpp
    src/AMQDestination.cpp
    src/AMQQueue.cpp
    src/AMQTopic.cpp
    src/DestinationId.cpp
    src/DestinationMap.cpp
    src/Logger.cpp
    src/Parameters.cpp
    src/FileUtilities)

    target_link_directories(UnitTests PUBLIC lib)

    target_include_directories(UnitTests 
    PRIVATE
        include
    PUBLIC 
        include/catch2
        include/amq
        include/apr-1
        /usr/include/gtk-3.0 
        /usr/include/atk-1.0 
        /usr/include/at-spi2-atk/2.0 
        /usr/include/pango-1.0
        /usr/include/gio-unix-2.0/ 
        /usr/include/cairo 
        /usr/include/gdk-pixbuf-2.0 
        /usr/include/glib-2.0 
        /usr/lib64/glib-2.0/include 
        /usr/include/at-spi-2.0 
        /usr/include/dbus-1.0 
        /usr/lib64/dbus-1.0/include 
        /usr/include/harfbuzz 
        /usr/include/freetype2 
        /usr/include/fribidi 
        /usr/include/libpng15 
        /usr/include/uuid 
        /usr/include/pixman-1 
        /usr/include/libdrm
    )
    
    target_link_libraries(UnitTests 
    PUBLIC
        logging
        activemq-cpp 
        apr-1
        gtk-3 
        gdk-3 
        atk-1.0 
        gio-2.0 
        pangocairo-1.0 
        gdk_pixbuf-2.0 
        cairo-gobject 
        pango-1.0 
        cairo 
        gobject-2.0 
        glib-2.0
        )

add_executable(TestingHarness 
    src/AMQClient.cpp
    src/AMQDestination.cpp
    src/AMQQueue.cpp
    src/AMQTopic.cpp
    src/CLI.cpp
    src/ConsoleViewer.cpp
    src/Dashboard.cpp
    src/DestinationChooser.cpp
    src/DestinationId.cpp
    src/DestinationListStore.cpp
    src/DestinationMap.cpp
    src/FileUtilities.cpp
    src/GtkUtilities.cpp
    src/HelpViewer.cpp 
    src/Main.cpp 
    src/JsonEditor.cpp
    src/JsonTreeStore.cpp
    src/JsonUtilities.cpp
    src/JsonViewer.cpp
    src/Logger.cpp 
    src/LogParser.cpp 
    src/MessageStore.cpp
    src/NavigationBar.cpp
    src/Parameters.cpp 
    src/ScriptParser.cpp
    src/SignalHandler.cpp
    src/TemplateChooser.cpp
    src/TemplateListStore.cpp
    src/TestCommand.cpp
    src/TestManager.cpp
    src/TourneyIdChooser.cpp)

target_link_directories(TestingHarness PUBLIC lib)

target_include_directories(TestingHarness 
PRIVATE
    include
PUBLIC 
    include/amq
    include/apr-1
    /usr/include/gtk-3.0 
    /usr/include/atk-1.0 
    /usr/include/at-spi2-atk/2.0 
    /usr/include/pango-1.0
    /usr/include/gio-unix-2.0/ 
    /usr/include/cairo 
    /usr/include/gdk-pixbuf-2.0 
    /usr/include/glib-2.0 
    /usr/lib64/glib-2.0/include 
    /usr/include/at-spi-2.0 
    /usr/include/dbus-1.0 
    /usr/lib64/dbus-1.0/include 
    /usr/include/harfbuzz 
    /usr/include/freetype2 
    /usr/include/fribidi 
    /usr/include/libpng15 
    /usr/include/uuid 
    /usr/include/pixman-1 
    /usr/include/libdrm
    )

target_link_libraries(TestingHarness 
PUBLIC
    logging
    activemq-cpp 
    apr-1
    gtk-3 
    gdk-3 
    atk-1.0 
    gio-2.0 
    pangocairo-1.0 
    gdk_pixbuf-2.0 
    cairo-gobject 
    pango-1.0 
    cairo 
    gobject-2.0 
    glib-2.0
    )

configure_file(Templates/Event.tb.templates.txt Event.tb.templates.txt COPYONLY)
configure_file(Templates/Event.tm.templates.txt Event.tm.templates.txt COPYONLY)
configure_file(Templates/Destinations.txt Destinations.txt COPYONLY)
configure_file(Templates/Q.MasterpointLaunch.templates.txt Q.MasterpointLaunch.templates.txt COPYONLY)
configure_file(Templates/Q.SectionLaunch.templates.txt Q.SectionLaunch.templates.txt COPYONLY)
configure_file(Templates/State.tm.templates.txt State.tm.templates.txt COPYONLY)
configure_file(TestScripts/StartTourney.txt Scripts/StartTourney.txt COPYONLY)
configure_file(TestScripts/Diagnostics.txt Scripts/Diagnostics.txt COPYONLY)
configure_file(TestScripts/DaylongTable.txt Scripts/DaylongTable.txt COPYONLY)
configure_file(TestScripts/DaylongTableRestart.txt Scripts/DaylongTableRestart.txt COPYONLY)
configure_file(TestScripts/FullTourney.txt Scripts/FullTourney.txt COPYONLY)
configure_file(TestScripts/FullDaylong.txt Scripts/FullDaylong.txt COPYONLY)
configure_file(TestScripts/PauseAndRestart.txt Scripts/PauseAndRestart.txt COPYONLY)
configure_file(TestScripts/OneBoardDaylong.txt Scripts/OneBoardDaylong.txt COPYONLY)
configure_file(TestScripts/TwoBoardDaylong.txt Scripts/TwoBoardDaylong.txt COPYONLY)
configure_file(TestScripts/ZeroTableDaylong.txt Scripts/ZeroTableDaylong.txt COPYONLY)
configure_file(TestScripts/LaunchTourney.txt Scripts/LaunchTourney.txt COPYONLY)
configure_file(TestScripts/LaunchDaylong.txt Scripts/LaunchDaylong.txt COPYONLY)
configure_file(TestScripts/LaunchLargeDaylong.txt Scripts/LaunchLargeDaylong.txt COPYONLY)
configure_file(TestScripts/LaunchRobotDaylong.txt Scripts/LaunchRobotDaylong.txt COPYONLY)
configure_file(TestScripts/AllLaunchTests.txt Scripts/AllLaunchTests.txt COPYONLY)
configure_file(TestScripts/AllSessionTests.txt Scripts/AllSessionTests.txt COPYONLY)
configure_file(TestScripts/AllRobotTests.txt Scripts/AllRobotTests.txt COPYONLY)
configure_file(TestScripts/DaylongWithRobots.txt Scripts/DaylongWithRobots.txt COPYONLY)
configure_file(TestScripts/DaylongWithRobotsRestart.txt Scripts/DaylongWithRobotsRestart.txt COPYONLY)
configure_file(TestScripts/DaylongWithRobotsAllSeats.txt Scripts/DaylongWithRobotsAllSeats.txt COPYONLY)
configure_file(TestScripts/ScoreTourney.txt Scripts/ScoreTourney.txt COPYONLY)
configure_file(TestScripts/Sitout.txt Scripts/Sitout.txt COPYONLY)
configure_file(TestScripts/README.txt Scripts/README.txt COPYONLY)
configure_file(TestScripts/RequestStateDump.txt Scripts/RequestStateDump.txt COPYONLY)
configure_file(TestScripts/StartTwoSections.txt Scripts/StartTwoSections.txt COPYONLY)
configure_file(TestScripts/TestMasterpoint.txt Scripts/TestMasterpoint.txt COPYONLY)
configure_file(lib/libactivemq-cpp.so.19 libactivemq-cpp.so.19 COPYONLY)
configure_file(lib/libapr-1.so.0 libapr-1.so.0 COPYONLY)
configure_file(lib/libssl.so.1.0.0 libssl.so.1.0.0 COPYONLY)
configure_file(lib/libcrypto.so.1.0.0 libcrypto.so.1.0.0 COPYONLY)
configure_file(wait-for-it.sh wait-for-it.sh COPYONLY)
configure_file(install.sh install.sh COPYONLY)
configure_file(run_test.sh run_test.sh COPYONLY)
configure_file(run_GUI.sh run_GUI.sh COPYONLY)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
