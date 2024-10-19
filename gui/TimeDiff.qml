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
    property color textColor: "#ffffff"


    property string sign
    property int hour
    property int min
    property int sec
    //anchors.fill: parent

    Timer {
        id : timer
        interval: 500; running: true; repeat: true
//        onTriggered: date = new Date;
        onTriggered: {
            sign = (new Date - rel) > 0 ? " " : "-";
            hour = Math.abs((new Date - rel)/(3600*1000));
            min = Math.abs(((new Date - rel)/(60*1000)) % 60);
            sec = Math.abs(((Math.floor((new Date - rel)/(1000))) % 60));
        }
    }


    //Rectangle {
    //    Layout.fillWidth: true
    //    Layout.minimumWidth: 50
    //}
    Layout.alignment: Qt.AlignHCenter

    Label {
        color: textColor
        text: sign
    }

    Label {
        color: textColor
        //text: date.toLocaleTimeString(Qt.locale(), "hh")
        text: hour.toString().padStart(2, '0')
    }

    Label {
        color: textColor
        text: ":"
    }

    Label {
        color: textColor
        //text: date.toLocaleTimeString(Qt.locale(), "mm")
        text: min.toString().padStart(2, '0')
    }

    Label {
        color: textColor
        text: ":"
    }

    Label {
        color: textColor
        //text: date.toLocaleTimeString(Qt.locale(), "ss")
        text: sec.toString().padStart(2, '0')
        //text: date.getSeconds().toString()
    }
}
