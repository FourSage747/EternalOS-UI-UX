import QtQuick
import QtQuick.Controls
import Eternal.Core

Popup {
    id: root
    width: 250
    height: 350
    padding: 0
    closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutsideParent
    onOpened: searchField.forceActiveFocus()
    onClosed: searchField.text = ""
    AppLauncher { id: appLauncher }
    AppModel { id: baseModel }
    AppFilterModel {
        id: filteredModel
        sourceModel: baseModel
        filterText: searchField.text // Зв'язуємо фільтр із текстом у полі
    }

    background: Rectangle {
        color: "#1e1e2e"
        border.color: "#313244"
        border.width: 1
        radius: 8
    }

    Column {
        id: programs
        anchors.fill: parent
        anchors.margins: 10
        spacing: 5
        clip: true

        Column {
            width: parent.width
            anchors.margins: 10
            spacing: 5
            Text {
                text: "Programs"
                color: "#cdd6f4"
                font.pixelSize: 18
                font.bold: true
            }
            // Поле пошуку
            TextField {
                id: searchField
                width: parent.width
                height: 30
                placeholderText: "Search..."
                placeholderTextColor: "#6c7086"
                color: "#cdd6f4"
                font.pixelSize: 14
                
                background: Rectangle {
                    color: "#313244"
                    radius: 4
                }
            }
        }

        ListView {
            width: parent.width
            height: parent.height // Займає весь залишок місця
            clip: true                 // Обрізає вміст, який виходить за межі
            spacing: 2
            
            // Підключаємо нашу C++ модель
            model: filteredModel
            
            // Шаблон для кожної програми
            delegate: Rectangle {
                width: ListView.view.width
                height: 40
                color: mouseArea.containsMouse ? "#313244" : "transparent"
                radius: 4

                Row {
                    anchors.fill: parent
                    anchors.leftMargin: 10
                    anchors.rightMargin: 10
                    spacing: 12

                    // Рендер іконки програми
                    Image {
                        anchors.verticalCenter: parent.verticalCenter
                        width: 20
                        height: 20
                        source: appIcon ? "image://icons/" + appIcon : ""
                        sourceSize: Qt.size(20, 20)
                    }

                    Text {
                        anchors.verticalCenter: parent.verticalCenter
                        text: appName
                        color: "#cdd6f4"
                        font.pixelSize: 14
                    }
                }

                MouseArea {
                    id: mouseArea
                    anchors.fill: parent
                    hoverEnabled: true
                    onClicked: {
                        appLauncher.launch(appExec)
                        root.close()
                    }
                }
            }
        }
    }
}