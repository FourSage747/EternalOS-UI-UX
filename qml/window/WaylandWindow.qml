import QtQuick
import QtWayland.Compositor
import QtWayland.Compositor.XdgShell

ShellSurfaceItem {
    id: surfaceItem
    autoCreatePopupItems: true
    
    // 1. Дозволяємо компоненту приймати фокус клавіатури
    focus: true

    // MouseArea {
    //     anchors.fill: parent
    //     acceptedButtons: Qt.LeftButton | Qt.RightButton | Qt.MiddleButton
    //     // ВАЖЛИВО: Вимикаємо hover, щоб не красти події наведення (wl_pointer.enter)
    //     hoverEnabled: false 
        
    //     onPressed: (mouse) => {
    //         // Даємо вікну фокус клавіатури
    //         surfaceItem.takeFocus()
    //         // Наказуємо QML пропустити сам клік далі вниз, прямо у Firefox
    //         mouse.accepted = false 
    //     }
    // }

    property var toplevel: shellSurface ? shellSurface.toplevel : null
    property string windowId 
    
    // Прокидаємо зовнішні залежності з Main.qml
    property var model
    property var container
    property var windowManagerRef

    onActiveFocusChanged: {
        console.log(
            "WaylandWindow:",
            windowId,
            "activeFocusChanged =",
            activeFocus,
            "focus =",
            focus
        )
    }

    Component.onCompleted: {
        surfaceItem.z = ++container.globalZ
        surfaceItem.takeFocus()
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
            console.log("WaylandWindow: activated changed =", toplevel.activated)
            console.log("WaylandWindow: states =", toplevel.states)
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