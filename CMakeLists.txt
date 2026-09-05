cmake_minimum_required(VERSION 3.19)
project(Absense_execuse_GUI LANGUAGES CXX)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Widgets)

qt_standard_project_setup()

qt_add_executable(Absense_execuse_GUI
    WIN32 MACOSX_BUNDLE
    main.cpp
    umams.cpp
    umams.h
    umams.ui
    dashboard.h dashboard.cpp dashboard.ui
    medicalapplication.h medicalapplication.cpp medicalapplication.ui
)

target_link_libraries(Absense_execuse_GUI
    PRIVATE
        Qt::Core
        Qt::Widgets
)

install(TARGETS Absense_execuse_GUI
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

qt_generate_deploy_app_script(
    TARGET Absense_execuse_GUI
    OUTPUT_SCRIPT deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
)
install(SCRIPT ${deploy_script})
