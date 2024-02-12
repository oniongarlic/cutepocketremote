import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

import org.tal

RowLayout {

    property CameraDevice cd;

    GridLayout {
        id: gl

        Layout.fillWidth: true
        Layout.alignment: Qt.AlignTop
        Layout.minimumWidth: 130
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
            running: focusDial.pressed || focusDial.value!=0
            onTriggered: {
                console.debug("RelFocus: "+focusDial.value)
                if (focusDial.value==0)
                    return;
                cd.focus(focusDial.value);
            }
        }
    }
}
