import QtQuick 2.4
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.3
import QtQuick.Shapes 1.5
import QtQml 2.3
import Dispenser 1.0

RowLayout {
    id: qnh

    Layout.alignment: Qt.AlignHCenter

    Label {
        color: Constants.name
        text: qsTr("QNH")
    }

    Label {
        color: Constants.value
        text: Constants.qnh
    }
}
