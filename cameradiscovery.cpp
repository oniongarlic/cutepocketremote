#include "cameradiscovery.h"

static const QBluetoothUuid BmdCameraService("291D567A-6D75-11E6-8B77-86F30CA893D3");

CameraDiscovery::CameraDiscovery(QObject *parent)
    : QObject{parent}
{
    m_discoveryAgent = new QBluetoothDeviceDiscoveryAgent(this);
    m_discoveryAgent->setLowEnergyDiscoveryTimeout(5000);
    
    connect(m_discoveryAgent, &QBluetoothDeviceDiscoveryAgent::deviceDiscovered, this, &CameraDiscovery::addCameraDevice);
    connect(m_discoveryAgent, &QBluetoothDeviceDiscoveryAgent::deviceUpdated, this, &CameraDiscovery::addCameraDevice);
    connect(m_discoveryAgent, &QBluetoothDeviceDiscoveryAgent::errorOccurred, this, &CameraDiscovery::deviceScanError);
    connect(m_discoveryAgent, &QBluetoothDeviceDiscoveryAgent::finished, this, &CameraDiscovery::deviceScanFinished);
    connect(m_discoveryAgent, &QBluetoothDeviceDiscoveryAgent::canceled, this, &CameraDiscovery::deviceScanFinished);

}

CameraDiscovery::~CameraDiscovery()
{    
    m_cameras.clear();
    m_devices.clear();
}

void CameraDiscovery::startDeviceDiscovery()
{    
    m_cameras.clear();
    emit devicesUpdated();
    
    if (m_discoveryAgent->isActive())
        return;
    
    m_discoveryAgent->start(QBluetoothDeviceDiscoveryAgent::LowEnergyMethod);
    
    if (m_discoveryAgent->isActive()) {
        m_discovering = true;
        emit discoveringChanged();
    }
    emit discoveryStart();
}

void CameraDiscovery::stopDeviceDiscovery()
{
    if (m_discoveryAgent->isActive())
        m_discoveryAgent->stop();
}

bool CameraDiscovery::discovering()
{
    return m_discovering;
}

void CameraDiscovery::deviceScanError(QBluetoothDeviceDiscoveryAgent::Error error)
{
    emit controllerErrorChanged();
    
    m_discovering = false;
    
    emit discoveringChanged();
}

/**
 * @brief CameraDevice::addCameraDevice
 * @param info
 *
 * Store found BLE device if it is a BM Camera, by first checking if the CameraService is available.
 * If not, ignore the BLE device.
 *
 */
void CameraDiscovery::addCameraDevice(const QBluetoothDeviceInfo &info)
{       
    // BMPCC is BLE only so ignore anything else
    if (info.coreConfigurations()!=QBluetoothDeviceInfo::LowEnergyCoreConfiguration)
        return;
    
    // Check if device has BM service 291d567a-6d75-11e6-8b77-86f30ca893d3 ?
    auto uuids=info.serviceUuids();
    if (!uuids.contains(BmdCameraService)) {
        return;
    }
    
    qDebug() << "Found BM camera service!";
    qDebug() << info.address() << info.name() << info.rssi();
    
#ifndef Q_OS_WIN32
    if (info.rssi()==0) {
        qDebug("Ignoring off-line device");
        return;
    }
#endif
    
    if (m_devices.contains(info.address().toString())) {
        qDebug("Dup!");
    } else {
        QBluetoothDeviceInfo *device=new QBluetoothDeviceInfo(info);
        QVariantMap dev;
        
        dev.insert("address", info.address().toString());
        dev.insert("name", info.name());
        
        m_cameras.append(dev);
        m_devices.insert(info.address().toString(), device);
    }
    
    emit devicesUpdated();
}

/**
 * @brief CameraDevice::deviceScanFinished
 *
 *
 */
void CameraDiscovery::deviceScanFinished()
{
    m_discovering = false;
    emit discoveringChanged();
    emit discoveryStop(m_cameras.count());
}

QString CameraDiscovery::getDefaultDevice()
{
    if (!m_cameras.isEmpty()) {
        return "";
    }
    return "";
}

QBluetoothDeviceInfo *CameraDiscovery::getBluetoothDevice(QString address)
{
    return m_devices.value(address);
}

QVariantList CameraDiscovery::getDevices()
{
    return m_cameras;
}
