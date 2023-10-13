#include <QtCore>

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickView>
#include <QQuickStyle>

#include "cameradiscovery.h"
#include "cameradevice.h"

#ifdef Q_OS_WIN32
#include <windows.h>
#endif

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    QCoreApplication::setOrganizationDomain("org.tal.cutepocketcamera");
    QCoreApplication::setOrganizationName("tal.org");
    QCoreApplication::setApplicationName("CutePocketCamera");
    QCoreApplication::setApplicationVersion("0.1");
    
#ifdef DEBUG
    QLoggingCategory::setFilterRules(QStringLiteral("qt.bluetooth* = true"));
#endif

    qmlRegisterType<CameraDevice>("org.tal", 1,0, "CameraDevice");
    qmlRegisterType<CameraDiscovery>("org.tal", 1,0, "CameraDiscovery");
    qRegisterMetaType<QBluetoothDeviceInfo *>("BluetoothDeviceInfo");

    QQuickStyle::setStyle("Universal");

    QQmlApplicationEngine engine;

    QObject::connect(&engine, &QQmlApplicationEngine::objectCreationFailed,
        &app, []() { QCoreApplication::exit(-1); },
        Qt::QueuedConnection);
    engine.loadFromModule("CutePocketRemote", "Main");
    
#ifdef Q_OS_WIN32
    SetThreadExecutionState(ES_CONTINUOUS | ES_DISPLAY_REQUIRED);
#endif

    return app.exec();
}
