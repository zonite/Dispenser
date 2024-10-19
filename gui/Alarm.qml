import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Shapes 1.15
import QtQml 2.15
import Dispenser 1.0

RowLayout {
    id: alarm

    property date date: new Date

    //anchors.fill: parent

    //Rectangle {
    //    Layout.fillWidth: true
    //    Layout.minimumWidth: 50
    //}
    Layout.alignment: Qt.AlignHCenter

    Label {
        text: date.toLocaleTimeString(Qt.locale(), "hh")
    }

    Label {
        text: ":"
    }

    Label {
        text: date.toLocaleTimeString(Qt.locale(), "mm")
    }

    Label {
        text: ":"
    }

    Label {
        text: date.toLocaleTimeString(Qt.locale(), "ss")
        //text: date.getSeconds().toString()
    }
}
