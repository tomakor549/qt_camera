cmake_minimum_required(VERSION 3.16)

project(QtCamera VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(QT_QML_GENERATE_QMLLS_INI ON)

find_package(Qt6 REQUIRED COMPONENTS Quick Multimedia Core Gui)

qt_standard_project_setup(REQUIRES 6.8)

qt_add_executable(appQtCamera
    main.cpp
)

qt_add_qml_module(appQtCamera
    URI QtCamera
    QML_FILES
        Main.qml
        SOURCES
        SOURCES cameramanager.h cameramanager.cpp
        SOURCES
        RESOURCES
        RESOURCES resources.qrc
        SOURCES
)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
set_target_properties(appQtCamera PROPERTIES
#    MACOSX_BUNDLE_GUI_IDENTIFIER com.example.appQtCamera
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

target_link_libraries(appQtCamera PRIVATE
    Qt6::Core
        Qt6::Gui
        Qt6::Multimedia
        Qt6::Quick
)
target_link_libraries(appQtCamera PRIVATE Qt6::Multimedia)

install(TARGETS appQtCamera
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
