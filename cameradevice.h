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

    Q_PROPERTY(bool controllerError READ hasControllerError NOTIFY controllerErrorChanged FINAL)

    Q_PROPERTY(bool recording READ recording NOTIFY recordingChanged FINAL)
    Q_PROPERTY(bool playing READ playing NOTIFY playingChanged FINAL)

    Q_PROPERTY(bool connected READ isConnected NOTIFY connectedChanged FINAL)

    Q_PROPERTY(QString name READ name NOTIFY nameChanged FINAL)

    Q_PROPERTY(int status READ status NOTIFY statusChanged FINAL)

    Q_PROPERTY(int wb READ wb NOTIFY wbChanged FINAL)
    Q_PROPERTY(int tint READ tint NOTIFY tintChanged FINAL)

    Q_PROPERTY(double aperture READ apterture NOTIFY apertureChanged FINAL)
    
    Q_PROPERTY(int iso READ iso NOTIFY isoChanged)
    
    Q_PROPERTY(int shutterSpeed READ shutterSpeed NOTIFY shutterSpeedChanged FINAL)

    Q_PROPERTY(int zoom READ zoom NOTIFY zoomChanged FINAL)

    Q_PROPERTY(QTime timecode READ timecode NOTIFY timecodeChanged FINAL)
    Q_PROPERTY(bool timecodeDisplay READ timecodeDisplay NOTIFY timecodeDisplayChanged FINAL)

    QML_ELEMENT
    QML_SINGLETON

public:
    CameraDevice();
    ~CameraDevice();

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

    double apterture() const;

    bool playing() const;
    
    int iso() const;
    
    int shutterSpeed() const;
    
    bool timecodeDisplay() const;
    
public slots:
    void connectDevice(QBluetoothDeviceInfo *device);
    void disconnectFromDevice();

    bool setCameraName(const QString name);

    bool autoFocus();
    bool autoAperture();

    bool setShutterSpeed(qint32 shutter);
    bool setGain(qint8 gain);
    bool setISO(qint32 is);

    bool setAperture(double ap);
    bool setApertureNormalized(double ap);
    bool setApertureStep(quint16 apstep);
    
    bool colorCorrectionReset();

    bool record(bool record);
    bool play(bool play);
    bool captureStill();
    
    bool whiteBalance(qint16 wb, qint16 tint);
    bool autoWhitebalance();
    bool restoreAutoWhiteBalance();

    bool focus(qint16 focus);
    bool playback(bool next);
    bool colorLift(double r, double g, double b, double l);
    bool colorGamma(double r, double g, double b, double l);
    bool colorGain(double r, double g, double b, double l);
    bool colorOffset(double r, double g, double b, double l);
    bool setDisplay(bool tc);

private slots:
    void scanServices(const QString &address);
    void scanServices(const QBluetoothDeviceInfo &device);
    
    void connectToService(const QString &uuid);

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
    void disconnected();
    
    void connectionFailure();
    void controllerErrorChanged();

    void connectedChanged();
    void recordingChanged();
    void timecodeChanged();
    void statusChanged();

    void wbChanged();

    void tintChanged();

    void nameChanged();

    void zoomChanged();

    void apertureChanged();

    void playingChanged();
    
    void isoChanged();
    
    void shutterSpeedChanged();
    
    void timecodeDisplayChanged();
    
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
    void handleStatusData(const QByteArray &data);    
    void handleMetaData(const QByteArray &data);

    bool colorControl(uint8_t c, double r, double g, double b, double l);
private:
    bool writeCameraCommand(const QByteArray &cmd);
    bool writeCameraName(const QString &name);

    QBluetoothDeviceInfo *m_currentDevice=nullptr;

    QHash<QString, QBluetoothDeviceInfo *> m_cameras;

    QList<QLowEnergyService *> m_services;
    QHash<QBluetoothUuid, QLowEnergyService *> m_servicess;

    bool m_connected = false;

    QLowEnergyController *m_controller = nullptr;
    QLowEnergyService *m_cameraService = nullptr;
    QLowEnergyCharacteristic *m_cameraOutgoing = nullptr;
    QLowEnergyCharacteristic *m_cameraName = nullptr;
    
    bool m_discovering = false;

    // Camera state
    QString m_name;
    qint8 m_status = 0;
    QTime m_timecode;
    bool m_recording = false;
    bool m_playing = false;
    qint8 m_gain = 0;
    qint32 m_iso = 100;
    qint32 m_exposure = 0;
    qint16 m_zoom = 0;
    qint16 m_wb = 4600;
    qint16 m_tint = 0;
    qint32 m_shutterSpeed=0;
    double m_aperture=0.0;
    double m_aperture_norm=0.0;
    
    bool m_timecodeDisplay=false;

    quint8 m_codec=0;
    quint8 m_codec_variant=0;
    quint8 m_media_speed=0;
    quint8 m_media_slot_1;
    quint8 m_media_slot_2;

    qint16 m_meta_reel;
    qint8 m_meta_tags;
    qint8 m_meta_location;
    qint8 m_meta_day;
    QString m_meta_scene;
    qint8 m_meta_take_number;
    qint8 m_meta_take_tags;
    QString m_meta_camera_id;
    QString m_meta_camera_operator;
    QString m_meta_director;
    QString m_meta_project_name;
    QString m_meta_lens_type;
    QString m_meta_lens_iris;
    QString m_meta_lens_focal;
    QString m_meta_lens_distance;
    QString m_meta_lens_filter;
    qint8 m_meta_slate_mode;
    QString m_meta_slate_target;
};

#endif // CAMERADEVICE_H
