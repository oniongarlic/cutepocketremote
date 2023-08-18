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
                text: "Ca&pture"
                enabled: cd.connectionReady && !cd.recording
                onClicked: cd.captureStill();
            }           
        }
    }

    footer: ToolBar {
        RowLayout {
            anchors.fill: parent
            Text {
                id: cameraStatus
                text: ''
                font.pixelSize: 24
            }
            Text {
                id: cameraName
                text: cd.connected ? cd.name : 'N/A'
                font.pixelSize: 24
            }
            Text {
                id: timeCodeText
                text: cd.connectionReady ? formatTimecode(cd.timecode) : '--:--:--.--'
                font.pixelSize: 24
                function formatTimecode(tc) {
                    let tcs=tc.getHours()+':'+tc.getMinutes()+':'+tc.getSeconds()+'.'+tc.getMilliseconds()
                    return tcs;
                }
            }
            Text {
                id: zoom
                text: cd.connectionReady ? cd.zoom : '--'
                font.pixelSize: 24
            }
            Text {
                text: cd.connectionReady ? cd.wb+"K" : '--'
                font.pixelSize: 24
            }
            Text {
                text: cd.connectionReady ? cd.tint : '--'
                font.pixelSize: 24
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
                Layout.minimumHeight: 200
                Layout.maximumHeight: 300
                Layout.margins: 4
                Layout.alignment: Qt.AlignTop
                spacing: 8
                RoundButton {
                    Layout.fillWidth: true
                    Layout.preferredWidth: bc.width/4
                    Layout.preferredHeight: width                    
                    text: "Record"
                    enabled: !cd.recording
                    highlighted: cd.recording
                    onClicked: cd.record(true)
                }
                RoundButton {
                    Layout.fillWidth: true
                    Layout.preferredHeight: width
                    Layout.preferredWidth: bc.width/4
                    text: "Stop"                    
                    onClicked: cd.record(false)
                }
                RoundButton {
                    Layout.fillWidth: true
                    Layout.preferredHeight: width
                    Layout.preferredWidth: bc.width/4
                    text: "Auto\nAperture"
                    onClicked: cd.autoAperture()
                }
                RoundButton {
                    Layout.fillWidth: true
                    Layout.preferredHeight: width
                    Layout.preferredWidth: bc.width/4
                    text: "Auto\nFocus"
                    onClicked: cd.autoFocus()
                }                
                ColumnLayout {
                    RoundButton {
                        text: "Focus -"
                        Layout.preferredHeight: width
                        onClicked: cd.focus(-100);
                    }
                    RoundButton {
                        text: "Focus +"
                        Layout.preferredHeight: width
                        onClicked: cd.focus(100);
                    }
                }
            }

            Slider {
                Layout.fillWidth: true                
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
                spacing: 4
                Slider {
                    id: sliderWb
                    Layout.fillWidth: true                    
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
