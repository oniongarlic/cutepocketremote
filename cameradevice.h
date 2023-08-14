#ifndef CAMERADEVICE_H
#define CAMERADEVICE_H

#include <QtCore>

#include <QBluetoothDeviceDiscoveryAgent>
#include <QBluetoothDeviceInfo>
#include <QLowEnergyController>
#include <QBluetoothUuid>

#include <QtQmlIntegration/qqmlintegration.h>

QT_BEGIN_NAMESPACE
class QBluetoothDeviceInfo;
class QBluetoothUuid;
QT_END_NAMESPACE

class CameraDevice: public QObject
{
    Q_OBJECT

    Q_PROPERTY(bool disocvering READ discovering NOTIFY discoveringChanged)
    Q_PROPERTY(bool controllerError READ hasControllerError NOTIFY controllerErrorChanged)

    Q_PROPERTY(bool recording READ recording NOTIFY recordingChanged)

    Q_PROPERTY(bool connected READ isConnected NOTIFY connectedChanged)

    Q_PROPERTY(QString name READ name NOTIFY nameChanged)

    Q_PROPERTY(int status READ status NOTIFY statusChanged)

    Q_PROPERTY(int wb READ wb NOTIFY wbChanged)
    Q_PROPERTY(int tint READ tint NOTIFY tintChanged)

    Q_PROPERTY(int zoom READ zoom NOTIFY zoomChanged)

    Q_PROPERTY(QTime timecode READ timecode NOTIFY timecodeChanged)

    QML_ELEMENT
    QML_SINGLETON

public:
    CameraDevice();
    ~CameraDevice();
    QVariant getDevices();

    bool isConnected() const;

    bool discovering();
    bool hasControllerError() const;

    bool recording() const;

    QTime timecode() const;

    int status() const;

    int wb() const;

    int tint() const;

    QString name() const;

    int zoom() const;
    
public slots:
    void startDeviceDiscovery();
    void stopDeviceDiscovery();
    
    void disconnectFromDevice();

    bool autoFocus();
    bool autoAperture();
    bool shutterSpeed(qint32 shutter);
    bool gain(qint8 gain);

    bool colorCorrectionReset();

    bool record(bool record);
    bool play(bool play);
    bool captureStill();
    
    bool whiteBalance(qint16 wb, qint16 tint);
    
private slots:
    void scanServices(const QString &address);
    void scanServices(const QBluetoothDeviceInfo &device);
    
    void connectToService(const QString &uuid);
    
    void addCameraDevice(const QBluetoothDeviceInfo&);
    void deviceScanFinished();
    void deviceScanError(QBluetoothDeviceDiscoveryAgent::Error);

    void addLowEnergyService(const QBluetoothUuid &uuid);
    void deviceConnected();
    void errorReceived(QLowEnergyController::Error);
    void serviceScanDone();
    void deviceDisconnected();

    void serviceDetailsDiscovered(QLowEnergyService::ServiceState newState);

    void characteristicChanged(QLowEnergyCharacteristic characteristic, QByteArray value);
    void serviceStateChanged(QLowEnergyService::ServiceState s);
    void confirmedDescriptorWrite(const QLowEnergyDescriptor &d, const QByteArray &value);

Q_SIGNALS:
    void devicesUpdated();        
    void updateChanged();
    void discoveringChanged();
    void disconnected();

    void discoveryStart();
    void discoveryStop(qsizetype devices);

    void controllerErrorChanged();

    void connectedChanged();
    void recordingChanged();
    void timecodeChanged();
    void statusChanged();

    void wbChanged();

    void tintChanged();

    void nameChanged();

    void zoomChanged();

protected:
    void handleLensData(const QByteArray &data);
    void handleVideoData(const QByteArray &data);
    void handleAudioData(const QByteArray &data);
    void handleOutputData(const QByteArray &data);
    void handleMediaData(const QByteArray &data);
    void handleDisplayData(const QByteArray &data);
    void handleTallyData(const QByteArray &data);
    void handleReferenceData(const QByteArray &data);
    void handleConfigData(const QByteArray &data);
    void handleColorData(const QByteArray &data);
    void handleMetaData(const QByteArray &data);

private:
    bool writeCameraCommand(const QByteArray &cmd);

    QBluetoothDeviceDiscoveryAgent *m_discoveryAgent;

    QBluetoothDeviceInfo *m_currentDevice;

    QHash<QString, QBluetoothDeviceInfo *> m_cameras;

    QList<QLowEnergyService *> m_services;
    QHash<QBluetoothUuid, QLowEnergyService *> m_servicess;

    bool m_connected = false;

    QLowEnergyController *m_controller = nullptr;
    QLowEnergyService *m_cameraService = nullptr;
    QLowEnergyCharacteristic *m_cameraOutgoing = nullptr;
    
    bool m_discovering = false;

    // Camera state
    QString m_name;
    qint8 m_status;
    QTime m_timecode;
    bool m_recording;
    qint8 m_gain;
    qint16 m_zoom;
    qint16 m_wb;
    qint16 m_tint;
    qint32 m_shutterSpeed;
    double m_aperture;
};

#endif // CAMERADEVICE_H
