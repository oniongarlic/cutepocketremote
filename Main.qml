import QtQuick
import QtQuick.Window
import QtQuick.Controls
import QtQuick.Layouts

import org.tal

ApplicationWindow {
    width: 800
    height: 520
    minimumHeight: 540
    minimumWidth: 640
    visible: true
    color: "grey"
    title: qsTr("CutePocketRemote")

    CameraDevice {
        id: cd
        onDiscoveryStart: {
            cameraStatus.text='Connecting...'
        }
        onDiscoveryStop: (devices) => {
                             if (devices==0)
                             cameraStatus.text="No cameras found!"
                             else
                             cameraStatus.text="Found "+devices
                         }

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
            console.debug("ISO is"+iso)
            comboISO.currentIndex=comboISO.indexOfValue(iso)
        }
        
        onConnectionFailure: {
            cameraStatus.text="Failed to connect"
        }
        
        property bool connectionReady: cd.connected && cd.status==3
    }

    header: ToolBar {
        RowLayout {
            ToolButton {
                text: "Scan and &Connect"
                enabled: !cd.connected
                onClicked: cd.startDeviceDiscovery()
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
            }
            ToolButton {
                text: "CCR"
                enabled: cd.connectionReady
                onClicked: cd.colorCorrectionReset()
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
                id: aperture
                text: cd.connectionReady ? cd.aperture.toFixed(1) : '--'
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
                Layout.margins: 4
                Layout.alignment: Qt.AlignTop
                spacing: 8
                ColumnLayout {
                    id: buttonsContainer
                    Layout.fillWidth: true
                    Layout.minimumWidth: 100
                    Button {
                        Layout.fillWidth: true
                        text: "Record"
                        enabled: !cd.recording
                        highlighted: cd.recording
                        onClicked: cd.record(true)
                    }
                    Button {
                        Layout.fillWidth: true
                        text: "Stop"
                        enabled: cd.recording || cd.playing
                        onClicked: {
                            cd.record(false) // false=stop
                        }
                    }
                    Button {
                        Layout.fillWidth: true
                        text: "Capture"
                        enabled: cd.connectionReady && !cd.recording && !cd.playing
                        onClicked: cd.captureStill()
                    }
                }
                TimeCodeText {
                    id: timeCodeText2
                    Layout.fillWidth: true
                    Layout.margins: 8
                    height: buttonsContainer.height
                    font.pixelSize: 42
                    // Layout.preferredWidth: 12*42
                    camera: cd
                    color: cd.recording ? "red" : "white"
                    Layout.alignment: Qt.AlignHCenter
                }
                GridLayout {
                    Layout.fillWidth: true
                    Layout.alignment: Qt.AlignTop
                    Layout.minimumWidth: 200
                    rows: 3
                    columns: 2
                    Button {
                        text: "Focus-"
                        Layout.fillWidth: true
                        onClicked: cd.focus(-100);
                    }
                    Button {
                        text: "Focus+"
                        Layout.fillWidth: true
                        onClicked: cd.focus(100);
                    }
                    Button {
                        text: "Focus--"
                        Layout.fillWidth: true
                        onClicked: cd.focus(-500);
                    }
                    Button {
                        text: "Focus++"
                        Layout.fillWidth: true
                        onClicked: cd.focus(500);
                    }
                    Button {
                        text: "Auto Focus"
                        onClicked: cd.autoFocus()
                        //Layout.fillHeight: true
                        Layout.fillWidth: true
                        Layout.minimumWidth: 100
                        Layout.columnSpan: 2
                    }
                }
                Dial {
                    id: focusDial
                    inputMode: Dial.Horizontal
                    from: -200
                    to: 200
                    stepSize: 10
                    onValueChanged: console.debug(value)
                    onPressedChanged: if (!pressed) value=0
                    wheelEnabled: true
                    Timer {
                        interval: 100
                        repeat: true
                        running: focusDial.pressed
                        onTriggered: {
                            console.debug("RelFocus: "+focusDial.value)
                            cd.focus(focusDial.value);
                        }
                    }
                }
            }

            Label {
                text: "Shutter speed: "+ shutterSpeed.value
            }

            RowLayout {
                Layout.fillWidth: true

                ComboBox {
                    id: comboShutter
                    model: [24,25,30,50,60,100,120,160,250]
                    onActivated: {
                        cd.shutterSpeed(currentValue)
                    }
                }

                Slider {
                    id: shutterSpeed
                    Layout.fillWidth: true
                    from: 24
                    to: 5000
                    value: 60
                    stepSize: 1
                    live: false
                    wheelEnabled: true
                    onValueChanged: {
                        cd.shutterSpeed(value)
                    }
                }
                Button {
                    text: "Auto\nAperture"
                    onClicked: cd.autoAperture()
                }
            }

            Label {
                text: "ISO"
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
                    text: gain.value+" gain"
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
                        cd.gain(value)
                    }
                }
            }

            Label {
                text: "White Balance: "+sliderWb.value
            }
            
            RowLayout {
                spacing: 4
                ComboBox {
                    id: comboWB
                    model: [3200,3600,5600]
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
                        from: -10
                        to: 10
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
                            cd.whiteBalance(3200, 0)
                        }
                    }
                    Button {
                        Layout.fillWidth: true
                        text: "Shade"
                        onClicked: {
                            cd.whiteBalance(3200, 0)
                        }
                    }
                    Button {
                        Layout.fillWidth: true
                        text: "Cloudy"
                        onClicked: {
                            cd.whiteBalance(3200, 0)
                        }
                    }
                    Button {
                        Layout.fillWidth: true
                        text: "Inside"
                        onClicked: {
                            cd.whiteBalance(3200, 0)
                        }
                    }
                    Button {
                        Layout.fillWidth: true
                        text: "4600K"
                        onClicked: {
                            cd.whiteBalance(4600, 0)
                        }
                    }
                    Button {
                        Layout.fillWidth: true
                        text: "5600K"
                        onClicked: {
                            cd.whiteBalance(5600, 0)
                        }
                    }
                }
            }
        }
    }
    
    ProgressBar {
        id: discoveringProgress
        anchors.centerIn: parent
        indeterminate: visible
        visible: cd.disocvering && !cd.connected
    }
}
