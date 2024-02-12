import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

import org.tal

ColumnLayout {
    Layout.fillWidth: true
    property CameraDevice cd;
    
    Label {
        text: "Zoom: "+cd.zoom
    }

    Dial {
        id: zoomDial
        inputMode: Dial.Circular
        Layout.fillHeight: true
        Layout.fillWidth: true
        Layout.minimumWidth: 180
        Layout.preferredWidth: 200
        Layout.alignment: Qt.AlignHCenter
        from: 0
        to: 100
        stepSize: 1
        onValueChanged: cd.zoom(focusDial.value/100);
        wheelEnabled: true
    }
}
