/****************************************************************************
**
** Copyright (C) 2017 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** BSD License Usage
** Alternatively, you may use this file under the terms of the BSD license
** as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

import QtQuick 2.15
import QtQuick.Controls 2.15
//import QtQuick 2.0 as OldQuick
//import QtQuick.Controls 1.4 as OldControls
//import QtQuick.Layouts 1.3 as OldLayout
import QtQuick.Layouts 1.15
import Dispenser 1.0

ApplicationWindow {
    id: appWindow
    visible: true
    visibility: "FullScreen"
    width: Constants.width
    height: Constants.height
    title: qsTr("Dispenser")

    readonly property bool inPortrait: appWindow.width < appWindow.height

    /** no toolbar
    ToolBar {
        id: overlayHeader

        z: 1
        width: parent.width
        //parent: Overlay.overlay
        parent: appWindow.overlay

        Label {
            id: label
            anchors.centerIn: parent
            text: qsTr("Dispenser")
        }
    }
    */


    Drawer {
        id: drawer

        //y: overlayHeader.height
        width: appWindow.width / 4
        height: appWindow.height //no ToolBar
        //height: appWindow.height - overlayHeader.height

        modal: inPortrait
        //interactive: true
        interactive: inPortrait
        position: inPortrait ? 0 : 1
        visible: !inPortrait

        Status {
            //implicitHeight: parent.height
            anchors.fill: parent
        }



        /*
        ListView {
            id: listView
            anchors.fill: parent
            clip: true

            headerPositioning: ListView.OverlayHeader
            header: Pane {
                id: header
                z: 2
                width: parent.width

                contentHeight: logo.height

                /*
                Image {
                    id: logo
                    width: parent.width
                    source: "images/qt-logo.png"
                    fillMode: implicitWidth > width ? Image.PreserveAspectFit : Image.Pad
                }
                /

                Text {
                    id: logo
                    text: qsTr("QT")
                }
/*
                MenuSeparator {
                    parent: header
                    width: parent.width
                    anchors.verticalCenter: parent.bottom
                    visible: !listView.atYBeginning
                }
                /
            }

            footer: ItemDelegate {
                id: footer
                text: qsTr("Footer")
                width: parent.width

                /*
                MenuSeparator {
                    parent: footer
                    width: parent.width
                    anchors.verticalCenter: parent.top
                }
                /
            }

            model: 10

            delegate: ItemDelegate {
                text: qsTr("Title %1").arg(index + 1)
                width: listView.width
            }

            ScrollIndicator.vertical: ScrollIndicator { }
        }
        */
    }

    //RowLayout {
        //height: parent.height

//        Status {
//            id: statusCol

//        }



    Unit {
        anchors.leftMargin: !inPortrait ? drawer.width : undefined
        moodeli: unitItem
        //unit: unitItem
    }

    /*
    ListView {
            id: view
            //Layout.fillWidth: true
            //Layout.fillHeight: true
            //implicitWidth: appWindow.width
            //anchors.left: statusCol.right
            //x: drawer.width
            //implicitHeight: appWindow.height
            //implicitWidth: appWindow.width - drawer.width
            anchors.fill: parent
            anchors.leftMargin: !inPortrait ? drawer.width : undefined
            topMargin: 5
            bottomMargin: 5
            //implicitHeight: 250
            clip: true
            orientation: Qt.Horizontal

            // Header as Drawer
            //headerPositioning: ListView.PullBackHeader
            //headerPositioning: ListView.OverlayHeader
            //header: Status {
            //    implicitHeight: parent.height
            //    z: 2 //make above items
            //}
            //


            //snapMode: ListView.SnapOneItem
            snapMode: ListView.SnapToItem

            //spacing: 5

            model: UnitModel {
                id: unitListModel
                list: unitList
            }


            delegate: TableView {
                id: unitView
                anchors.fill: parent
                anchors.margins: 20

                rowSpacing: 5
                columnSpacing: 5

                clip: true
                model: SlotModel
                delegate: cellDelegate
            }


            delegate: Unit {
                //parent: this
                //moodeli: unitListModel.getUnit(index)
                //moodeli: model

                //required property int index
                //unitpointer: view.model.list
            }


            Component {
                id: unitDelegate
                property UnitItem unit

                unitListModel.setUnit: UnitList.at(index)

                Rectangle {
                    id: wrapper
                    required property int index
                    //required property int number

                    width: 40
                    height: 40

                    Text {
                        anchors.centerIn: parent
                        font.pixelSize: 10
                        text: wrapper.index
                    }
                }
            }

        }
        */
    //}

    //ApplicationFlow {
    //}

    //MainWindow {
        //minimumWidth: mainWindow.Layout.minimumWidth
        //minimumHeight: main.Layout.minimumHeight
        //maximumWidth: main.Layout.maximumWidth
        //maximumHeight: mainWindow.Layout.maximumHeight
    //}

    //minimumWidth: mainWindow.Layout.minimumWidth
    //minimumHeight: mainWindow.main.Layout.minimumHeight
    //maximumWidth: main.Layout.maximumWidth
    //maximumHeight: mainWindow.Layout.maximumHeight



}
