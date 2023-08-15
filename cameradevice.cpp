#include "cameradevice.h"
#include "cameratypes.h"

#include <QLowEnergyCharacteristic>

// Services that BM camera should have
static const QBluetoothUuid GenericService("00001800-0000-1000-8000-00805f9b34fb");
static const QBluetoothUuid DeviceInformation("0000180a-0000-1000-8000-00805f9b34fb");
static const QBluetoothUuid BmdCameraService("291D567A-6D75-11E6-8B77-86F30CA893D3");

// Characteristics available
static const QBluetoothUuid OutgoingCameraControl("5DD3465F-1AEE-4299-8493-D2ECA2F8E1BB");
static const QBluetoothUuid IncomingCameraControl("B864E140-76A0-416A-BF30-5876504537D9");
static const QBluetoothUuid Timecode("6D8F2110-86F1-41BF-9AFB-451D87E976C8");
static const QBluetoothUuid CameraStatus("7FE8691D-95DC-4FC5-8ABD-CA74339B51B9");
static const QBluetoothUuid DeviceName("FFAC0C52-C9FB-41A0-B063-CC76282EB89C");

CameraDevice::CameraDevice()
{
    m_discoveryAgent = new QBluetoothDeviceDiscoveryAgent(this);
    m_discoveryAgent->setLowEnergyDiscoveryTimeout(5000);

    connect(m_discoveryAgent, &QBluetoothDeviceDiscoveryAgent::deviceDiscovered, this, &CameraDevice::addCameraDevice);
    connect(m_discoveryAgent, &QBluetoothDeviceDiscoveryAgent::deviceUpdated, this, &CameraDevice::addCameraDevice);
    connect(m_discoveryAgent, &QBluetoothDeviceDiscoveryAgent::errorOccurred, this, &CameraDevice::deviceScanError);
    connect(m_discoveryAgent, &QBluetoothDeviceDiscoveryAgent::finished, this, &CameraDevice::deviceScanFinished);
    connect(m_discoveryAgent, &QBluetoothDeviceDiscoveryAgent::canceled, this, &CameraDevice::deviceScanFinished);
}

CameraDevice::~CameraDevice()
{    
    qDeleteAll(m_services);
    m_cameras.clear();
    m_services.clear();
    if (m_controller) {
        m_controller->disconnectFromDevice();
    }
}

void CameraDevice::startDeviceDiscovery()
{    
    m_cameras.clear();
    emit devicesUpdated();

    m_discoveryAgent->start(QBluetoothDeviceDiscoveryAgent::LowEnergyMethod);

    if (m_discoveryAgent->isActive()) {
        m_discovering = true;
        emit discoveringChanged();
    }
    emit discoveryStart();
}

void CameraDevice::stopDeviceDiscovery()
{
    if (m_discoveryAgent->isActive())
        m_discoveryAgent->stop();
}

/**
 * @brief CameraDevice::addCameraDevice
 * @param info
 *
 * Store found BLE device if it is a BM Camera, by first checking if the CameraService is available.
 * If not, ignore the BLE device.
 *
 */
void CameraDevice::addCameraDevice(const QBluetoothDeviceInfo &info)
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
    qDebug() << info.address() << info.name() << info.coreConfigurations() << info.serviceClasses() << info.serviceUuids() << info.manufacturerIds() << info.majorDeviceClass();

    if (m_cameras.contains(info.address().toString())) {
        qDebug("Dup!");
    }

    QBluetoothDeviceInfo *device = new QBluetoothDeviceInfo(info);
    m_cameras.insert(device->address().toString(), device);

    emit devicesUpdated();
}

/**
 * @brief CameraDevice::deviceScanFinished
 *
 *
 */
void CameraDevice::deviceScanFinished()
{
    m_discovering = false;
    emit discoveringChanged();
    if (!m_cameras.isEmpty()) {
        auto tmp=m_cameras.values();
        m_currentDevice=tmp.first();
        scanServices(*m_currentDevice);
    }
    emit discoveryStop(m_cameras.count());
}

QVariant CameraDevice::getDevices()
{
    return QVariant::fromValue(m_cameras.values());
}

void CameraDevice::scanServices(const QBluetoothDeviceInfo &device)
{
    qDebug() << "Scanning for BM services for " << device.name();

    if (m_controller) {
        m_controller->disconnectFromDevice();
        delete m_controller;
        m_controller = nullptr;
    }

    if (!m_controller) {
        m_controller = QLowEnergyController::createCentral(*m_currentDevice, this);

        connect(m_controller, &QLowEnergyController::connected, this, &CameraDevice::deviceConnected);
        connect(m_controller, &QLowEnergyController::errorOccurred, this, &CameraDevice::errorReceived);
        connect(m_controller, &QLowEnergyController::disconnected, this, &CameraDevice::deviceDisconnected);
        connect(m_controller, &QLowEnergyController::serviceDiscovered, this, &CameraDevice::addLowEnergyService);
        connect(m_controller, &QLowEnergyController::discoveryFinished, this, &CameraDevice::serviceScanDone);
    }

    m_controller->setRemoteAddressType(QLowEnergyController::PublicAddress);

    qDebug() << "Connecting to " << device.name();
    m_controller->connectToDevice();
}

void CameraDevice::scanServices(const QString &address)
{
    if (m_cameras.contains(address)) {
        m_currentDevice=m_cameras.value(address);
    } else {
        qWarning() << "Device not discovered" << address;
        return;
    }

    if (!m_currentDevice->isValid()) {
        qWarning() << "Not a valid device";
        return;
    }

    scanServices(*m_currentDevice);
}

void CameraDevice::addLowEnergyService(const QBluetoothUuid &serviceUuid)
{
    qDebug() << "Service discovered" << serviceUuid;
    QLowEnergyService *service = m_controller->createServiceObject(serviceUuid);
    if (!service) {
        qWarning() << "Cannot create service for uuid";
        return;
    }

    qDebug() << "Service:" << service;

    m_services.append(service);    
}

void CameraDevice::serviceScanDone()
{
    qDebug() << "Services discovered";
    // xxx error
    if (m_services.isEmpty()) {        
        qDebug() << "No services found ?";
        return;
    }

    qDebug() << "Connecting to camera service" << BmdCameraService.toString();

    connectToService(BmdCameraService.toString());
}

void CameraDevice::connectToService(const QString &uuid)
{
    QLowEnergyService *service = nullptr;
    for (QLowEnergyService *s: std::as_const(m_services)) {
        if (s->serviceUuid().toString() == uuid) {
            service = s;
            break;
        }
    }

    if (!service)
        return;

    if (service->state() == QLowEnergyService::RemoteService) {
        connect(service, &QLowEnergyService::stateChanged, this, &CameraDevice::serviceDetailsDiscovered);
        service->discoverDetails(QLowEnergyService::FullDiscovery); //xxx QLowEnergyService::FullDiscovery / SkipValueDiscovery        
    } else if (service->state() == QLowEnergyService::RemoteServiceDiscovered) {
        serviceDetailsDiscovered(QLowEnergyService::RemoteServiceDiscovered);
    } else {
        qWarning() << "connectToService" << service->state();
    }
}

void CameraDevice::deviceConnected()
{
    qDebug() << "Connected, discovering services";

    m_connected = true;
    m_controller->discoverServices();
    emit connectedChanged();

    m_name=m_currentDevice->name();
    emit nameChanged();
}

void CameraDevice::errorReceived(QLowEnergyController::Error error)
{
    switch (error) {
    case QLowEnergyController::RemoteHostClosedError:
        deviceDisconnected();
        break;
    default:
        qDebug() << "errorReceived" << error << m_controller->errorString();    
    }
}

void CameraDevice::disconnectFromDevice()
{
    if (m_controller->state() != QLowEnergyController::UnconnectedState)
        m_controller->disconnectFromDevice();
    else
        deviceDisconnected();
}

void CameraDevice::deviceDisconnected()
{
    qWarning() << "Disconnect from device";
    if (m_cameraOutgoing) {
        delete m_cameraOutgoing;
        m_cameraOutgoing=nullptr;
    }

    m_name="";
    emit nameChanged();
    
    m_connected=false;
    emit connectedChanged();
    emit disconnected();
}

void CameraDevice::serviceDetailsDiscovered(QLowEnergyService::ServiceState newState)
{
    qDebug() << "serviceDetailsDiscovered" << newState;

    auto service = qobject_cast<QLowEnergyService *>(sender());
    if (!service) {
        qDebug() << "... invalid service?";
        return;
    }
    
    if (service->state()==QLowEnergyService::RemoteServiceDiscovering) {
        return;
    }
    
    if (service->state()==QLowEnergyService::InvalidService) {
        qDebug() << "Invalid service, disconnected from device ?";
        return;
    }
    
    const QList<QLowEnergyCharacteristic> chars = service->characteristics();
    qDebug() << "Service: " << service->serviceName() << service->serviceUuid();    

    connect(service, &QLowEnergyService::stateChanged, this, &CameraDevice::serviceStateChanged);
    connect(service, &QLowEnergyService::characteristicChanged, this, &CameraDevice::characteristicChanged);
    connect(service, &QLowEnergyService::descriptorWritten, this, &CameraDevice::confirmedDescriptorWrite);

    m_cameraService=service;

    for (const QLowEnergyCharacteristic &ch : chars) {
        qDebug() << "QLowEnergyCharacteristic" << ch.uuid() << ch.value().toHex(':');

        QLowEnergyDescriptor desc = ch.descriptor(QBluetoothUuid::DescriptorType::ClientCharacteristicConfiguration);

        if (ch.uuid()==OutgoingCameraControl) {
            qDebug() << "Found OutgoingCameraControl!";
            m_cameraOutgoing=new QLowEnergyCharacteristic(ch);
        }

        uint permission = ch.properties();
        if ((permission & QLowEnergyCharacteristic::Notify)) {
            qDebug() << "Enabling notifications for " << ch.uuid() << ch.value().toHex(':');
            service->writeDescriptor(desc, QLowEnergyCharacteristic::CCCDEnableNotification);
        } else if (permission & QLowEnergyCharacteristic::Indicate) {
            qDebug() << "Enabling indications for " << ch.uuid() << ch.value().toHex(':');
            service->writeDescriptor(desc, QLowEnergyCharacteristic::CCCDEnableIndication);
        } else if (permission & QLowEnergyCharacteristic::Write) {
            qDebug() << "WriteCharacteristics" << ch.uuid();
        }
    }
}

/**
 * @brief CameraDevice::handleLensData
 * @param data
 */
void CameraDevice::handleLensData(const QByteArray &data)
{
    switch (data.at(5)) {
    case 0: // Focus
        qDebug() << "Focus" << data.toHex(':');
        break;
    case 1:
        qDebug() << "AutoFocus triggered";
        break;
    case 2: // Aperture f-stop
    {
        uint16_t v =  CutePocket::uint16at(data, 8);

        m_aperture = sqrt(pow(2.0f, ((double)(v) / 2048.0f)));
        qDebug() << "Aperture f" << data.toHex(':') << v << m_aperture;
    }
        break;
    case 3: // Aperture normalized
    {
        uint16_t v = CutePocket::uint16at(data, 8); // data[8] + (data[9] << 8);

        qDebug() << "Aperture norm" << data.toHex(':') << v;
    }
    break;
    case 7: // Zoom
        m_zoom= CutePocket::uint16at(data, 8);
        emit zoomChanged();
        qDebug() << "Zoom" << data.toHex(':') << m_zoom;
        break;

    default:
        qDebug() << "Unknown lens data" << data.toHex(':') << data.toStdString();
    }
}

/**
 * @brief CameraDevice::handleVideoData
 * @param data
 */
void CameraDevice::handleVideoData(const QByteArray &data)
{
    switch (data.at(5)) {
    case 1: // Video mode
        qDebug() << "Mode" << data.toHex(':');
        break;
    case 2: // WB
        m_wb=data.at(8)+(data.at(9) << 8);
        m_tint=data.at(10)+(data.at(11) << 8);
        qDebug() << "WB" << data.toHex(':') << m_wb << m_tint;
        
        emit wbChanged();
        emit tintChanged();
        break;
    default:
        qDebug() << "Unknown video data" << data.toHex(':') << data.toStdString();
    }
}

/**
 * @brief CameraDevice::handleAudioData
 * @param data
 */
void CameraDevice::handleAudioData(const QByteArray &data)
{
    qDebug() << "Unknown audio data" << data.toHex(':') << data.toStdString();
}

/**
 * @brief CameraDevice::handleOutputData
 * @param data
 */
void CameraDevice::handleOutputData(const QByteArray &data)
{
    qDebug() << "Unknown output data" << data.toHex(':') << data.toStdString();
}

/**
 * @brief CameraDevice::handleMediaData
 * @param data
 */
void CameraDevice::handleMediaData(const QByteArray &data)
{
    switch (data.at(5)) {
    case 0: // Codec
        qDebug() << "Codec" << data.toHex(':');
        break;
    case 1: // Transport mode
        qDebug() << "Mode" << data.toHex(':');
        m_recording=data.at(8)==2 ? true : false;
        emit recordingChanged();
        break;
    case 2: // Playback
        qDebug() << "WB" << data.toHex(':');
        break;
    default:
        qDebug() << "Unknown video data" << data.toHex(':') << data.toStdString();
    }
}

/**
 * @brief CameraDevice::handleDisplayData
 * @param data
 */
void CameraDevice::handleDisplayData(const QByteArray &data)
{
    qDebug() << "handleDisplayData" << data.toHex(':');
}

/**
 * @brief CameraDevice::handleTallyData
 * @param data
 */
void CameraDevice::handleTallyData(const QByteArray &data)
{
    qDebug() << "handleTallyData" << data.toHex(':');
}

/**
 * @brief CameraDevice::handleReferenceData
 * @param data
 */
void CameraDevice::handleReferenceData(const QByteArray &data)
{
    qDebug() << "handleReferenceData" << data.toHex(':');
}

/**
 * @brief CameraDevice::handleConfigData
 * @param data
 */
void CameraDevice::handleConfigData(const QByteArray &data)
{
    qDebug() << "handleConfigData" << data.toHex(':');
}

/**
 * @brief CameraDevice::handleColorData
 * @param data
 */
void CameraDevice::handleColorData(const QByteArray &data)
{
    qDebug() << "handleColorData" << data.toHex(':');
}

/**
 * @brief CameraDevice::handleStatusData
 * @param data
 */
void CameraDevice::handleStatusData(const QByteArray &data)
{
    switch (data.at(5)) {
    case 0: // ?
    {
        uint8_t ticker=data.at(8);
        uint8_t charge=data.at(9);
        uint8_t power=data.at(12); // 1b=ac/psu, 0b=volt/psu, 19=volt/battery, 09=no/psu
        
        qDebug() << "Status" << ticker << power << data.toHex(':');
    }
        break;
    case 1: // USB-C attach + size ?
        break;
    case 2: // Time left?
        break;
    default:
        qDebug() << "handleStatusData" << data.toHex(':');
    }
}


/**
 * @brief CameraDevice::handleMetaData
 * @param data
 */
void CameraDevice::handleMetaData(const QByteArray &data)
{
    qDebug() << "handleMetaData" << data.toHex(':');
}

static int bcdtoint(uint8_t v) { return v-6*(v >> 4); }

void CameraDevice::characteristicChanged(QLowEnergyCharacteristic characteristic, QByteArray value)
{
    //qDebug() << "characteristicChanged" << characteristic.name() << characteristic.uuid() << value.toHex(':');
    if (characteristic.uuid()==Timecode) {
        uint8_t h,m,s,f;
        h=value.at(11);
        m=value.at(10);
        s=value.at(9);
        f=value.at(8);
        m_timecode.setHMS(bcdtoint(h),bcdtoint(m), bcdtoint(s) ,bcdtoint(f));
        emit timecodeChanged();
    } else if (characteristic.uuid()==IncomingCameraControl && (quint8)value[0]==255) {
        switch (value.at(4)) {
        case 0: // Lens
            handleLensData(value);
            break;
        case 1: // Video
            handleVideoData(value);
            break;
        case 2: // Audio
            handleAudioData(value);
            break;
        case 3: // Output
            handleOutputData(value);
            break;
        case 4: // Display
            handleDisplayData(value);
            break;
        case 5: // Tally
            handleTallyData(value);
            break;
        case 6: // Reference
            handleReferenceData(value);
            break;
        case 7: // Config
            handleConfigData(value);
            break;
        case 8: // Color correction
            handleColorData(value);
            break;
        case 9: // Undocumented, status/power related ?
            handleStatusData(value);
            break;
        case 10: // Media
            handleMediaData(value);
            break;
        case 11: // PTZ
            // handlePTZData(value);
            break;
        case 12: // Metadata
            handleMetaData(value);
            break;
        }
    } else if (characteristic.uuid()==CameraStatus) {
        m_status=value.at(0);
        qDebug() << "CameraStatus" << value.toHex(':') << m_status;
        emit statusChanged();
    }
}

void CameraDevice::serviceStateChanged(QLowEnergyService::ServiceState s)
{
    qDebug() << "serviceStateChanged" << s;
}

void CameraDevice::confirmedDescriptorWrite(const QLowEnergyDescriptor &d, const QByteArray &value)
{
    qDebug() << "confirmedDescriptorWrite" << d.name() << d.uuid() << value;
}

void CameraDevice::deviceScanError(QBluetoothDeviceDiscoveryAgent::Error error)
{
    emit controllerErrorChanged();
    
    m_discovering = false;

    emit discoveringChanged();
}

bool CameraDevice::isConnected() const
{
    return m_connected;
}

bool CameraDevice::discovering()
{
    return m_discovering;
}

bool CameraDevice::hasControllerError() const
{
    return (m_controller && m_controller->error() != QLowEnergyController::NoError);
}

bool CameraDevice::writeCameraCommand(const QByteArray &cmd)
{
    if (!m_controller) {
        qWarning("No controller!");
        return false;
    }

    //if (controller->state()!=QLowEnergyController::DiscoveredState) {
    //    qWarning() << "Not connected" << controller->state();
    //    return false;
    //}

    if (!m_cameraService) {
        qWarning("Camera service not available");
        return false;
    }

    if (!m_cameraOutgoing) {
        qWarning("Camera descriptor not available");
        return false;
    }

    if (!m_cameraOutgoing->isValid())
        qWarning("Camera descriptor is not valid ?");

    if (cmd.length()>64) {
        qWarning("Command too large");
        return false;
    }

    if (cmd.length() % 4 !=0 ) {
        qWarning("Command must be aligned by 4 bytes");
        return false;
    }

    qDebug() << "cmd" << cmd.toHex(':');

    m_cameraService->writeCharacteristic(*m_cameraOutgoing, cmd);

    return true;
}

bool CameraDevice::autoFocus()
{
    QByteArray cmd(8, 0);
    cmd[0]=0xff; // Destination
    cmd[1]=0x04; // Length
    cmd[2]=0x00; // Command ID
    cmd[3]=0x00; // Reserved = 0

    cmd[4]=0x00; // Category
    cmd[5]=0x01; // Param
    cmd[6]=0x00; // Type
    cmd[7]=0x00; // Operation

    return writeCameraCommand(cmd);
}

bool CameraDevice::autoAperture()
{
    QByteArray cmd(8, 0);
    cmd[0]=0xff; // Destination
    cmd[1]=0x04; // Length
    cmd[4]=0x00; // Category
    cmd[5]=0x05; // Param

    return writeCameraCommand(cmd);
}

bool CameraDevice::autoWhitebalance()
{
    QByteArray cmd(8, 0);
    cmd[0]=0xff; // Destination
    cmd[1]=0x04; // Length
    cmd[4]=0x01; // Category
    cmd[5]=0x03; // Param
    
    return writeCameraCommand(cmd);
}

bool CameraDevice::restoreAutoWhiteBalance()
{
    QByteArray cmd(8, 0);
    cmd[0]=0xff; // Destination
    cmd[1]=0x04; // Length
    cmd[4]=0x01; // Category
    cmd[5]=0x04; // Param
    
    return writeCameraCommand(cmd);
}

bool CameraDevice::shutterSpeed(qint32 shutter)
{
    if (shutter < 24 && shutter > 5000)
        return false;

    QByteArray cmd(12, 0);
    cmd[0]=0xff; // Destination
    cmd[1]=0x08; // Length
    cmd[4]=0x01; // Category
    cmd[5]=0x0C; // Param
    cmd[6]=0x03;

    cmd[8]=shutter & 0xff;
    cmd[9]=(shutter >> 8);

    return writeCameraCommand(cmd);
}

bool CameraDevice::whiteBalance(qint16 wb, qint16 tint)
{
    if (tint < -50 && tint > 50)
        return false;
    
    QByteArray cmd(12, 0);
    cmd[0]=0xff; // Destination
    cmd[1]=0x08; // Length
    cmd[4]=0x01; // Category
    cmd[5]=0x02; // Param
    cmd[6]=0x03;
    
    cmd[8]=wb & 0xff;
    cmd[9]=(wb >> 8);
    
    cmd[10]=tint & 0xff;
    cmd[11]=(tint >> 8);
    
    return writeCameraCommand(cmd);
}

bool CameraDevice::gain(qint8 gain)
{
    QByteArray cmd(12, 0);
    cmd[0]=0xff; // Destination
    cmd[1]=0x05; // Length
    cmd[4]=0x01; // Category
    cmd[5]=0x0D; // Param
    cmd[6]=0x01;
    cmd[8]=gain;

    return writeCameraCommand(cmd);
}

bool CameraDevice::colorCorrectionReset()
{
    QByteArray cmd(8, 0);
    cmd[0]=0xff; // Destination
    cmd[1]=0x04; // Length
    cmd[4]=0x08; // Category
    cmd[5]=0x07; // Param

    return writeCameraCommand(cmd);
}

bool CameraDevice::captureStill()
{
    QByteArray cmd(8, 0);
    cmd[0]=0xff; // Destination
    cmd[1]=0x04; // Length
    cmd[4]=0x0A; // Category
    cmd[5]=0x03; // Param

    return writeCameraCommand(cmd);
}

bool CameraDevice::record(bool record) {
    QByteArray cmd(12, 0);
    cmd[0]=0xff;
    cmd[1]=0x05;
    cmd[4]=0x0A;
    cmd[5]=0x01;
    cmd[6]=0x01;
    cmd[8]=record ? 2 : 0;

    return writeCameraCommand(cmd);
}

bool CameraDevice::play(bool play) {
    QByteArray cmd(12, 0);
    cmd[0]=0xff;
    cmd[1]=0x05;
    cmd[4]=0x0A;
    cmd[5]=0x01;
    cmd[6]=0x01;
    cmd[8]=play ? 1 : 0;

    return writeCameraCommand(cmd);
}

bool CameraDevice::recording() const
{
    return m_recording;
}

QTime CameraDevice::timecode() const
{
    return m_timecode;
}

int CameraDevice::status() const
{
    return m_status;
}

int CameraDevice::wb() const
{
    return m_wb;
}

int CameraDevice::tint() const
{
    return m_tint;
}

QString CameraDevice::name() const
{
    return m_name;
}

int CameraDevice::zoom() const
{
    return m_zoom;
}
