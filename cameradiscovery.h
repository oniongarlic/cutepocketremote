#ifndef CAMERADISCOVERY_H
#define CAMERADISCOVERY_H

#include <QObject>
#include <QQmlEngine>

#include <QBluetoothDeviceDiscoveryAgent>
#include <QBluetoothDeviceInfo>

class CameraDiscovery : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool discovering READ discovering NOTIFY discoveringChanged FINAL)
    Q_PROPERTY(int count READ count NOTIFY countChanged FINAL)
    Q_PROPERTY(DiscoveryStatus status READ status NOTIFY statusChanged FINAL)
    QML_ELEMENT
    QML_SINGLETON
public:
    enum DiscoveryStatus {
        DiscoveryIdle,
        DiscoveryActive,
        DiscoveryDone,
        DiscoveryDisabled,
        DiscoveryError,
    };
    Q_ENUM(DiscoveryStatus)
    explicit CameraDiscovery(QObject *parent = nullptr);
    
    ~CameraDiscovery();
    int count() const;

    CameraDiscovery::DiscoveryStatus status() const;
    
    Q_INVOKABLE void startDeviceDiscovery();
    Q_INVOKABLE void stopDeviceDiscovery();

    Q_INVOKABLE QVariantList getDevices();
    Q_INVOKABLE QString getDefaultDevice();
    Q_INVOKABLE QBluetoothDeviceInfo *getBluetoothDevice(const QString &address);
    
signals:
    void discoveryStart();
    void discoveryStop(qsizetype devices);
    void devicesUpdated();
    void statusChanged(CameraDiscovery::DiscoveryStatus discoveryStatus);
    void discoveringChanged();
    void discoveryError();
    void noDeviceError();
    void countChanged();
    
protected:
    bool discovering();
private slots:
    void addCameraDevice(const QBluetoothDeviceInfo&);
    void deviceScanFinished();
    void deviceScanError(QBluetoothDeviceDiscoveryAgent::Error);
    
private:
    QBluetoothDeviceDiscoveryAgent *m_discoveryAgent;
    QVariantList m_cameras;
    QMap<QString, QBluetoothDeviceInfo *> m_devices;
    bool m_discovering=false;
    void clearDevices();
    DiscoveryStatus m_status=DiscoveryIdle;
};

#endif // CAMERADISCOVERY_H
