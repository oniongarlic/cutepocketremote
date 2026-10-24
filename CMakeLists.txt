cmake_minimum_required(VERSION 3.16)

project(cpr VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 6.6 REQUIRED COMPONENTS Bluetooth Core Gui Quick QuickControls2)

set(app_icon_resource_windows "${CMAKE_CURRENT_SOURCE_DIR}/icon.rc")

qt_standard_project_setup(REQUIRES 6.6)

qt_add_executable(cpr
    main.cpp
    ${app_icon_resource_windows}
)

qt_add_qml_module(cpr
    URI CutePocketRemote
    VERSION 1.0
    QML_FILES Main.qml
    SOURCES cameradevice.h cameradevice.cpp
    SOURCES cameratypes.h
    SOURCES cameradiscovery.h cameradiscovery.cpp
    QML_FILES TimeCodeText.qml
    QML_FILES RelativeFocus.qml
    QML_FILES AbsoluteFocus.qml
    QML_FILES Zoom.qml
    QML_FILES WhiteBalanceButton.qml
    QML_FILES ApertureButton.qml
    QML_FILES ISOButton.qml
    QML_FILES ColorDrawer.qml
    QML_FILES ColorSlider.qml
    QML_FILES ColorInterface.qml
    QML_FILES FpsButton.qml
    QML_FILES ShutterSpeedButton.qml
)

target_link_libraries(cpr PUBLIC
    Qt::Bluetooth
    Qt::Core
    Qt::Gui
    Qt::Quick
    Qt::QuickControls2
)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
set_target_properties(cpr PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER org.tal.CutePocketRemote
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

if(APPLE)
    set_target_properties(cpr PROPERTIES MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/Info.plist")
endif()

target_link_libraries(cpr
    PRIVATE Qt6::Quick
)

if(WIN32)
    set(CMAKE_INSTALL_PREFIX "C:/Export/Qt/CutePocketRemote")
endif()

include(GNUInstallDirs)
install(TARGETS cpr
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

qt_generate_deploy_qml_app_script(
    TARGET cpr
    OUTPUT_SCRIPT deploy_qml_script
    NO_UNSUPPORTED_PLATFORM_ERROR
    DEPLOY_USER_QML_MODULES_ON_UNSUPPORTED_PLATFORM
    NO_TRANSLATIONS
)

install(SCRIPT ${deploy_qml_script})