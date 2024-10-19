/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the Qt Quick Controls module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or (at your option) the GNU General
** Public license version 3 or any later version approved by the KDE Free
** Qt Foundation. The licenses are as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html and
** https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

import QtQuick 2.4
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.3
import QtQuick.Shapes 1.5
import Dispenser 1.0

/*!
    \qmltype Button
    \inqmlmodule QtQuick.Controls
    \since 5.1
    \ingroup controls
    \brief A push button with a text label.

    \image button.png

    The push button is perhaps the most commonly used widget in any graphical
    user interface. Pushing (or clicking) a button commands the computer to
    perform some action or answer a question. Common examples of buttons are
    OK, Apply, Cancel, Close, Yes, No, and Help buttons.

    \qml
    Button {
        text: "Button"
    }
    \endqml

    Button is similar to the QPushButton widget.

    You can create a custom appearance for a Button by
    assigning a \l {ButtonStyle}.
 */


Rectangle {
    id: root
    implicitWidth: Constants.buttonW
    implicitHeight: Constants.buttonH
    //width: Constants.buttonW
    //height: Constants.buttonH
    color: "transparent"

    signal clicked

    property date alarm
    property int duration: 250
    property real h: 3
    property real w: 3
    //property alias text: label.text

    MouseArea {
        anchors.fill: parent
        onClicked: root.clicked()
        onPressed: {
            //up.visible = true
            //down.visible = false
            //transit.visible = false
            //animation1.start()
            //animation2.start()
        }
    }

    //ColumnLayout {
    //anchors.centerIn: parent
    Rectangle {
        id: up
        visible: false
        implicitWidth: Constants.buttonW
        implicitHeight: Constants.buttonH

        color: Constants.backgroundColor
        border.color: Constants.upColor
        border.width: Constants.borderW

        Label {
            id: labelUP
            //x: 292
            //y: 252
            text: qsTr("UP")
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.verticalCenter: parent.verticalCenter
            color: Constants.upColor
            font.family: Constants.fontFamily
            font.pixelSize: Constants.buttonH / 2
        }
    }

    Rectangle {
        id: down
        visible: false
        implicitWidth: Constants.buttonH
        implicitHeight: Constants.buttonH
        anchors.horizontalCenter: parent.horizontalCenter

        radius: Constants.buttonH / 2
        color: Constants.backgroundColor
        border.color: Constants.downColor
        border.width: Constants.borderW

        Label {
            id: labelDN
            //x: 292
            //y: 252
            text: qsTr("DN")
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.verticalCenter: parent.verticalCenter
            color: Constants.downColor
            font.family: Constants.fontFamily
            font.pixelSize: Constants.buttonH / 3
        }
    }


    Shape {
        id: transit
        visible: false
        ShapePath {
            //strokeWidth: 1
            strokeWidth: Constants.borderW
            strokeColor: "orange"
            fillColor: "transparent"

            /*
        fillGradient: LinearGradient {
            x1: 20; y1: 20
            x2: 180; y2: 130
            GradientStop { position: 0; color: "blue" }
            GradientStop { position: 0.2; color: "green" }
            GradientStop { position: 0.4; color: "red" }
            GradientStop { position: 0.6; color: "yellow" }
            GradientStop { position: 1; color: "cyan" }
        }
        */
            //strokeStyle: ShapePath.DashLine
            //dashPattern: [ 1, 4 ]
            startX: 20; startY: 0
            PathLine { x: 170; y: 150 }
            PathLine { x: 170; y: 110 }
            PathLine { x: 60; y: 0 }
            PathLine { x: 100; y: 0 }
            PathLine { x: 170; y: 70 }
            PathLine { x: 170; y: 30 }
            PathLine { x: 140; y: 0 }
            PathLine { x: 170; y: 0 }
            PathLine { x: 170; y: 150 }
            PathLine { x: 130; y: 150 }
            PathLine { x: 0; y: 20 }
            PathLine { x: 0; y: 60 }
            PathLine { x: 90; y: 150 }
            PathLine { x: 50; y: 150 }
            PathLine { x: 0; y: 100 }
            PathLine { x: 0; y: 140 }
            PathLine { x: 10; y: 150 }
            PathLine { x: 0; y: 150 }
            PathLine { x: 0; y: 0 }
            PathLine { x: 170; y: 0 }
            PathLine { x: 170; y: 150 }
            PathLine { x: 10; y: 150 }
        }
    }

    ColumnLayout {
        //fillWidth: true
        anchors.horizontalCenter: parent.horizontalCenter
        //preferredHeight: Constants.buttonH
        Layout.preferredHeight: Constants.buttonH
        Layout.alignment: Qt.AlignBottom

        Rectangle {
            implicitHeight: Constants.buttonH / 3 * 2
        }

        Alarm {
            y: parent.verticalCenter + 200
            //anchors.horizontalCenter: parent.horizontalCenter
            //anchors.top: transit.bottom
            date: root.alarm
            Layout.fillHeight: true
        }
        TimeDiff {
            rel: root.alarm
        }
    }

    states: [
        State {
            name: "unknown"
            //PropertyChanges { target: transit; visible: true }
        },
        State {
            name: "up"
            PropertyChanges { target: up; visible: true }
        },
        State {
            name: "down"
            PropertyChanges { target: down; visible: true }
        },
        State {
            name: "transit"
            PropertyChanges { target: transit; visible: true }
        }
    ]
}

/*
Rectangle {
    id: button

    signal clicked

    property int duration: 250
    //property alias text: label.text

    MouseArea {
        anchors.fill: parent
        onClicked: root.clicked()
        onPressed: {
            //glow.visible = true
            //animation1.start()
            //animation2.start()
        }
    }


    Rectangle {
        implicitHeight: 150
        implicitWidth: 150
        color: "black"
        border.color: "orange"
        //radius: 4
    }


    Canvas {
        id: drawingCanvas
        implicitHeight: 200
        implicitWidth: 200
        anchors.fill: parent
        onPaint: {
            var ctx = getContext("2d")

            ctx.fillStyle = "white"
            ctx.fillRect(0,0,drawingCanvas.width ,drawingCanvas.height )

            /*
            ctx.lineWidth = 15;
            ctx.strokeStyle = "red"
            ctx.beginPath()
            ctx.moveTo(drawingCanvas.width / 2, 0)
            ctx.lineTo((drawingCanvas.width / 2) + 10, 0)
            //ctx.closePath()
            ctx.stroke()

        }
    }

    /*
    Path {
        stroke: 1
        startX: 0; startY: 100
        PathLine { x: 150; y: 0 }
    }

}
*/
