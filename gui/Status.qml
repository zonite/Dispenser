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

    required property SlotModel slot
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

        Label {
            text: qsTr("Dispenser")
            //Layout.fillWidth: true
            Layout.alignment: Qt.AlignHCenter
            font.pixelSize: 24
        }

        RowLayout {
            //anchors.fill: parent
            spacing: Constants.spacing


            ColumnLayout {
                //anchors.fill: parent
                anchors.topMargin: 5
                //Layout.alignment: Qt.AlignHCenter


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

                /*
                Messages {
                    Layout.fillWidth: true
                }
*/
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

            Rectangle {
                Layout.fillHeight: true
                Layout.fillWidth: true
                //Layout.minimumWidth: childrenRect.width
                Layout.minimumWidth: door.contentWidth + 2 * Constants.margins
                //Layout.preferredWidth: parent.width * 0.3
                //implicitHeight: root.height
                //implicitWidth: 10
                //implicitWidth: childrenRect.width
                border.width: Constants.borderW
                border.color: Constants.borderColor
                color: Constants.backgroundColor

                ColumnLayout {
                    id: col
                    anchors.fill: parent
                    anchors.margins: Constants.margins

                    Label {
                        id: door
                        text: "DOOR SERV FWD OPEN"
                        color: Constants.warning
                        visible: slotModel.door
                    }

                    Label {
                        text: "BATT 1 DISCHARGING"
                        color: Constants.caution
                        visible: slotModel.charging
                    }

                    Label {
                        text: "DATALINK 1 FAIL"
                        color: Constants.advisory
                        visible: false
                    }

                    Label {
                        text: "LIGHT ON"
                        color: Constants.advisory
                        visible: slotModel.light
                    }

                    Label {
                        text: "NIGHT MODE"
                        color: Constants.advisory
                        visible: slotModel.night
                    }

                    Label {
                        text: "TEST"
                        color: Constants.advisory
                        visible: false
                    }

                    Label {
                        text: "END"
                        //Layout.fillWidth: true
                        Layout.alignment: Qt.AlignHCenter
                        //anchors.horizontalCenter: parent.horizontalCenter
                        color: Constants.info
                    }

                    Rectangle {
                        Layout.fillHeight: true
                    }
                }
            }
        }

        Image {
            id: varsa
            source: "qrc:///varsa.png"
        }

    }
}
