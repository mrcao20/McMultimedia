import QtQuick 2.9
import QtQuick.Window 2.2
import McMultimedia 1.0

Window {
    visible: true
    width: 640
    height: 480
    title: qsTr("Hello World")
    MediaPlayer{
        id: aa
        videoRenderer: video
        source: "D:/temp/222.mp4"
    }
    VideoRenderer{
        id: video
        visible: true
        anchors.fill: parent
    }
}
