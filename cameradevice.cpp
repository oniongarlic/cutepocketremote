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

static qint16 mapf(double x, double in_min, double in_max, double out_min, double out_max)
{
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

static int16_t float2fix(double n)
{
    unsigned short int int_part = 0, frac_part = 0;
    int i;
    double t;

    int_part = (n > 0) ? (((int)floor(n)) << 11) : (((int)ceil(n)) << 11);
    n = fabs(n) - floor(fabs(n));

    t = 0.5;
    for (i = 0; i < 11; i++) {
        if ((n - t) >= 0) {
            n -= t;
            frac_part += (1 << (11 - 1 - i));
        }
        t = t /2;
    }

    return int_part + frac_part;
}

CameraDevice::CameraDevice()
{
    m_timecode.setHMS(0,0,0,0);
}

CameraDevice::~CameraDevice()
{    
    qDeleteAll(m_services);
    m_services.clear();
    if (m_controller) {
        m_controller->disconnectFromDevice();
    }
}

void CameraDevice::connectDevice(QBluetoothDeviceInfo *device)
{
    if (device->isValid())
        scanServices(*device);
}

void CameraDevice::scanServices(const QBluetoothDeviceInfo &device)
{
    qDebug() << "Scanning for BM services for " << device.name();

    if (m_controller) {
        m_controller->disconnectFromDevice();
        delete m_controller;
        m_controller = nullptr;
    }
    
    if (m_currentDevice) {
        delete m_currentDevice;
        m_currentDevice=nullptr;
    }
    m_currentDevice=new QBluetoothDeviceInfo(device);

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
    
    qDebug() << "Service:" << service->serviceUuid() << service->state();

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
    case QLowEnergyController::ConnectionError:
        emit connectionFailure();
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

bool CameraDevice::setCameraName(const QString name)
{
    return writeCameraName(name);
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
        const QList<QLowEnergyCharacteristic> chars = service->characteristics();
        qDebug() << "Service: " << service->serviceName() << service->serviceUuid() << chars.size();
        
        for (const QLowEnergyCharacteristic &ch : chars) {
            qDebug() << "QLowEnergyCharacteristic" << ch.uuid() << ch.value() << ch.value().size() << ch.value().toHex(':');
        }
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
        qDebug() << "QLowEnergyCharacteristic" << ch.uuid() << ch.value() << ch.value().size() << ch.value().toHex(':');

        QLowEnergyDescriptor desc = ch.descriptor(QBluetoothUuid::DescriptorType::ClientCharacteristicConfiguration);

        if (ch.uuid()==OutgoingCameraControl) {
            qDebug() << "Found OutgoingCameraControl!";
            m_cameraOutgoing=new QLowEnergyCharacteristic(ch);
        } else if (ch.uuid()==DeviceName) {
            qDebug() << "Found DeviceName!";
            m_cameraName=new QLowEnergyCharacteristic(ch);
        } else if (ch.uuid()==CameraStatus) {
            // XXX: Seems under Windows we get the value already here and it won't update later from a notification ?
            if (!ch.value().isNull()) {
                m_status=ch.value().at(0);
                qDebug() << "CameraStatus" << m_status;
                emit statusChanged();
            }
        } else {
            qDebug() << "Unhandled" << ch.uuid();
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
        qDebug() << "AutoFocus triggered" << data.toHex(':');
        break;
    case 2: // Aperture f-stop
    {
        uint16_t v =  CutePocket::uint16at(data, 8);

        m_aperture = sqrt(pow(2.0f, ((double)(v) / 2048.0f)));
        qDebug() << "Aperture f" << data.toHex(':') << v << m_aperture;

        emit apertureChanged();
    }
        break;
    case 3: // Aperture normalized
    {
        uint16_t v = CutePocket::uint16at(data, 8); // data[8] + (data[9] << 8);

        qDebug() << "Aperture norm" << data.toHex(':') << v;
        m_aperture_norm=v;
    }
    break;
    case 6:
        qDebug() << "OIS" << data.toHex(':');
        break;
    case 7: // Zoom
        m_zoom= CutePocket::uint16at(data, 8);
        emit zoomChanged();
        qDebug() << "Zoom" << data.toHex(':') << m_zoom;
        break;
    case 8:
    case 9:
        qDebug() << "Zoom?" << data.toHex(':');
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
    case 0: // Video mode
        qDebug() << "Mode" << data.toHex(':');
        break;
    case 2: // WB
        m_wb=CutePocket::int16at(data, 8);
        m_tint=CutePocket::int16at(data, 10);
        qDebug() << "WB" << data.toHex(':') << m_wb << m_tint;
        
        emit wbChanged();
        emit tintChanged();
        break;
    case 3:
        qDebug() << "AutoWB Triggered";
        break;
    case 4:
        qDebug() << "AutoWB Restored";
        break;
    case 5:
        m_exposure=CutePocket::int32at(data, 8);
        qDebug() << "Exposure" << data.toHex(':') << m_exposure;
        break;
    case 7:
        qDebug() << "DRM" << data.toHex(':');
        break;
    case 8:
        qDebug() << "Sharpening" << data.toHex(':');
        break;
    case 9:
        qDebug() << "Format" << data.toHex(':');
        break;
    case 10:
        qDebug() << "AutoExposureMode" << data.toHex(':');
        break;
    case 11:
        qDebug() << "Shutter Angle" << data.toHex(':');
        break;
    case 12:
        qDebug() << "Shutter speed" << data.toHex(':');
        m_shutterSpeed=CutePocket::int32at(data, 8);
        emit shutterSpeedChanged();
        break;
    case 13:
        qDebug() << "Gain" << data.toHex(':');
        break;
    case 14:
        m_iso=CutePocket::int32at(data, 8);
        qDebug() << "ISO" << data.toHex(':') << m_iso;
        emit isoChanged();
        break;
    case 15:
        qDebug() << "LUT" << data.toHex(':');
        break;
    case 16:
        qDebug() << "ND" << data.toHex(':');
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
    switch (data.at(5)) {
    case 1:
        qDebug() << "Headphone level" << data.toHex(':');
        break;
    case 2:
        qDebug() << "Headphone program mix" << data.toHex(':');
        break;
    case 3:
        qDebug() << "Input type" << data.toHex(':');
        break;
    case 4:
        qDebug() << "Input levels" << data.toHex(':');
        break;
    case 6:
        qDebug() << "Phantom power" << data.toHex(':');
        break;
    default:
        qDebug() << "Unknown audio data" << data.toHex(':') << data.toStdString();
    }
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

        m_codec=data.at(8);
        m_codec_variant=data.at(9);
        break;
    case 1: // Transport mode
        qDebug() << "Mode" << data.toHex(':');
        m_recording=data.at(8)==2 ? true : false;
        emit recordingChanged();

        m_playing=data.at(8)==1 ? true : false;
        emit playingChanged();
        
        m_media_speed=data.at(9);
        m_media_slot_1=data.at(10);
        m_media_slot_2=data.at(11);
        
        qDebug() << "Media" << m_media_speed << m_media_slot_1 << m_media_slot_2;
        break;
    case 2: // Playback
        qDebug() << "Playback" << data.toHex(':');
        break;
    case 3:
        qDebug() << "Capture" << data.toHex(':');
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
    switch (data.at(5)) {
    case 0:
        qDebug() << "Brightness" << data.toHex(':');
        break;
    case 1:
        qDebug() << "Exposure and focus tools" << data.toHex(':');
        break;
    case 2:
        qDebug() << "Zebra level" << data.toHex(':');
        break;
    case 3:
        qDebug() << "Peaking level" << data.toHex(':');
        break;
    case 4:
        qDebug() << "Color bar" << data.toHex(':');
        break;
    case 5:
        qDebug() << "Focus assist" << data.toHex(':');
        break;
    case 6:
        qDebug() << "Return feed" << data.toHex(':');
        break;
    case 7:
        qDebug() << "Time display (?)" << data.toHex(':');
        m_timecodeDisplay=data.at(8)==1 ? true : false;
        emit timecodeDisplayChanged();
        break;
    default:
        qDebug() << "handleDisplayData" << data.toHex(':');
    }
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
    switch (data.at(5)) {
    case 0:
        qDebug() << "Reference source" << data.toHex(':');
        break;
    case 1:
        qDebug() << "Reference offset" << data.toHex(':');
        break;
    default:
        qDebug() << "handleReferenceData" << data.toHex(':');
    }
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
    double r,g,b,l;
    uint16_t v;
    qDebug() << "handleColorData" << data.toHex(':');

    switch (data.at(5)) {
    case 0: // Lift
        v =  CutePocket::uint16at(data, 8);
        r = sqrt(pow(2.0f, ((double)(v) / 2048.0f)));

        v =  CutePocket::uint16at(data, 8);
        g = sqrt(pow(2.0f, ((double)(v) / 2048.0f)));

        v =  CutePocket::uint16at(data, 8);
        b = sqrt(pow(2.0f, ((double)(v) / 2048.0f)));

        v =  CutePocket::uint16at(data, 8);
        l = sqrt(pow(2.0f, ((double)(v) / 2048.0f)));

        qDebug() << "lift" << r << g << b << l;
        break;
    }
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
        
        qDebug() << "Status" << ticker << power << charge << data.toHex(':');
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
    QString str;
    qDebug() << "handleMetaData" << data.toHex(':');
    switch (data.at(5)) {
    case 0: // Reel
        break;
    case 1: // Scene tags
        break;
    case 2: // Scene
        m_meta_scene=data.mid(8);
        break;
    case 3: // Take
        m_meta_take_number=data.at(8);
        m_meta_take_tags=data.at(9);
        break;
    case 4:
        break;
    case 5: // ID
        m_meta_camera_id=data.mid(8);
        break;
    case 6: // Operator
        m_meta_camera_operator=data.mid(8);
        break;
    case 7: // Director
        m_meta_director=data.mid(8);
        break;
    case 8: // Name
        m_meta_project_name=data.mid(8);
        break;
    case 9: // Lens type
        m_meta_lens_type=data.mid(8);
        break;
    case 10: // Iris
        m_meta_lens_iris=data.mid(8);
        break;
    case 11: // Focal length
        m_meta_lens_focal=data.mid(8);
        break;
    case 12: // Distance
        m_meta_lens_distance=data.mid(8);
        break;
    case 13: // Filter
        m_meta_lens_filter=data.mid(8);
        break;
    case 14: // Slate mode
        m_meta_slate_mode=data.at(8);
        break;
    case 15: // Slate target
        m_meta_slate_target=data.mid(8);
        break;
    }    
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

bool CameraDevice::isConnected() const
{
    return m_connected;
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

bool CameraDevice::writeCameraName(const QString &name)
{
    if (!m_controller) {
        qWarning("No controller!");
        return false;
    }

    if (!m_cameraService) {
        qWarning("Camera service not available");
        return false;
    }

    if (!m_cameraName) {
        qWarning("Camera name descriptor not available");
        return false;
    }

    if (!m_cameraName->isValid())
        qWarning("Camera name descriptor is not valid ?");

    if (name.length()>32) {
        qWarning("Name too long");
        return false;
    }

    m_cameraService->writeCharacteristic(*m_cameraName, name.toLocal8Bit());

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

bool CameraDevice::focus(qint16 focus)
{
    if (focus < -2048 && focus > 2048)
        return false;
    
    // https://github.com/schoolpost/BlueMagic32/issues/5#issuecomment-731750648
    // https://forum.blackmagicdesign.com/viewtopic.php?f=12&t=139745&p=752492&hilit=relative+focus#p752492
    // https://forum.blackmagicdesign.com/viewtopic.php?f=12&t=119495&p=656983&hilit=relative+focus#p656983
    QByteArray cmd(12, 0);
    cmd[0]=0xff; // Destination
    cmd[1]=0x08; // Length
    cmd[4]=0x00; // Category
    cmd[5]=0x00; // Param
    cmd[6]=0x80;
    cmd[7]=0x01; // 0=Absolute MFT, 1=Relative EF
    cmd[8]=focus & 0xff;
    cmd[9]=(focus >> 8);
    
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

bool CameraDevice::setShutterSpeed(qint32 shutter)
{
    if (shutter < 24 && shutter > 5000)
        return false;

    QByteArray cmd(12, 0);
    cmd[0]=0xff; // Destination
    cmd[1]=0x08; // Length
    cmd[4]=0x01; // Category
    cmd[5]=0x0C; // Param
    cmd[6]=0x02;

    cmd[8]=shutter & 0xff;
    cmd[9]=(shutter >> 8);

    return writeCameraCommand(cmd);
}

bool CameraDevice::setISO(qint32 is)
{
    if (is < 100 && is > 25600)
        return false;

    QByteArray cmd(12, 0);
    cmd[0]=0xff; // Destination
    cmd[1]=0x08; // Length
    cmd[4]=0x01; // Category
    cmd[5]=0x0E; // Param
    cmd[6]=0x03;

    cmd[8]=is & 0xff;
    cmd[9]=(is >> 8);
    cmd[10]=(is >> 16);
    cmd[11]=(is >> 24);
    
    return writeCameraCommand(cmd);
}

bool CameraDevice::setAperture(double ap)
{
    double f;
    
    if (ap < 1.0 && ap > 22.0)
        return false;
    
    f=ap*ap;
    f=log2(f);
    
    qDebug() << "AP" << ap << f;
    
    quint16 m=float2fix(f);
    
    QByteArray cmd(12, 0);
    cmd[0]=0xff; // Destination
    cmd[1]=0x08; // Length
    cmd[4]=0x00; // Category
    cmd[5]=0x02; // Param
    cmd[6]=0x80;
    
    cmd[8]=m & 0xff;
    cmd[9]=(m >> 8);
    
    return writeCameraCommand(cmd);
}

bool CameraDevice::setApertureNormalized(double ap)
{
    QByteArray cmd(12, 0);
    cmd[0]=0xff; // Destination
    cmd[1]=0x08; // Length
    cmd[4]=0x00; // Category
    cmd[5]=0x03; // Param
    cmd[6]=0x02;
    
    quint16 m=float2fix(ap);
    
    cmd[8]=m & 0xff;
    cmd[9]=(m >> 8);
    
    return writeCameraCommand(cmd);
}

bool CameraDevice::setApertureStep(quint16 apstep)
{
    QByteArray cmd(12, 0);
    cmd[0]=0xff; // Destination
    cmd[1]=0x08; // Length
    cmd[4]=0x00; // Category
    cmd[5]=0x04; // Param
    cmd[6]=0x02;
    
    cmd[8]=apstep & 0xff;
    cmd[9]=(apstep >> 8);
    
    return writeCameraCommand(cmd);
}

bool CameraDevice::whiteBalance(qint16 wb, qint16 tint)
{
    if (tint < -50 && tint > 50)
        return false;

    if (wb < 2500 && wb > 10000)
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

bool CameraDevice::setGain(qint8 gain)
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

bool CameraDevice::playback(bool next) {
    QByteArray cmd(12, 0);
    cmd[0]=0xff;
    cmd[1]=0x05;
    cmd[4]=0x0A;
    cmd[5]=0x01;
    cmd[6]=0x01;
    cmd[8]=next ? 1 : 0;

    return writeCameraCommand(cmd);
}

/*
 * Time display (?) "ff:05:00:00:04:07:01:02:01"
 * Time display (?) "ff:05:00:00:04:07:01:02:00"
 * */
bool CameraDevice::setDisplay(bool tc) {
    QByteArray cmd(12, 0);
    cmd[0]=0xff;
    cmd[1]=0x05;
    cmd[4]=0x04;
    cmd[5]=0x07;
    cmd[6]=0x01;
    cmd[6]=0x02;
    cmd[8]=tc ? 1 : 0;
    
    return writeCameraCommand(cmd);
}

/**
 * @brief CameraDevice::colorControl
 * @param c
 * @param r
 * @param g
 * @param b
 * @param l
 * @return
 */
bool CameraDevice::colorControl(uint8_t c, double r, double g, double b, double l) {
    int16_t ir,ig,ib,il;

    QByteArray cmd(16, 0);
    cmd[0]=0xff;
    cmd[1]=0x0C;
    cmd[4]=0x08;
    cmd[5]=c;
    cmd[6]=0x80;
    cmd[7]=0x00;

    ir=float2fix(r);
    ig=float2fix(g);
    ib=float2fix(b);
    il=float2fix(l);

    qDebug() << ir << ig << ib << il;

    cmd[8]=ir & 0xff;
    cmd[9]=(ir >> 8);

    cmd[10]=ig & 0xff;
    cmd[11]=(ig >> 8);

    cmd[12]=ib & 0xff;
    cmd[13]=(ib >> 8);

    cmd[14]=il & 0xff;
    cmd[15]=(il >> 8);

    return writeCameraCommand(cmd);
}

bool CameraDevice::colorLift(double r, double g, double b, double l) {
    return colorControl(0, r, g, b, l);
}

bool CameraDevice::colorGamma(double r, double g, double b, double l) {
    return colorControl(1, r, g, b, l);
}

bool CameraDevice::colorGain(double r, double g, double b, double l) {
    return colorControl(2, r, g, b, l);
}

bool CameraDevice::colorOffset(double r, double g, double b, double l) {
    return colorControl(3, r, g, b, l);
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

double CameraDevice::apterture() const
{
    return m_aperture;
}

bool CameraDevice::playing() const
{
    return m_playing;
}

int CameraDevice::iso() const
{
    return m_iso;
}

int CameraDevice::shutterSpeed() const
{
    return m_shutterSpeed;
}

bool CameraDevice::timecodeDisplay() const
{
    return m_timecodeDisplay;
}
