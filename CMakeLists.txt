cmake_minimum_required(VERSION 3.19)

project(Absense_execute_GUI LANGUAGES CXX)

# =========================================================
# Qt
# =========================================================

find_package(Qt6 6.5 REQUIRED COMPONENTS
    Core
    Widgets
    Sql
    Network
)

qt_standard_project_setup()

# =========================================================
# EXECUTABLE
# =========================================================

qt_add_executable(Absense_execute_GUI
    WIN32
    MACOSX_BUNDLE

    # -----------------------------------------------------
    # Main
    # -----------------------------------------------------
    main.cpp
    figma_ui.h
    app_session.h

    # -----------------------------------------------------
    # Main Login
    # -----------------------------------------------------
    umams.cpp
    umams.h
    umams.ui

    # -----------------------------------------------------
    # Admin Login
    # -----------------------------------------------------
    adminlogin.cpp
    adminlogin.h
    adminlogin.ui

    # -----------------------------------------------------
    # Student Login
    # -----------------------------------------------------
    studentlogin.cpp
    studentlogin.h
    studentlogin.ui

    # Student self-registration
    studentregistration.cpp
    studentregistration.h
    studentregistration.ui

    # -----------------------------------------------------
    # Admin Dashboard
    # -----------------------------------------------------
    dashboard.cpp
    dashboard.h
    dashboard.ui

    # -----------------------------------------------------
    # Student Dashboard
    # -----------------------------------------------------
    studentdashboard.cpp
    studentdashboard.h
    studentdashboard.ui

    # -----------------------------------------------------
    # Student Profile
    # -----------------------------------------------------
    studentprofile.cpp
    studentprofile.h
    studentprofile.ui

    # -----------------------------------------------------
    # Change Password
    # -----------------------------------------------------
    changepassword.cpp
    changepassword.h
    changepassword.ui

    # -----------------------------------------------------
    # My Applications
    # -----------------------------------------------------
    myapplications.cpp
    myapplications.h
    myapplications.ui

    # -----------------------------------------------------
    # Medical Application
    # -----------------------------------------------------
    medicalapplication.cpp
    medicalapplication.h
    medicalapplication.ui

    # -----------------------------------------------------
    # View Applications
    # -----------------------------------------------------
    dialog.cpp
    dialog.h
    dialog.ui

    # -----------------------------------------------------
    # Application Details
    # -----------------------------------------------------
    applicationdetails.cpp
    applicationdetails.h
    applicationdetails.ui

    # -----------------------------------------------------
    # Approve / Decline
    # -----------------------------------------------------
    approvedecline.cpp
    approvedecline.h
    approvedecline.ui

    # -----------------------------------------------------
    # Notifications
    # -----------------------------------------------------
    notifications.cpp
    notifications.h
    notifications.ui

    # -----------------------------------------------------
    # Reports
    # -----------------------------------------------------
    reports.cpp
    reports.h
    reports.ui

    # Approved medical periods grouped by registration prefix and month
    monthlyapprovedmedicals.cpp
    monthlyapprovedmedicals.h
    monthlyapprovedmedicals.ui

    # -----------------------------------------------------
    # Database
    # -----------------------------------------------------
    database.cpp
    database.h
    password_hash.h
)

# =========================================================
# QSS RESOURCE
# =========================================================

qt_add_resources(Absense_execute_GUI
    "app_resources"
    PREFIX "/"
    FILES
        umams.qss
        assets/wayamba_logo.png
        assets/Inter.ttf
        assets/OFL-Inter.txt
)

# =========================================================
# Qt LIBRARIES
# =========================================================

target_link_libraries(Absense_execute_GUI
    PRIVATE
        Qt6::Core
        Qt6::Widgets
        Qt6::Sql
        Qt6::Network
)

# =========================================================
# APPLICATION SETTINGS
# =========================================================

set_target_properties(Absense_execute_GUI PROPERTIES
    WIN32_EXECUTABLE TRUE
)

# =========================================================
# INSTALL
# =========================================================

include(GNUInstallDirs)

install(
    TARGETS Absense_execute_GUI
    BUNDLE DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
# Use the installed MySQL client runtime beside this executable.
set(UMAMS_MYSQL_ROOT "C:/Program Files/MySQL/MySQL Server 8.0" CACHE PATH "Installed MySQL Server client runtime")
foreach(runtime IN ITEMS lib/libmysql.dll bin/libssl-3-x64.dll bin/libcrypto-3-x64.dll)
    if(EXISTS "${UMAMS_MYSQL_ROOT}/${runtime}")
        add_custom_command(TARGET Absense_execute_GUI POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${UMAMS_MYSQL_ROOT}/${runtime}" "$<TARGET_FILE_DIR:Absense_execute_GUI>")
    endif()
endforeach()
find_file(UMAMS_MYSQL_PLUGIN qsqlmysql.dll
    HINTS "${Qt6_DIR}/../../../plugins/sqldrivers" NO_DEFAULT_PATH)
if(UMAMS_MYSQL_PLUGIN)
    add_custom_command(TARGET Absense_execute_GUI POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:Absense_execute_GUI>/sqldrivers"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "${UMAMS_MYSQL_PLUGIN}"
            "$<TARGET_FILE_DIR:Absense_execute_GUI>/sqldrivers/qsqlmysql.dll")
endif()
