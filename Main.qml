import QtQuick
import QtQuick.Window
import QtQuick.Controls
import QtQuick.Layouts

import org.tal

ApplicationWindow {
    width: 640
    height: 480
    visible: true
    title: qsTr("Hello World")

    CameraDevice {
        id: cd
        onDiscoveryStart: {
            
        }
        onDiscoveryStop: {
            if (devices==0)
                cameraName.text="No cameras found!"
            eles
                cameraName.text="Found "+devices
        }

        onStatusChanged: {
            console.debug("CameraStatus is now: "+status)
        }
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
        }
    }

    footer: ToolBar {
        RowLayout {
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
        }

    }

    StackView {
        anchors.fill: parent
        ColumnLayout {
            anchors.fill: parent
            RowLayout {
                Layout.fillWidth: true
                Layout.fillHeight: true
                RoundButton {
                    //Layout.fillWidth: true
                    text: "Record"
                    width: 64
                    height: width
                    enabled: cd.connected && !cd.recording
                    onClicked: cd.record(true)
                }
                RoundButton {
                    //Layout.fillWidth: true
                    width: 64
                    height: width
                    text: "Stop"
                    enabled: cd.connected && cd.recording
                    onClicked: cd.record(false)
                }
                RoundButton {
                    //Layout.fillWidth: true
                    width: 64
                    height: width
                    text: "Focus"
                    enabled: cd.connected
                    onClicked: cd.autoFocus()
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
                onValueChanged: {
                    cd.gain(value)
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
