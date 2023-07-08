import QtQuick 2.7
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.3
import Dispenser 1.0
import eu.nykyri.dispenser 1.0

RowLayout {
    id: main
    anchors.fill: parent
    spacing: 6


    Status {

    }

    Flickable {
        //anchors.fill: parent
        //width: 400; height: 400
        width: appWindow.width
        height: appWindow.height
        contentWidth: 1000; contentHeight: 400
        clip: true

        Manager {
            id: manager
        }


        RowLayout {
            anchors.fill: parent

            Button {
                text: qsTr("New Connection")
                onClicked: appWindow.visibility = "Windowed"
            }
            /*
            Rectangle {
                visible: true
                color: "blue"
                Layout.fillWidth: true
                Layout.fillHeight: true
            }
            SlotButton {

            }
            */
        }
    }
}
