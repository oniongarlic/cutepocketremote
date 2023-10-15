import QtQuick
import QtQuick.Window
import QtQuick.Controls
import QtQuick.Layouts

import Qt.labs.qmlmodels

import org.tal

ApplicationWindow {
    width: 800
    height: 480
    minimumHeight: 480
    minimumWidth: 640
    visible: true
    color: "grey"
    title: qsTr("CutePocketRemote")
    
    CameraDiscovery {
        id: disocvery
        onDiscoveryStart: {
            cameraStatus.text='Connecting...'
            cameraDrawer.open()
        }
        onDiscoveryStop: (devices) => {
                             if (devices==0) {
                                 cameraStatus.text="No cameras found!"
                                 cameraDrawer.close()
                                 return;
                             }
                             cameraStatus.text="Found "+devices
                             var d=getDevices();
                             deviceList.model=d;
                         }
    }
    
    ListModel {
        id: deviceModel
    }
    
    Drawer {
        id: cameraDrawer
        height: parent.height
        width: parent.width/2
        interactive: false
        modal: true
        closePolicy: Popup.NoAutoClose
        
        ColumnLayout {
            anchors.fill: parent
            anchors.margins: 8
            Text {
                id: deviceCount
                text: "Cameras: "+deviceList.count
            }
            ListView {
                id: deviceList
                enabled: !disocvery.discovering
                Layout.fillHeight: true
                Layout.fillWidth: true
                delegate: ItemDelegate {
                    text: modelData.name+" ("+modelData.address+")"
                    onClicked: {
                        var device=disocvery.getBluetoothDevice(modelData.address);
                        console.debug(device)
                        deviceList.currentIndex=index;
                        cd.connectDevice(device)
                        cameraDrawer.close()
                    }
                }
            }
            RowLayout {
                Button {
                    enabled: !disocvery.discovering
                    text: "Refresh"
                    onClicked: disocvery.startDeviceDiscovery()
                }
                Button {
                    enabled: !disocvery.discovering
                    text: "Close"
                    onClicked: {
                        cameraDrawer.close()
                    }
                }
            }
        }
        
        ProgressBar {
            id: discoveringProgress
            anchors.centerIn: parent
            indeterminate: visible
            visible: disocvery.discovering
        }
    }
    
    CameraDevice {
        id: cd
        
        onStatusChanged: {
            console.debug("CameraStatus is now: "+status)
            switch (status) {
            case 1:
                cameraStatus.text="Connected"
                break;
            case 3:
                cameraStatus.text="Ready"
                break;
            }
        }
        
        onIsoChanged: {
            console.debug("ISO is:"+iso)
            comboISO.currentIndex=comboISO.indexOfValue(iso)
        }
        
        onShutterSpeedChanged: {
            console.debug("shutterSpeed is:"+shutterSpeed)
            comboShutter.currentIndex=comboShutter.indexOfValue(shutterSpeed)
        }
        
        onApertureChanged: {
            console.debug("Aperture is: "+aperture)
            let tmp=cd.aperture.toFixed(1);
            comboAperture.currentIndex=comboAperture.indexOfValue(tmp)
        }
        
        onWbChanged: {
            console.debug("WB is:"+wb)
            comboWB.currentIndex=comboWB.indexOfValue(wb)
        }
        
        onTintChanged: {
            spinTint.value=tint
        }
        
        onConnectionFailure: {
            cameraStatus.text="Failed to connect"
        }
        
        property bool connectionReady: cd.connected && cd.status==3
    }
    
    Action {
        id: quitAction
        shortcut: StandardKey.Quit
        onTriggered: Qt.quit()
    }
    
    header: ToolBar {
        RowLayout {
            ToolButton {
                text: "Find"
                enabled: !cd.connected
                onClicked: disocvery.startDeviceDiscovery()
            }
            ToolButton {
                text: "&Disconnect"
                enabled: cd.connected
                onClicked: cd.disconnectFromDevice()
            }
            ToolButton {
                text: "Play"
                enabled: cd.connectionReady && !cd.recording && !cd.playing
                onClicked: cd.play(true);
                icon.name: "play"
            }
            /*
            ToolButton {
                text: "CCR"
                enabled: cd.connectionReady
                onClicked: cd.colorCorrectionReset()
            }
            */
            ToolButton {
                text: "Slate"
                enabled: cd.connectionReady
                onClicked: slateDrawer.open()
            }
        }
    }
    
    footer: ToolBar {
        RowLayout {
            anchors.fill: parent
            Label {
                id: cameraStatus
                text: ''
                font.pixelSize: 24
                Layout.alignment: Qt.AlignLeft
            }
            Label {
                id: cameraName
                text: cd.connected ? cd.name : 'N/A'
                font.pixelSize: 24
                Layout.alignment: Qt.AlignLeft
            }
            Label {
                id: zoom
                text: cd.connectionReady ? cd.zoom : '--'
                font.pixelSize: 24
            }
            Label {
                text: cd.connectionReady ? cd.iso : '---'
                font.pixelSize: 24
            }
            Label {
                text: cd.connectionReady ? '1/'+cd.shutterSpeed : '-/--'
                font.pixelSize: 24
            }
            Label {
                id: aperture
                text: cd.connectionReady ? 'f'+cd.aperture.toFixed(1) : '--'
                font.pixelSize: 24
            }
            Label {
                text: cd.connectionReady ? cd.wb+"K" : '--'
                font.pixelSize: 24
            }
            Label {
                text: cd.connectionReady ? cd.tint : '--'
                font.pixelSize: 24
            }
            TimeCodeText {
                id: timeCodeText
                Layout.preferredWidth: 12*24
                camera: cd
                Layout.alignment: Qt.AlignRight
            }
        }
    }
    
    StackView {
        anchors.fill: parent
        ColumnLayout {
            anchors.fill: parent
            anchors.margins: 4
            spacing: 8
            enabled: cd.connectionReady
            RowLayout {
                id: bc
                Layout.fillWidth: true
                Layout.fillHeight: true
                Layout.margins: 4
                Layout.alignment: Qt.AlignTop
                spacing: 8
                ColumnLayout {
                    id: buttonsContainer
                    Layout.alignment: Qt.AlignTop
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    Layout.minimumWidth: 100
                    Layout.maximumWidth: 200
                    Layout.maximumHeight: 300
                    Button {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        text: "Record"
                        icon.name: "media-record"
                        enabled: !cd.recording
                        highlighted: cd.recording
                        onClicked: cd.record(true)
                    }
                    Button {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        text: "Stop"
                        icon.name: "media-playback-stop"
                        enabled: cd.recording || cd.playing
                        onClicked: {
                            cd.record(false) // false=stop
                        }
                    }
                    Button {
                        Layout.fillWidth: true
                        text: "Capture"
                        icon.name: "camera-photo"
                        enabled: cd.connectionReady && !cd.recording && !cd.playing
                        onClicked: cd.captureStill()
                    }
                }
                TimeCodeText {
                    id: timeCodeText2
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    Layout.margins: 8
                    Layout.preferredWidth: 12*32
                    Layout.minimumWidth: 12*32 // contentWidth
                    Layout.alignment: Qt.AlignHCenter | Qt.AlignTop
                    height: buttonsContainer.height
                    minimumPixelSize: 24
                    font.pixelSize: 92
                    font.weight: Font.Bold
                    horizontalAlignment: Text.AlignHCenter
                    fontSizeMode:Text.HorizontalFit
                    color: cd.recording ? "red" : "white"
                    style: Text.Outline
                    styleColor: "black"
                    camera: cd
                    onClicked: {
                        cd.setDisplay(!cd.timecodeDisplay)
                    }
                }
                GridLayout {
                    Layout.fillWidth: true
                    Layout.alignment: Qt.AlignTop
                    Layout.minimumWidth: 160
                    Layout.minimumHeight: 120
                    Layout.maximumHeight: 300
                    rows: 3
                    columns: 2
                    Button {
                        text: "Focus-"
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        onClicked: cd.focus(-100);
                    }
                    Button {
                        text: "Focus+"
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        onClicked: cd.focus(100);
                    }
                    Button {
                        text: "Focus--"
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        onClicked: cd.focus(-500);
                    }
                    Button {
                        text: "Focus++"
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        onClicked: cd.focus(500);
                    }
                    Button {
                        text: "Auto Focus"
                        icon.name: "zoom-fit-best"
                        onClicked: cd.autoFocus()
                        Layout.fillHeight: true
                        Layout.fillWidth: true
                        Layout.minimumWidth: 100
                        Layout.columnSpan: 2
                    }
                }
                Dial {
                    id: focusDial
                    inputMode: Dial.Horizontal
                    Layout.fillHeight: true
                    Layout.fillWidth: true
                    Layout.minimumWidth: 180
                    Layout.preferredWidth: 200
                    from: -200
                    to: 200
                    stepSize: 10
                    onValueChanged: console.debug(value)
                    onPressedChanged: if (!pressed) value=0
                    wheelEnabled: true
                    Timer {
                        interval: 200
                        repeat: true
                        running: focusDial.pressed
                        onTriggered: {
                            console.debug("RelFocus: "+focusDial.value)
                            if (focusDial.value==0)
                                return;
                            cd.focus(focusDial.value);
                        }
                    }
                }
            }
            
            Label {
                text: "Shutter speed: "+ cd.shutterSpeed
            }
            RowLayout {
                Layout.fillWidth: true
                
                ComboBox {
                    id: comboShutter
                    model: [24,25,30,50,60,100,120,125,160,200,250,500,1000,2000]
                    onActivated: {
                        cd.setShutterSpeed(currentValue)
                    }
                }
                
                Slider {
                    id: shutterSpeedSlider
                    Layout.fillWidth: true
                    from: 24
                    to: 5000
                    value: 60
                    stepSize: 1
                    live: false
                    wheelEnabled: true
                    onValueChanged: {
                        cd.setShutterSpeed(value)
                    }
                }
            }
            
            Label {
                text: "Aperture"
            }
            RowLayout {
                Layout.fillWidth: true
                ComboBox {
                    id: comboAperture
                    model: [ 2.8, 2.9, 3.1, 4.0, 4.2, 5.0, 5.6, 7, 8, 10, 12, 16, 22 ]
                    onActivated: {
                        cd.setAperture(currentValue)
                    }
                }
                Slider {
                    id: apertureSlider
                    Layout.fillWidth: true
                    from: 0
                    to: 1
                    value: 0
                    stepSize: 0.05
                    live: false
                    wheelEnabled: true
                    onValueChanged: {
                        cd.setApertureNormalized(value)
                    }
                }
                Button {
                    text: "Auto\nAperture"
                    onClicked: cd.autoAperture()
                }
            }
            
            Label {
                text: "ISO: "+cd.iso
            }
            RowLayout {
                Layout.fillWidth: true
                
                ComboBox {
                    id: comboISO
                    model: [100,125,160,200,250,320,400,500,640,800,1000,1250,1600,2000,2500,3200,4000,5000,6400,8000,10000,12800,16000,20000,25600]
                    onActivated: {
                        cd.setISO(currentValue)
                    }
                }
                
                Label {
                    text: "Gain: "+gain.value
                }
                
                Slider {
                    id: gain
                    Layout.fillWidth: true
                    from: -10
                    to: 10
                    value: 0
                    stepSize: 1
                    live: false
                    wheelEnabled: true
                    onValueChanged: {
                        cd.setGain(value)
                    }
                }
            }
            
            Label {
                text: "White Balance: "+cd.wb+'K/'+cd.tint
            }
            
            RowLayout {
                spacing: 4
                ComboBox {
                    id: comboWB
                    model: [3200,3600,4000,4600,5600,6500,7500]
                    onActivated: {
                        cd.whiteBalance(currentValue, spinTint.value)
                    }
                }
                ColumnLayout {
                    Layout.fillWidth: true
                    Slider {
                        Layout.fillWidth: true
                        id: sliderWb
                        from: 2500
                        to: 10000
                        value: cd.connectionReady ? cd.wb : 4600
                        stepSize: 50
                        live: false
                        snapMode: Slider.SnapAlways
                        wheelEnabled: true
                        property bool userMoved: false
                        onMoved: userMoved=true
                        onValueChanged: {
                            if (pressed || userMoved)
                                cd.whiteBalance(value, spinTint.value)
                            userMoved=false
                        }
                    }
                    SpinBox {
                        id: spinTint
                        Layout.fillWidth: true
                        from: -50
                        to: 50
                        value: cd.connectionReady ? cd.tint : 0
                        wheelEnabled: true
                        onValueModified: {
                            cd.whiteBalance(sliderWb.value, value)
                        }
                    }
                }
                ColumnLayout {
                    Button {
                        Layout.fillWidth: true
                        text: "Auto"
                        onClicked: cd.autoWhitebalance();
                    }
                    Button {
                        Layout.fillWidth: true
                        text: "Restore"
                        onClicked: cd.restoreAutoWhiteBalance()
                    }
                }
                GridLayout {
                    rows: 2
                    columns: 3
                    Button {
                        Layout.fillWidth: true
                        text: "Sun"
                        onClicked: {
                            cd.whiteBalance(5600, 10)
                        }
                    }
                    Button {
                        Layout.fillWidth: true
                        text: "Light 1"
                        onClicked: {
                            cd.whiteBalance(3200, 0)
                        }
                    }
                    Button {
                        Layout.fillWidth: true
                        text: "Light 2"
                        onClicked: {
                            cd.whiteBalance(4000, 15)
                        }
                    }
                    Button {
                        Layout.fillWidth: true
                        text: "Shade"
                        onClicked: {
                            cd.whiteBalance(4500, 15)
                        }
                    }
                    Button {
                        Layout.fillWidth: true
                        text: "Cloudy"
                        onClicked: {
                            cd.whiteBalance(6500, 10)
                        }
                    }
                    Button {
                        Layout.fillWidth: true
                        text: "4600K"
                        onClicked: {
                            cd.whiteBalance(4600, 0)
                        }
                    }
                }
            }
        }
    }
    
    TableModel {
        id: metadataModel
        TableModelColumn { display: "metadata" }
        TableModelColumn { display: "value" }
        
        rows: [
            {
                metadata: "Camera ID",
                value: 'A'
            },
            {
                metadata: "Camera operator",
                value: 'Somebody Anonymous'
            }
        ]
    }
    
    Drawer {
        id: slateDrawer
        width: parent.width/1.5
        height: parent.height
        
        TableView {
            anchors.fill: parent
            columnSpacing: 2
            rowSpacing: 1
            model: metadataModel
            delegate: ItemDelegate {
                text: model.display
            }
        }
    }
    
}
