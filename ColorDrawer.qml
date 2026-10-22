import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

import org.tal;

Drawer {
    id: ccud
    height: parent.height
    width: parent.width/1.3

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 8
        Button {
            text: "Reset Color Correction"
            Layout.fillWidth: true
            onClicked: {
                cd.colorCorrectionReset();
            }
        }

        RowLayout {
            spacing: 32
            ColorInterface {
                id: ccLift
                ccFrom: -2
                ccTo: 2
                defaultValue: 0
                text: "Lift"
                onRgbChanged: {
                    cd.colorLift(r, g, b, level)
                }
            }
            ColorInterface {
                id: ccGamma
                ccFrom: -4
                ccTo: 4
                defaultValue: 0
                text: "Gamma"
                onRgbChanged: {
                    cd.colorGamma(r, g, b, level)
                }
            }
            ColorInterface {
                id: ccGain
                ccFrom: 0
                ccTo: 16
                defaultValue: 1
                text: "Gain"
                onRgbChanged: {
                    cd.colorGain(r, g, b, level)
                }
            }
        }
    }
}