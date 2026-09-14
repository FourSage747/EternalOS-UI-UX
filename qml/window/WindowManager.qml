import QtQuick

QtObject {
    id: windowManager
    
    // Залежності, які ми передамо з Main.qml
    property var appModel
    property var container

    function updateWindowData(winId, key, value) {
        for (let i = 0; i < appModel.count; i++) {
            if (appModel.get(i).winId === winId) {
                appModel.setProperty(i, key, value);
                break;
            }
        }
    }

    function toggleWindow(winId) {
        for (let i = 0; i < container.children.length; i++) {
            let surface = container.children[i];
            
            if (surface.windowId === winId) {
                if (surface.visible && surface.activeFocus) {
                    surface.visible = false;
                    updateWindowData(winId, "isMinimized", true);
                    updateWindowData(winId, "isActive", false);
                } else {
                    surface.visible = true;
                    updateWindowData(winId, "isMinimized", false);
                    updateWindowData(winId, "isActive", true);
                    surface.z = ++container.globalZ;
                    surface.takeFocus();
                }
                break;
            }
        }
    }
}