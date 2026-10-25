import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

ColumnLayout {
    Layout.fillWidth: true

    property alias from: c.from
    property alias to: c.to
    property alias value: c.value

    property alias backgroundColor: bg.color
    property alias color: l.color

    property double defaultValue;

    signal reset;

    Component.onCompleted: defaultValue=value;

    Label {
        id: l
        Layout.alignment: Qt.AlignHCenter
        padding: 4
        Layout.fillWidth: true
        horizontalAlignment: Qt.AlignHCenter
        text: c.value.toFixed(2)
        background: Rectangle {
            id: bg
            color: "transparent"
        }
        MouseArea {
            anchors.fill: parent
            onDoubleClicked: value=defaultValue
        }
    }

    Button {
        text: "+"
        Layout.alignment: Qt.AlignHCenter
        Layout.maximumWidth: c.width*2
        onClicked: c.increase()
    }

    Slider {
        id: c
        Layout.alignment: Qt.AlignHCenter
        Layout.fillHeight: true
        orientation: Qt.Vertical
        from: 0
        to: 2
        stepSize: 0.01
        value: 0
        wheelEnabled: true
        live: false
    }

    Button {
        text: "-"
        Layout.maximumWidth: c.width*2
        Layout.alignment: Qt.AlignHCenter
        onClicked: c.decrease()
    }

}
