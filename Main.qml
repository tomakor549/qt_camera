import QtQuick
import QtMultimedia
import QtQuick.Controls
import MyModule

Window {
    width: 640
    height: 480
    visible: true
    title: qsTr("Podgląd kamery")

    signal setVideoSink

    CameraManager {
        id: cameraManager
    }

    VideoOutput {
        id: videoOutput
        anchors.fill: parent
        Component.onCompleted: cameraManager.videoSink = videoOutput.videoSink
    }

    Row {
        spacing: 10
        Button {
            text: "Start"
            onClicked: cameraManager.start("/dev/video0")
        }

        Button {
            text: "Stop"
            onClicked: cameraManager.stop()
        }

        Button {
            text: "Start Recording"
            onClicked: cameraManager.startRecord("/home/arravot", "name")
        }

        Button {
            text: "Stop Recording"
            onClicked: cameraManager.stopRecord()
        }
    }
}
