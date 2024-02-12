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
    QML_ELEMENT
    QML_SINGLETON
public:
    explicit CameraDiscovery(QObject *parent = nullptr);
    
    ~CameraDiscovery();
    int count() const;
    
public slots:
    void stopDeviceDiscovery();
    void startDeviceDiscovery();
    QVariantList getDevices();
    QString getDefaultDevice();
    QBluetoothDeviceInfo *getBluetoothDevice(QString address);
    
signals:
    void discoveryStart();
    void discoveryStop(qsizetype devices);
    void devicesUpdated();
    void discoveringChanged();
    void controllerErrorChanged();
    
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
};

#endif // CAMERADISCOVERY_H
