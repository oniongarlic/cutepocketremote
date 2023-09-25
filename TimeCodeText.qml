import QtQuick
import QtQuick.Controls
import org.tal

Label {
    id: timeCodeText

    property CameraDevice camera;

    text: camera.connectionReady ? formatTimecode(camera.timecode) : '--:--:--.--'
    font.pixelSize: 24
    function formatTimecode(tc) {
        let tcs=tc.getHours()+':'+tc.getMinutes()+':'+tc.getSeconds()+'.'+tc.getMilliseconds()
        return tcs;
    }
    // Layout.alignment: Qt.AlignRight
}
