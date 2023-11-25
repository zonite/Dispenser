import QtQuick 2.4
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.3
import QtQuick.Shapes 1.5
import Dispenser 1.0

Frame {
    width: Constants.leftSideBarWidth
    //Layout.fillWidth: true
    Layout.fillHeight: true
    //color: "#202020"

    /*
    background: Rectangle {
        anchors.fill: parent
        color: "black"
        z: -2
    }
    */

    //border.color: Constants.borderColor
    //border.style: ridge
    //border.width: Constants.borderW


    ColumnLayout {
        anchors.fill: parent
        anchors.topMargin: 5
        //Layout.alignment: Qt.AlignHCenter

        Label {
            text: qsTr("Dispenser")
            //Layout.fillWidth: true
            Layout.alignment: Qt.AlignHCenter
            font.pixelSize: 24
        }

        Time {
            Layout.fillWidth: true
            //anchors.centerIn: parent
            //Layout.alignment: Qt.AlignHCenter
        }


        Temp {
            Layout.fillWidth: true
        }

        QNH {
            Layout.fillWidth: true
        }

        DewP {
            Layout.fillWidth: true
        }

        Messages {
            Layout.fillWidth: true
        }

        Button {
            text: qsTr("Add Connection")
            Layout.fillWidth: true
            onClicked: unitList.appendItem()
        }
        Button {
            text: qsTr("Remove")
            Layout.fillWidth: true
            onClicked: unitList.removeCompletedItems()
        }
    }
}
