#include <QtCore>

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickView>
#include <QQuickStyle>

#include "cameradevice.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    QCoreApplication::setOrganizationDomain("org.tal.cutepocketcamera");
    QCoreApplication::setOrganizationName("tal.org");
    QCoreApplication::setApplicationName("CutePocketCamera");
    QCoreApplication::setApplicationVersion("0.1");

    QLoggingCategory::setFilterRules(QStringLiteral("qt.bluetooth* = true"));    

    qmlRegisterType<CameraDevice>("org.tal", 1,0, "CameraDevice");

    QQuickStyle::setStyle("Universal");

    QQmlApplicationEngine engine;

    QObject::connect(&engine, &QQmlApplicationEngine::objectCreationFailed,
        &app, []() { QCoreApplication::exit(-1); },
        Qt::QueuedConnection);
    engine.loadFromModule("CutePocketRemote", "Main");

    return app.exec();
}
