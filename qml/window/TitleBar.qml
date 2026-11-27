import QtQuick

Rectangle {
    id: titleBar

    property var toplevel
    property var windowSurface
    property var container

    height: 34
    color: "#252525"

    Row {
        anchors.fill: parent
        anchors.leftMargin: 12
        anchors.rightMargin: 8
        spacing: 8

        Text {
            anchors.verticalCenter: parent.verticalCenter
            width: parent.width - 100
            text: titleBar.toplevel ? titleBar.toplevel.title : "Application"
            color: "#ffffff"
            font.pixelSize: 12
            elide: Text.ElideRight
        }
    }

    Row {
        anchors.right: parent.right
        anchors.rightMargin: 6
        anchors.verticalCenter: parent.verticalCenter
        spacing: 4
        z: 2

        // Maximize / Restore
        Rectangle {
            width: 26
            height: 26
            radius: 4
            color: maximizeMouse.containsMouse ? "#454545" : "transparent"

            Text {
                anchors.centerIn: parent
                text: titleBar.toplevel && titleBar.toplevel.maximized ? "❐" : "□"
                color: "#ffffff"
                font.pixelSize: 16
            }

            MouseArea {
                id: maximizeMouse
                anchors.fill: parent
                hoverEnabled: true

                onClicked: {
                    if (!titleBar.toplevel || !titleBar.windowSurface)
                        return

                    if (titleBar.toplevel.maximized) {
                        titleBar.toplevel.sendUnmaximized(Qt.size(0, 0))
                    } else {
                        titleBar.toplevel.sendMaximized(
                            Qt.size(titleBar.container.width, titleBar.container.height)
                        )
                    }
                }
            }
        }

        // Close
        Rectangle {
            width: 26
            height: 26
            radius: 4
            color: closeMouse.containsMouse ? "#c42b3b" : "transparent"

            Text {
                anchors.centerIn: parent
                text: "×"
                color: "#ffffff"
                font.pixelSize: 19
            }

            MouseArea {
                id: closeMouse
                anchors.fill: parent
                hoverEnabled: true

                onClicked: {
                    if (titleBar.toplevel)
                        titleBar.toplevel.sendClose()
                }
            }
        }
    }

    MouseArea {
        anchors.fill: parent
        acceptedButtons: Qt.LeftButton
        preventStealing: true
        cursorShape: Qt.SizeAllCursor

        property point initialPointerPosition: Qt.point(0, 0)
        property point initialWindowPosition: Qt.point(0, 0)

        onPressed: (mouse) => {
            if (!windowSurface || !container)
                return

            windowSurface.activate("titlebar")

            initialPointerPosition = titleBar.mapToItem(
                container, mouse.x, mouse.y
            )

            initialWindowPosition = Qt.point(
                windowSurface.x, windowSurface.y
            )
        }

        onPositionChanged: (mouse) => {
            if (!(mouse.buttons & Qt.LeftButton) ||
                !windowSurface || !container)
                return

            let currentPointerPosition = titleBar.mapToItem(
                container, mouse.x, mouse.y
            )

            let deltaX = currentPointerPosition.x - initialPointerPosition.x
            let deltaY = currentPointerPosition.y - initialPointerPosition.y

            windowSurface.x = initialWindowPosition.x + deltaX
            windowSurface.y = initialWindowPosition.y + deltaY
        }
    }
}