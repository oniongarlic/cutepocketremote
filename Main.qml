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
                enabled: cd.connectionReady && !cd.recording && !cd.playing
                onClicked: cd.captureStill();
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
                id: aperture
                text: cd.connectionReady ? cd.aperture : '--'
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
            Label {
                id: timeCodeText
                text: cd.connectionReady ? formatTimecode(cd.timecode) : '--:--:--.--'
                font.pixelSize: 24
                function formatTimecode(tc) {
                    let tcs=tc.getHours()+':'+tc.getMinutes()+':'+tc.getSeconds()+'.'+tc.getMilliseconds()
                    return tcs;
                }
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
                Layout.minimumHeight: 200
                Layout.maximumHeight: 300
                Layout.margins: 4
                Layout.alignment: Qt.AlignTop
                spacing: 8
                ColumnLayout {
                    Layout.fillWidth: true
                    Layout.preferredWidth: bc.width/5
                    Layout.maximumHeight: 300
                    Layout.alignment: Qt.AlignTop
                    RoundButton {
                        Layout.fillWidth: true
                        Layout.preferredHeight: width
                        text: "Record"
                        enabled: !cd.recording
                        highlighted: cd.recording
                        onClicked: cd.record(true)
                    }
                    RoundButton {
                        Layout.fillWidth: true
                        Layout.preferredHeight: width
                        text: "Stop"
                        enabled: cd.recording || cd.playing
                        onClicked: {
                            cd.record(false) // false=stop
                        }
                    }
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
                    Layout.fillWidth: true
                    Layout.preferredWidth: bc.width/5
                    Layout.maximumHeight: 300
                    Layout.alignment: Qt.AlignTop
                    RoundButton {
                        text: "Focus\n-"
                        Layout.fillWidth: true
                        Layout.preferredHeight: width
                        onClicked: cd.focus(-100);
                    }
                    RoundButton {
                        text: "Focus\n+"
                        Layout.fillWidth: true
                        Layout.preferredHeight: width
                        onClicked: cd.focus(100);
                    }
                }
                ColumnLayout {
                    Layout.fillWidth: true
                    Layout.preferredWidth: bc.width/5
                    Layout.maximumHeight: 300
                    Layout.alignment: Qt.AlignTop
                    RoundButton {
                        text: "Focus\n--"
                        Layout.fillWidth: true
                        Layout.preferredHeight: width
                        onClicked: cd.focus(-500);
                    }
                    RoundButton {
                        text: "Focus\n++"
                        Layout.fillWidth: true
                        Layout.preferredHeight: width
                        onClicked: cd.focus(500);
                    }
                }
            }

            RowLayout {
                Layout.fillWidth: true
                Text {
                    text: shutterSpeed.value+" ss"
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
            }

            RowLayout {
                Layout.fillWidth: true

                ComboBox {
                    id: iso
                    model: [100,125,160,200,250,320,400,500,640,800,1000,1250,1600,2000,2500,3200,4000,5000,6400,8000,10000,12800,16000,20000,25600]
                    onActivated: {
                        cd.iso(currentValue)
                    }
                }

                Text {
                    text: gain.value+" gain"
                }

                Slider {
                    id: gain
                    Layout.fillWidth: true
                    from: -127
                    to: 127
                    value: 0
                    stepSize: 1
                    live: false
                    wheelEnabled: true
                    onValueChanged: {
                        cd.gain(value)
                    }
                }
            }
            
            RowLayout {
                spacing: 4
                Text {
                    text: "WB "+sliderWb.value
                }
                Slider {
                    id: sliderWb
                    Layout.fillWidth: true
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
                    from: -10
                    to: 10
                    value: cd.connectionReady ? cd.tint : 0
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
