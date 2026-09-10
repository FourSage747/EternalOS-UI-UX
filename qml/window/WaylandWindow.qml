import QtQuick
import QtWayland.Compositor
import QtWayland.Compositor.XdgShell

ShellSurfaceItem {
    id: surfaceItem
    autoCreatePopupItems: true
    
    // 1. Дозволяємо компоненту приймати фокус клавіатури
    focus: true

    TapHandler {
        onTapped: surfaceItem.forceActiveFocus()
    }
    Keys.onPressed: (event) => {
        console.log("ПЕРЕХОПЛЕНО В КОМПОЗИТОРІ! Код:", event.key, "Текст:", event.text)
    }
    property var toplevel: shellSurface ? shellSurface.toplevel : null
    property string windowId 
    
    // Прокидаємо зовнішні залежності з Main.qml
    property var model
    property var container
    property var windowManagerRef

    Component.onCompleted: {
        surfaceItem.z = ++container.globalZ
        surfaceItem.forceActiveFocus()
    }

    Connections {
        target: toplevel
        
        function onTitleChanged() {
            if (toplevel) windowManagerRef.updateWindowData(windowId, "appTitle", toplevel.title);
        }
        
        function onAppIdChanged() {
            if (!toplevel || !toplevel.appId) return;
            let incomingId = toplevel.appId.toLowerCase();
            let currentIdx = -1;
            let pinnedIdx = -1;

            for (let i = 0; i < model.count; i++) {
                let item = model.get(i);
                if (item.winId === windowId) {
                    currentIdx = i; 
                } else if (item.isPinned && !item.isRunning && 
                          (incomingId.includes(item.appExec) || incomingId.includes(item.appId.toLowerCase()))) {
                    pinnedIdx = i; 
                }
            }

            if (pinnedIdx !== -1 && currentIdx !== -1) {
                model.setProperty(pinnedIdx, "winId", windowId);
                model.setProperty(pinnedIdx, "isRunning", true);
                if (toplevel.title) {
                    model.setProperty(pinnedIdx, "appTitle", toplevel.title);
                }
                model.remove(currentIdx);
            } else if (currentIdx !== -1) {
                windowManagerRef.updateWindowData(windowId, "appId", toplevel.appId);
            }
        }
        
        function onActivatedChanged() {
            if (toplevel) {
                windowManagerRef.updateWindowData(windowId, "isActive", toplevel.activated);
                if (toplevel.activated) {
                    surfaceItem.z = ++container.globalZ;
                }
            }
        }
        
        function onMaximizedChanged() {
            if (!toplevel) return; 
            if (toplevel.maximized) {
                surfaceItem.x = 0
                surfaceItem.y = 0
                toplevel.sendMaximized(Qt.size(container.width, container.height))
            } else {
                toplevel.sendUnmaximized(Qt.size(0, 0))
            }
        }
    }
    
    onSurfaceDestroyed: {
        for (let i = 0; i < model.count; i++) {
            let item = model.get(i);
            if (item.winId === windowId) {
                if (item.isPinned) {
                    model.setProperty(i, "isRunning", false);
                    model.setProperty(i, "isActive", false);
                    model.setProperty(i, "isMinimized", false);
                    model.setProperty(i, "winId", "");
                } else {
                    model.remove(i);
                }
                break;
            }
        }
        destroy();
    }
}