import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import Dispenser 1.0


Rectangle {
    required property UnitItem moodeli

    height: parent.height
    anchors.fill: parent

    color: "#111111"


    TableView {
        id: unitView
        anchors.fill: parent
        anchors.margins: 10

        rowSpacing: 10
        columnSpacing: 10

        //rowsMoved: rowCount()
        property real heightFactor: height / rows
        property real widthFactor: width / columns

        //clip: true

        //model: 4

        /*
        Rectangle {
            implicitHeight: 40
            implicitWidth: 40
            color: "#ff0000"
        }

        Text {
            text: "testiiiiiiiiiiiiiii"
        }
*/

        model: SlotModel {
            id: slotModel
            unit: moodeli
        }


        delegate: SlotButton {
            //state: "up"
            state: model.state
            alarm: model.alarm
            //state: "transit"
        }

        //Shortcut { sequence: StandardKey.Quit; onActivated: Qt.quit() }

            /*
Rectangle {
            id: cell
            implicitHeight: 15
            implicitWidth: 15
            //required property var model
            //required property bool value
            property bool value: true

            color: value ? "#f3f3f4" : "#b5b7bf"

            Text {
                id: test
                anchors.centerIn: parent
                anchors.fill: parent
                width: parent.width

                text: model.display //
                //text: column + "" + row //Toimii ->  00 -> 22
                //text: row //Toimii -> row 0 -> 2
                //text: column //Toimii -> column 0 -> 1
                //text: index //Toimii -> index 0 -> 5
                //text: model.num
                //text: slotModel.data(index)
            }
        }
        */
    }
}

/*
Rectangle {
    id: unitManager

    state: "notConnected"

    states: [
        State {
            name: "notConnected"
        },
        State {
            name: "connected"

            PropertyChanges {
                target: rex
                color: "blue"
            }

            PropertyChanges {
                target: box
                checked: false
            }
        }
    ]

    implicitHeight: view.height

    TableView {
        anchors.fill: parent
        columnSpacing: 1
        rowSpacing: 1
        clip: true

        model: SlotModel {

        }

        delegate: SlotButton {

        }
    }

    StackView {
        id: unitView
        initialItem: init
        anchors.fill: parent
    }

    Component {
        id: init


        RowLayout {
            height: parent.height
            StackView.visible: true

            CheckBox {
                id: box
                checked: model.done
                onClicked: model.done = checked
            }
            Rectangle {
                id: rex
                implicitHeight: 50
                implicitWidth: 50
            }

            TextField {
                Layout.fillWidth: true
                text: model.description
                onEditingFinished: model.description = text
            }

            Button {
                text: qsTr("Connect")
                onClicked: unitManager.state == "connected" ? unitManager.state = "notConnected" : unitManager.state = "connected"
            }
        }
    }

    /*
    pushEnter: Transition {
        id: pushEnter
        ParallelAnimation {
            PropertyAction { property: "x"; value: pushEnter.ViewTransition.item.pos }
            NumberAnimation { properties: "y"; from: pushEnter.ViewTransition.item.pos + stackView.offset; to: pushEnter.ViewTransision.item.pos; duration: 400; easing.type: Easing.OutCubic }
            NumberAnimation { properties: "opacity"; from: 0; to: 1; duration: 400; easing.type: Easing.OutCubic }
        }
    }
    popExit: Transition {
        id: popExit
        ParallelAnimation {
            PropertyAction { property: "x"; value: popExit.ViewTransition.item.pos }
            NumberAnimation { properties: "y"; from: popExit.ViewTransition.item.pos; to: popExit.ViewTransision.item.pos + stackView.offset; duration: 400; easing.type: Easing.OutCubic }
            NumberAnimation { properties: "opacity"; from: 1; to: 0; duration: 400; easing.type: Easing.OutCubic }
        }
    }
    pushExit: Transition {
        id: pushExit
        PropertyAction { property: "x"; value: pushExit.ViewTransition.item.pos }
        PropertyAction { property: "y"; value: pushExit.ViewTransition.item.pos }
    }
    popEnter: Transition {
        id: popEnter
        PropertyAction { property: "x"; value: popEnter.ViewTransition.item.pos }
        PropertyAction { property: "y"; value: popEnter.ViewTransition.item.pos }
    }
    */
//}
//*/
