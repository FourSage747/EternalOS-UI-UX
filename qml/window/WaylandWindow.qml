import QtQuick
import QtWayland.Compositor
import QtWayland.Compositor.XdgShell

ShellSurfaceItem {
    id: surfaceItem
    autoCreatePopupItems: true
    focusOnClick: true
    moveItem: surfaceItem
    property int titleBarHeight: 32
    property bool showServerTitleBar: true
    property var toplevel: shellSurface ? shellSurface.toplevel : null
    property string windowId 
    property bool initialPositionSet: false
    
    // Прокидаємо зовнішні залежності з Main.qml
    property var model
    property var container
    property var windowManagerRef

    function updateMaximizedGeometry() {
        if (!toplevel || !toplevel.maximized)
            return

        surfaceItem.x = 0
        surfaceItem.y = 0

        toplevel.sendMaximized(
            Qt.size(container.width, container.height)
        )
    }

    function centerWindow() {
        if (initialPositionSet || !container)
            return

        if (container.width <= 0 || container.height <= 0)
            return

        if (surfaceItem.width <= 0 || surfaceItem.height <= 0)
            return

        surfaceItem.x = Math.max(0, (container.width - surfaceItem.width) / 2)
        surfaceItem.y = Math.max(0, (container.height - surfaceItem.height) / 2)

        initialPositionSet = true
    }

    function activate(reason) {
        waylandManager.traceFocus(reason, surface, surfaceItem);
        // Toplevel creation precedes the first configured buffer commit.
        // A request made before mapping is fulfilled by onHasContentChanged.
        if (!surface || !surface.hasContent)
            return;
        visible = true;
        surfaceItem.z = ++container.globalZ;
        surfaceItem.takeFocus();
    }

    Connections {
        target: surfaceItem.surface
        function onHasContentChanged() {
            if (surfaceItem.surface.hasContent) {
                surfaceItem.centerWindow()
                surfaceItem.activate("mapped")
            }
        }
    }

    onActiveFocusChanged: {
        waylandManager.traceFocus("qml.activeFocusChanged", surface, surfaceItem);
    }

    Component.onCompleted: {
        waylandManager.traceFocus("created", surface, surfaceItem);
        surfaceItem.z = ++container.globalZ
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
            if (!toplevel) return

            if (toplevel.maximized) {
                surfaceItem.updateMaximizedGeometry()
            } else {
                toplevel.sendUnmaximized(Qt.size(0, 0))
            }
        }
    }
    Connections {
        target: container

        function onWidthChanged() {
            surfaceItem.updateMaximizedGeometry()
            surfaceItem.centerWindow()
        }

        function onHeightChanged() {
            surfaceItem.updateMaximizedGeometry()
            surfaceItem.centerWindow()
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

    // TitleBar {
    //     id: serverTitleBar

    //     visible: surfaceItem.showServerTitleBar
    //     width: surfaceItem.width
    //     height: surfaceItem.titleBarHeight
    //     z: 100

    //     toplevel: surfaceItem.toplevel
    //     windowSurface: surfaceItem
    //     container: surfaceItem.container
    // }
}
