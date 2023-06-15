import QtQuick 2.7
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.3
import Dispenser 1.0

RowLayout {
    id: main
    anchors.fill: parent
    spacing: 6


    Status {

    }

    Button {
        text: "exit fullscreen"
        onClicked: appWindow.visibility = "Windowed"
    }

    Flickable {
        width: 400; height: 400
        contentWidth: 1000; contentHeight: 400

        RowLayout {
            anchors.fill: parent

            Rectangle {
                visible: true
                color: "blue"
                Layout.fillWidth: true
                Layout.fillHeight: true
            }
            SlotButton {

            }
        }
    }
}
