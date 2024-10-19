import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Shapes 1.15
import QtQml 2.15
import Dispenser 1.0

RowLayout {
    id: time

    property alias running: timer.running
    property date rel
    property date date: new Date()


    property int hour
    property int min
    property int sec
    //anchors.fill: parent

    Timer {
        id : timer
        interval: 500; running: true; repeat: true
//        onTriggered: date = new Date;
        onTriggered: { hour = Math.floor((rel - new Date)/(3600*1000)); min = ((Math.floor((rel - new Date)/(60*1000))) % 60 + 60) % 60; sec = ((Math.floor((rel - new Date)/(1000))) % 60 + 60) % 60 ; }
    }


    //Rectangle {
    //    Layout.fillWidth: true
    //    Layout.minimumWidth: 50
    //}
    Layout.alignment: Qt.AlignHCenter

    Label {
        //text: date.toLocaleTimeString(Qt.locale(), "hh")
        text: hour
    }

    Label {
        text: ":"
    }

    Label {
        //text: date.toLocaleTimeString(Qt.locale(), "mm")
        text: min
    }

    Label {
        text: ":"
    }

    Label {
        //text: date.toLocaleTimeString(Qt.locale(), "ss")
        text: sec
        //text: date.getSeconds().toString()
    }
}
