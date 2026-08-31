import QtQuick
import QtWayland.Compositor.XdgShell

XdgShell {
    id: xdgShell

    // Залежності, які ми передамо з Main.qml
    property var appModel
    property var container
    property var windowManagerRef

    onToplevelCreated: (toplevel, xdgSurface) => {
        let uniqueId = Math.random().toString();

        // Створюємо вікно у переданому контейнері
        let winComp = Qt.createComponent("WaylandWindow.qml");
        winComp.createObject(container, {
            "shellSurface": xdgSurface,
            "windowId": uniqueId,
            "model": appModel,
            "container": container,
            "windowManagerRef": windowManagerRef
        });

        let foundPinned = false;
        let incomingAppId = toplevel.appId ? toplevel.appId.toLowerCase() : "";
        
        for (let i = 0; i < appModel.count; i++) {
            let item = appModel.get(i);
            if (item.isPinned && !item.isRunning && 
               (incomingAppId.includes(item.appExec.toLowerCase()) || incomingAppId === item.appId.toLowerCase())) {
                appModel.setProperty(i, "winId", uniqueId);
                appModel.setProperty(i, "isRunning", true);
                appModel.setProperty(i, "appTitle", toplevel.title || item.appTitle);
                foundPinned = true;
                break;
            }
        }

        if (!foundPinned) {
            appModel.append({
                "winId": uniqueId,
                "appTitle": toplevel.title || "Loading...",
                "appId": toplevel.appId || "",
                "isActive": false,
                "isMinimized": false,
                "isRunning": true,
                "isPinned": false,
                "appExec": ""
            });
        }
    }
}