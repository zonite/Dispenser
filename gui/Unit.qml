import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import Dispenser 1.0


TableView {
    id: unitView
    anchors.fill: parent
    anchors.margins: 20
    required property int num

    rowSpacing: 5
    columnSpacing: 5

    clip: true

    model: SlotModel {
        id: unitModel
        //unit: parent
    }

    delegate: Rectangle {
        id: cell
        implicitHeight: 15
        implicitWidth: 15
        required property var model
        required property bool value

        color: value ? "#f3f3f4" : "#b5b7bf"
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
