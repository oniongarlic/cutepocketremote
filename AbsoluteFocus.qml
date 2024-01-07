import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

import org.tal

ColumnLayout {
    Layout.fillWidth: true
    property CameraDevice cd;

    Button {
        text: "Auto Focus"
        icon.name: "zoom-fit-best"
        onClicked: cd.autoFocus()
        // Layout.fillHeight: true
        Layout.fillWidth: true
        Layout.minimumWidth: 180
        Layout.columnSpan: 2
    }
    Dial {
        id: focusDial
        inputMode: Dial.Circular
        Layout.fillHeight: true
        Layout.fillWidth: true
        Layout.minimumWidth: 180
        Layout.preferredWidth: 200
        Layout.alignment: Qt.AlignHCenter
        from: -2000
        to: 2000
        stepSize: 10
        onValueChanged: cd.focus(focusDial.value, false);
        onPressedChanged: if (!pressed) value=0
        wheelEnabled: true
    }
}
