import QtQuick
import QtQuick.Window
import QtQuick.Controls
import QtQuick.Layouts

import org.tal

ApplicationWindow {
    width: 640
    height: 480
    visible: true
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
        
        onConnectionFailure: {
            cameraStatus.text="Failed to connect"
        }
        
        property bool connectionReady: cd.connected && cd.status==3
    }

    header: ToolBar {
        RowLayout {
            ToolButton {
                text: "Scan & Connect"
                enabled: !cd.connected
                onClicked: cd.startDeviceDiscovery()
            }
            ToolButton {
                text: "Disconnect"
                enabled: cd.connected
                onClicked: cd.disconnectFromDevice()
            }
            ToolButton {
                text: "AF"
                enabled: cd.connected
                onClicked: cd.autoFocus()
            }
            ToolButton {
                text: "Rec"
                enabled: cd.connected && !cd.recording
                onClicked: cd.record(true)
            }
            ToolButton {
                text: "Stop"
                enabled: cd.connected && cd.recording
                onClicked: cd.record(false)
            }
            ToolButton {
                text: "Capture"
                enabled: cd.connected && !cd.recording
                onClicked: cd.captureStill();
            }
            ToolButton {
                text: "Focus -"
                enabled: cd.connected
                onClicked: cd.focus(-100);
            }
            ToolButton {
                text: "Focus +"
                enabled: cd.connected
                onClicked: cd.focus(100);
            }
        }
    }

    footer: ToolBar {
        RowLayout {
            Text {
                id: cameraStatus
                text: ''
            }
            Text {
                id: cameraName
                text: cd.connected ? cd.name : 'N/A'
            }
            Text {
                id: timeCodeText
                text: cd.connected ? cd.timecode : '--:--:--:--'
            }
            Text {
                id: zoom
                text: cd.connected ? cd.zoom : '--'
            }
            Text {
                text: cd.connected ? cd.wb+"K" : '--'
            }
            Text {
                text: cd.connected ? cd.tint : '--'
            }
        }

    }

    StackView {
        anchors.fill: parent
        ColumnLayout {
            anchors.fill: parent
            RowLayout {
                Layout.fillWidth: true
                Layout.minimumHeight: 200
                Layout.maximumHeight: 300
                spacing: 16
                RoundButton {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    text: "Record"
                    enabled: cd.connected && !cd.recording
                    onClicked: cd.record(true)
                }
                RoundButton {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    text: "Stop"
                    enabled: cd.connected && cd.recording
                    onClicked: cd.record(false)
                }
                RoundButton {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    text: "AutoFocus"
                    enabled: cd.connected
                    onClicked: cd.autoFocus()
                }
                RoundButton {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    text: "AutoAperture"
                    enabled: cd.connected
                    onClicked: cd.autoAperture()
                }
            }

            Slider {
                Layout.fillWidth: true
                enabled: cd.connected
                from: 24
                to: 5000
                value: 30
                stepSize: 1
                live: false
                wheelEnabled: true
                onValueChanged: {
                    cd.shutterSpeed(value)
                }
            }
            Slider {
                Layout.fillWidth: true
                enabled: cd.connected
                from: -128
                to: 127
                value: 0
                stepSize: 1
                live: false
                wheelEnabled: true
                onValueChanged: {
                    cd.gain(value)
                }
            }
            
            RowLayout {
                Slider {
                    id: sliderWb
                    Layout.fillWidth: true
                    enabled: cd.connected
                    from: 2500
                    to: 10000
                    value: 4600
                    stepSize: 100
                    live: false
                    snapMode: Slider.SnapAlways
                    wheelEnabled: true
                    onValueChanged: {
                        cd.whiteBalance(value, spinTint.value)
                    }
                }
                SpinBox {
                    id: spinTint
                    from: -10
                    to: 10
                    value: 0
                    onValueModified: {
                        cd.whiteBalance(sliderWb.value, value)
                    }
                }
                Button {
                    text: "Auto"
                    onClicked: cd.autoWhitebalance();
                }
                Button {
                    text: "Restore"
                    onClicked: cd.restoreAutoWhiteBalance()
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
