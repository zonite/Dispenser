import QtQuick 2.7
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.3
import Dispenser 1.0

Frame {
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

    StackView {
        id: unitView

        initialItem: Component {
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

    }
}
