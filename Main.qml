import QtQuick
import QtQuick.Window
import QtQuick.Controls
import QtQuick.Layouts

import Qt.labs.qmlmodels

import org.tal

ApplicationWindow {
    id: root
    width: 800
    height: 480
    minimumHeight: 480
    minimumWidth: 800
    visible: true
    color: "#eaeaf6"
    title: qsTr("CutePocketRemote")

    palette.disabled.buttonText: "grey"

    property bool smallInterface: height>500 ? false : true
    property int smallFontSize: smallInterface ? 16 : 24
    
    property bool relativeFocus: menuFocusRelative.checked
    
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
        closePolicy: disocvery.discovering ? Popup.NoAutoClose : (Popup.CloseOnEscape | Popup.CloseOnPressOutside)
        
        ColumnLayout {
            anchors.fill: parent
            anchors.margins: 8
            Text {
                id: deviceCount
                text: "Cameras: "+disocvery.count
            }
            ListView {
                id: deviceList
                enabled: !disocvery.discovering
                Layout.fillHeight: true
                Layout.fillWidth: true
                delegate: ItemDelegate {
                    text: modelData.name+" ("+modelData.address+")"
                    font.italic: modelData.rssi==0 ? true : false
                    enabled: modelData.rssi!=0
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
                Layout.fillWidth: true
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
        
        onAutoFocusTriggered: {
            console.debug("Autofocusing...")
            setTimedMessage('AutoFocus');
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
            console.debug("Aperture is: "+cd.aperture)
            let tmp=cd.aperture.toFixed(1);
            console.log(tmp)
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
    
    menuBar: MenuBar {
        id: mainMenu
        visible: !smallInterface
        Menu {
            title: "File"
            MenuItem {
                text: "Slate"
                enabled: cd.connectionReady
                onClicked: slateDrawer.open()
            }
            MenuItem {
                text: "Quit"
                onClicked: Qt.quit()
            }
        }
        Menu {
            title: "Device"
            MenuItem {
                text: "Find"
                enabled: !cd.connected
                onClicked: disocvery.startDeviceDiscovery()
            }
            MenuItem {
                text: "&Disconnect"
                enabled: cd.connected
                onClicked: cd.disconnectFromDevice()
            }
        }
        Menu {
            title: "Lens control"
            MenuItem {
                id: menuFocusRelative
                text: "Relative Focus"
                checkable: true
                checked: true
                ButtonGroup.group: focusGroup
            }
            MenuItem {
                id: menuFocusAbsolute
                text: "Absolute Focus"
                checkable: true
                ButtonGroup.group: focusGroup
            }
            MenuSeparator {
                
            }
            MenuItem {
                id: menuZoomEnabled
                text: "Show zoom"
                checkable: true
                checked: false
            }
        }
        Menu {
            title: "Assists"
            MenuItem {
                id: menuZebraEnabled
                text: "Zebra"
                enabled: false
                checkable: true
                checked: false
            }
            MenuItem {
                id: menuPeakingEnabled
                text: "Peaking"
                enabled: false
                checkable: true
                checked: false
            }
            MenuItem {
                id: menuFalseColorEnabled
                text: "False color"
                enabled: false
                checkable: true
                checked: false
            }
        }
    }
    
    ButtonGroup {
        id: focusGroup
        onClicked: (button) => {
                       
                   }
    }
    
    header: ToolBar {
        visible: smallInterface
        RowLayout {
            ToolButton {
                text: "Connect"
                icon.name: "edit-find"
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
    
    Timer {
        id: timerMessageTimer
        interval: 2000
        triggeredOnStart: false
        repeat: false
        onTriggered: timedMessage.text=''
    }
    
    function setTimedMessage(msg) {
        timedMessage.text=msg;
        timerMessageTimer.start()
    }
    
    footer: ToolBar {
        RowLayout {
            anchors.fill: parent
            Label {
                id: cameraStatus
                text: ''
                font.pixelSize: smallFontSize
                Layout.alignment: Qt.AlignLeft
            }
            Label {
                id: cameraName
                text: cd.connected ? cd.name : 'N/A'
                font.pixelSize: smallFontSize
                Layout.alignment: Qt.AlignLeft
            }
            Label {
                id: timedMessage
                text: ''
                font.pixelSize: smallFontSize
                Layout.alignment: Qt.AlignLeft
            }
            Label {
                id: zoom
                text: cd.connectionReady ? cd.zoom : '--'
                font.pixelSize: smallFontSize
            }
            Label {
                text: cd.connectionReady ? cd.iso : '---'
                font.pixelSize: smallFontSize
            }
            Label {
                text: cd.connectionReady ? '1/'+cd.shutterSpeed : '-/--'
                font.pixelSize: smallFontSize
            }
            Label {
                id: aperture
                text: cd.connectionReady ? 'f'+cd.aperture.toFixed(1) : '--'
                font.pixelSize: smallFontSize
            }
            Label {
                text: cd.connectionReady ? cd.wb+"K" : '--'
                font.pixelSize: smallFontSize
            }
            Label {
                text: cd.connectionReady ? cd.tint : '--'
                font.pixelSize: smallFontSize
            }
            Label {
                text: cd.connectionReady ? 'Take: '+cd.metaTakeNumber : ''
            }
            
            TimeCodeText {
                id: timeCodeText
                Layout.preferredWidth: 12*24
                camera: cd
                Layout.alignment: Qt.AlignRight
                font.pixelSize: smallFontSize
            }
        }
    }
    
    StackView {
        anchors.fill: parent
        focus: true
        Keys.enabled: true
        Keys.onPressed: {
            switch (event.key) {
            case Qt.Key_F2:
                if (!cd.connected)
                    disocvery.startDeviceDiscovery()
                event.accepted=true;
                break;
            }
        }
        ColumnLayout {
            anchors.fill: parent
            anchors.margins: 4
            spacing: 4
            enabled: cd.connectionReady
            RowLayout {
                id: bc
                Layout.fillWidth: true
                Layout.fillHeight: true
                Layout.margins: 0
                Layout.alignment: Qt.AlignTop
                spacing: 4
                ColumnLayout {
                    id: buttonsContainer
                    Layout.alignment: Qt.AlignTop
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    Layout.minimumWidth: 100
                    Layout.maximumWidth: 200
                    Layout.minimumHeight: 100
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
                        Layout.fillHeight: true
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
                    Layout.margins: 4
                    Layout.preferredWidth: 12*32
                    Layout.minimumWidth: 12*18 // contentWidth
                    Layout.alignment: Qt.AlignHCenter | Qt.AlignTop
                    height: buttonsContainer.height
                    minimumPixelSize: 18
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
                ColumnLayout {
                    RelativeFocus {
                        cd: cd
                        visible: relativeFocus
                        Layout.fillWidth: true
                        Layout.alignment: Qt.AlignTop
                    }
                    AbsoluteFocus {
                        cd: cd
                        visible: !relativeFocus
                        Layout.fillWidth: true
                        Layout.alignment: Qt.AlignTop
                    }
                    Label {
                        text: cd.metaLensDistance
                    }
                    Label {
                        text: cd.metaLensFocal
                    }
                }
                ColumnLayout {
                    Zoom {
                        cd: cd
                        visible: menuZoomEnabled.checked
                        Layout.fillWidth: true
                        Layout.alignment: Qt.AlignTop
                    }
                }
            }
            
            Label {
                text: "Shutter speed: "+ cd.shutterSpeed
                visible: !smallInterface
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
                visible: !smallInterface
            }
            RowLayout {
                Layout.fillWidth: true
                ComboBox {
                    id: comboAperture
                    model: [ '2.8', '2.9', '3.1',
                        '4.0', '4.2', '4.4', '4.6', '4.8',
                        '5.0', '5.2', '5.4', '5.6', '5.9', '6.2', '6.4', '6.7',
                        '7.0', '8.0', '10.0', '11.0', '12.0',
                        '13.0', '14.0', '15.0', '16.0', '17.0', '18.0', '19.0', '20.0', '21.0', '22.0' ]
                    displayText: "f/"+currentText
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
                    Layout.fillWidth: false
                    text: "f/4.0"
                    onClicked: cd.setAperture('4.0')
                }
                Button {
                    Layout.fillWidth: false
                    text: "f/5.6"
                    onClicked: cd.setAperture('5.6')
                }
                Button {
                    Layout.fillWidth: false
                    text: "f/6.2"
                    onClicked: cd.setAperture('6.2')
                }
                Button {
                    Layout.fillWidth: false
                    text: "f/8.0"
                    onClicked: cd.setAperture('8.0')
                }
                Button {
                    id: autoApertureButton
                    Layout.fillWidth: false
                    text: "Auto\nAperture"
                    onClicked: cd.autoAperture()
                }
            }
            
            Label {
                text: "ISO: "+cd.iso
                visible: !smallInterface
            }
            RowLayout {
                Layout.fillWidth: true
                
                ComboBox {
                    id: comboISO
                    model: [100,125,160,200,250,320,400,500,640,800,1000,1250,1600,2000,2500,3200,4000,5000,6400,8000,10000,12800,16000,20000,25600]
                    displayText: "ISO "+currentText
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
                Button {
                    Layout.fillWidth: true
                    text: "400"
                    onClicked: {
                        cd.setISO(400)
                    }
                }
                Button {
                    Layout.fillWidth: true
                    text: "600"
                    onClicked: {
                        cd.setISO(600)
                    }
                }
                Button {
                    Layout.fillWidth: true
                    text: "800"
                    onClicked: {
                        cd.setISO(800)
                    }
                }
                Button {
                    Layout.fillWidth: true
                    text: "3200"
                    onClicked: {
                        cd.setISO(3200)
                    }
                }
            }
            
            Label {
                text: "White Balance: "+cd.wb+'K/'+cd.tint
                visible: !smallInterface
            }
            
            RowLayout {
                spacing: 4
                ColumnLayout {
                    ComboBox {
                        id: comboWB
                        model: [3200,3600,4000,4600,5600,6500,7500]
                        displayText: currentText+"K"
                        onActivated: {
                            cd.whiteBalance(currentValue, spinTint.value)
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
                value: cd.metaCameraID
            },
            {
                metadata: "Camera operator",
                value: cd.metaCameraOperator
            },
            {
                metadata: "Director",
                value: cd.metaDirector
            },
            {
                metadata: "Project name",
                value: cd.metaProjectName
            },
            {
                metadata: "Lens type",
                value: cd.metaLensType
            },
            {
                metadata: "Lens iris",
                value: cd.metaLensIris
            },
            {
                metadata: "Lens focal",
                value: cd.metaLensFocal
            },
            {
                metadata: "Lens distance",
                value: cd.metaLensDistance
            },
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
