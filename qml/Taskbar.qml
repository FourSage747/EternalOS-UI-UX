import QtQuick
import QtQuick.Controls

Rectangle {
    id: root
    height: 50
    color: "#11111b"
    signal windowClicked(string winId)
    signal launchRequested(string appExec)
    property var windowModel
    
    Row {
        anchors.left: parent.left
        anchors.leftMargin: 15
        anchors.verticalCenter: parent.verticalCenter
        spacing: 15
        
        // Кнопка меню (Логотип Eternal OS)
        Rectangle {
            width: 45
            height: 40
            // Робимо легку підсвітку при наведенні, інакше фон прозорий
            color: mouseAreaMenu.containsMouse ? "#313244" : "transparent"
            radius: 6
            
            Launcher {
                id: launcherMenu
                y: -height - 10
                x: 0
            }
            Image {
                anchors.centerIn: parent
                width: 40  // Розмір самого логотипа всередині кнопки
                height: 40
                source: "qrc:/res/images/logo.png"
                sourceSize: Qt.size(40, 40)
                fillMode: Image.PreserveAspectFit
                smooth: true
                mipmap: true
            }
            
            MouseArea { 
                id: mouseAreaMenu // Додали ID для відстеження наведення
                anchors.fill: parent
                hoverEnabled: true 
                onClicked: launcherMenu.opened ? launcherMenu.close() : launcherMenu.open()
            }
        }

        // Розділювач між меню та відкритими програмами
        Rectangle {
            width: 1
            height: 24
            color: "#313244"
            anchors.verticalCenter: parent.verticalCenter
        }

        // Динамічний рендер іконок відкритих вікон
        Repeater {
            model: root.windowModel
            
            Rectangle {
                width: 40
                height: 40
                // Фон підсвічується ТІЛЬКИ якщо програма відкрита І активна
                color: (model.isRunning && model.isActive) ? "#45475a" : (mouseArea.containsMouse ? "#313244" : "transparent")
                radius: 6
                
                Image {
                    anchors.centerIn: parent
                    width: 24
                    height: 24
                    
                    // Якщо appId є, шукаємо його, інакше ставимо заглушку
                    source: model.appId ? "image://icons/" + model.appId : "image://icons/application-x-executable"
                    sourceSize: Qt.size(24, 24)

                    // Якщо оригінальна іконка бита, миттєво підміняємо її
                    onStatusChanged: {
                        if (status === Image.Error) {
                            source = "image://icons/application-x-executable"
                        }
                    }
                }

                // Індикатор активного вікна (синя смужка знизу)
                Rectangle {
                    width: 20
                    height: 3
                    anchors.bottom: parent.bottom
                    anchors.bottomMargin: 2
                    anchors.horizontalCenter: parent.horizontalCenter
                    color: model.isActive ? "#89b4fa" : "#6c7086"
                    visible: model.isRunning
                    radius: 2
                }

                // Спливаюча підказка з назвою вікна
                ToolTip.visible: mouseArea.containsMouse
                ToolTip.text: model.appTitle

                MouseArea {
                    id: mouseArea
                    anchors.fill: parent
                    hoverEnabled: true
                    onClicked: model.isRunning ? root.windowClicked(winId) : root.launchRequested(appExec)
                }
            }
        }
    }
    
    Text {
        id: clockText
        anchors.right: parent.right
        anchors.rightMargin: 20
        anchors.verticalCenter: parent.verticalCenter
        text: Qt.formatDateTime(new Date(), "hh:mm")
        color: "#cdd6f4"
        font.pixelSize: 16
    }
    
    Timer {
        interval: 60000
        running: true
        repeat: true
        onTriggered: clockText.text = Qt.formatDateTime(new Date(), "hh:mm")
    }
}