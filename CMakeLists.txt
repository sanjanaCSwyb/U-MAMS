cmake_minimum_required(VERSION 3.19)

project(Absense_execute_GUI LANGUAGES CXX)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Widgets Sql)

qt_standard_project_setup()

qt_add_executable(Absense_execute_GUI
    WIN32
    MACOSX_BUNDLE

    main.cpp

    umams.cpp
    umams.h
    umams.ui

    dashboard.cpp
    dashboard.h
    dashboard.ui

    medicalapplication.cpp
    medicalapplication.h
    medicalapplication.ui

    dialog.cpp
    dialog.h
    dialog.ui

    applicationdetails.cpp
    applicationdetails.h
    applicationdetails.ui
    database.h database.cpp

    database.cpp
    database.h
    approvedecline.h approvedecline.cpp approvedecline.ui
    notifications.h notifications.cpp notifications.ui
    reports.h reports.cpp reports.ui
    studentlogin.h studentlogin.cpp studentlogin.ui
    studentdashboard.h studentdashboard.cpp studentdashboard.ui
    myapplications.h myapplications.cpp myapplications.ui
    studentprofile.h studentprofile.cpp studentprofile.ui
    changepassword.h changepassword.cpp changepassword.ui
    adminlogin.h adminlogin.cpp adminlogin.ui
)

target_link_libraries(Absense_execute_GUI
    PRIVATE
        Qt6::Core
        Qt6::Widgets
        Qt6::Sql
)

install(TARGETS Absense_execute_GUI
    BUNDLE DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

qt_generate_deploy_app_script(
    TARGET Absense_execute_GUI
    OUTPUT_SCRIPT deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
)

install(SCRIPT ${deploy_script})