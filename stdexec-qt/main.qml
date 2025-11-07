import QtQuick
import QtQuick.Controls

Window {
    width: 640
    height: 480
    visible: true
    title: qsTr("Hello World")
    Column {
        Button {
            text: "Run some tasks!"
            onClicked: httpDownloader.fetchFile();
        }
        Button {
            text: "Run some coros!"
            onClicked: httpDownloader.fetchFileWithCoro();
        }
        Button {
            text: "Run some multi-arg coros!"
            onClicked: httpDownloader.fetchFileWithTupleCoro();
        }
        Button {
            text: "Run some stackful coros!"
            onClicked: httpDownloader.fetchFileWithStackfulCoro();
        }
        Button {
            text: "Run just a HEAD request with a coro!"
            onClicked: httpDownloader.fetchHeadWithCoro();
        }
        Button {
            text: "Run another stackful coro test!"
            onClicked: httpDownloader.runStackfulCoro();
        }

        ProgressBar {
            id: progressBar
        }
        Text {
            text: Math.floor(progressBar.value * 100) + "%"
        }
        Text {
            text: "Content length of our request is " + httpDownloader.contentLength
        }

        Connections {
            target: httpDownloader
            function onDownloadProgress(progress) {
                progressBar.value = progress
            }
        }

    }
}
