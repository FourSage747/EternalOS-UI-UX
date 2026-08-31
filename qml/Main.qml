import QtQuick
import QtWayland.Compositor
import QtWayland.Compositor.XdgShell // Додаємо модуль для роботи з вікнами
import QtCore
import Eternal.Core
import "background"
import "window"
import "appmodel"

WaylandCompositor {
    id: compositor
    socketName: "eternal-0"

    WaylandManager { id: waylandManager }
    AppLauncher { id: sysAppLauncher }
    AppModel { id: runningAppsModel }
    WindowManager { 
        id: winManager
        appModel: runningAppsModel
        container: windowsContainer 
    }

    Component.onCompleted: {
        waylandManager.setupProtocols(compositor)
    }
    WaylandOutput {
        sizeFollowsWindow: true
        
        window: Window {
            id: rootWindow
            width: 1024
            height: 768
            visible: true
            title: "Eternal OS Compositor"
            
            DesktopBackground {}
            
            // Це невидимий контейнер, у який ми будемо складати відкриті вікна
            // Оновлений контейнер для вікон
            Item {
                id: windowsContainer
                property int globalZ: 1
                anchors.top: parent.top
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.bottom: taskbar.top // Ключова зміна: контейнер не пускає вікна нижче цієї лінії
            }
            // Панель задач (Taskbar)
            Taskbar {
                id: taskbar
                anchors.bottom: parent.bottom
                width: parent.width
                windowModel: runningAppsModel // Передаємо модель у панель
                onWindowClicked: (winId) => { winManager.toggleWindow(winId) }
                onLaunchRequested: (appExec) => { sysAppLauncher.launch(appExec) }
            }
        }
    }

    AppXdgShell {
        appModel: runningAppsModel
        container: windowsContainer
        windowManagerRef: winManager
    }
}