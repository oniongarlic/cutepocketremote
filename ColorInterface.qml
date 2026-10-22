import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

ColumnLayout {
    id: cir

    property double ccFrom: 0
    property double ccTo: 2

    property double r: cr.value
    property double g: cg.value
    property double b: cb.value
    property double level: cl.value

    property vector4d rgb: Qt.vector4d(r, g, b, level)

    property double defaultValue: 0

    property alias text: ccl.text

    onDefaultValueChanged: {
        set(defaultValue, defaultValue, defaultValue, defaultValue);
    }

    /**
     * Set specific limits for current CC type
    */
    function setLimits(f,t,d) {
        limits(f, t)
        defaultValue=d;
    }

    function limits(f,t) {
        ccFrom=f;
        ccTo=t;
    }

    function set(r,g,b,l) {
        cr.value=r;
        cg.value=g;
        cb.value=b;
        cl.value=l;
    }

    Label {
        id: ccl
        Layout.fillWidth: true
        font.bold: true
    }

    Button {
        text: "Reset"
        Layout.fillWidth: true
        onClicked: {
            set(defaultValue, defaultValue, defaultValue, defaultValue);
        }
    }

    RowLayout {
        Layout.fillHeight: true
        Layout.fillWidth: true
        uniformCellSizes: true
        spacing: 16
        ColorSlider {
            id: cr
            from: ccFrom
            to: ccTo
            value: 0
        }
        ColorSlider {
            id: cg
            from: ccFrom
            to: ccTo
            value: 0
        }
        ColorSlider {
            id: cb
            from: ccFrom
            to: ccTo
            value: 0
        }
        ColorSlider {
            id: cl
            from: ccFrom
            to: ccTo
            value: 0
        }
    }
}
