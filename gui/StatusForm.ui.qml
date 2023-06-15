import QtQuick 2.4
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.3
import QtQuick.Shapes 1.5
import Dispenser 1.0

Rectangle {
    width: Constants.leftSideBarWidth
    //Layout.fillWidth: true
    Layout.fillHeight: true
    color: "#202020"

    border.color: Constants.borderColor
    //border.style: ridge
    border.width: Constants.borderW

    ColumnLayout {
        anchors.fill: parent

        Time {
            //anchors.centerIn: parent
            //Layout.alignment: Qt.AlignHCenter
        }

        Temp {

        }

        QNH {

        }

        DewP {

        }

        Messages {

        }
    }
}
