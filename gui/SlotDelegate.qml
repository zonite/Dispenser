import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import Dispenser 1.0


/*
Item {

}
*/

Component {
    id: cellDelegate

    GreenBox {
        id: wrapper
        required property string display

        implicitHeight: 40
        implicitWidth: 40

        Text {
            anchors.centerIn: parent
            text: wrapper.display
        }
    }
}
