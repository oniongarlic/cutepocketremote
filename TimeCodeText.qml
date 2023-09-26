import QtQuick
import QtQuick.Controls
import org.tal

Label {
    id: timeCodeText

    property CameraDevice camera;
    
    signal clicked();
    signal doubleClicked();
    
    horizontalAlignment: Text.AlignRight
    verticalAlignment: Text.AlignVCenter

    text: camera.connectionReady ? formatTimecode(camera.timecode) : '--:--:--.--'
    font.family: "Courier"
    font.bold: true
    font.pixelSize: 24
    function formatTimecode(tc) {
        let h=tc.getHours();
        let m=tc.getMinutes();
        let s=tc.getSeconds();
        let f=tc.getMilliseconds();
        let tcs=(h<10 ? '0'+h : h)+':'+(m<10 ? '0'+m : m)+':'+(s<10 ? '0'+s : s)+'.'+(f<10 ? '0'+f : f)
        return tcs;
    }
    // Layout.alignment: Qt.AlignRight
    MouseArea {
        anchors.fill: parent
        onClicked: timeCodeText.clicked()
        onDoubleClicked: timeCodeText.doubleClicked()
    }
}
