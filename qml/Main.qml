import QtQuick
import QtWayland.Compositor
import QtWayland.Compositor.XdgShell
import QtCore
import Eternal.Core

WaylandCompositor {
    id: compositor
    socketName: "eternal-0"

    XdgActivation {
        id: xdgActivation
        onActivationRequested: (surface) => winManager.activateSurface(surface)
    }
    PrimarySelection {id: primarySelection}
    TextInputV1 {id: textInputV1}
    LayerShell {id: layerShell}
    XdgOutputManager {id: xdgOutputManager}

    WaylandManager { id: waylandManager }
    AppLauncher { id: sysAppLauncher }
    RunningAppsModel { id: runningAppsModel }
    WindowManager { 
        id: winManager
        appModel: runningAppsModel
        container: windowsContainer 
    }

    Component.onCompleted: {
        xdgActivation.setCompositor(compositor)
        xdgActivation.initialize()
        primarySelection.setCompositor(compositor)
        primarySelection.initialize()
        textInputV1.setCompositor(compositor)
        textInputV1.initialize()
        layerShell.setCompositor(compositor)
        layerShell.initialize()
        xdgOutputManager.setCompositor(compositor)
        xdgOutputManager.initialize()
        waylandManager.setupProtocols(compositor)
    }
    WaylandOutput {
        id: waylandOutput
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

                x: waylandOutput.availableGeometry.x
                y: waylandOutput.availableGeometry.y
                width: waylandOutput.availableGeometry.width

                height: Math.max(
                    0,
                    waylandOutput.availableGeometry.height - taskbar.height
                )
            }
            // Контейнер для Wayland layer-shell поверхонь
            Item {
                id: layerSurfaceContainer
                anchors.fill: parent
                z: 1000
                property var layerSurfaceItems: []
            }
            Connections {
                target: layerShell

                function onLayerSurfaceCreated(layerSurface) {
                    const item = Qt.createQmlObject(
                        'import QtQuick; import QtWayland.Compositor; WaylandQuickItem {}',
                        layerSurfaceContainer
                    )

                    item.surface = layerSurface.surface
                    console.log("Waybar item children:", item.children.length)
                    layerSurfaceContainer.layerSurfaceItems.push({
                        surface: layerSurface,
                        item: item
                    })
                    item.output = layerSurface.output
                    item.anchors.fill = undefined
                    item.z = 1

                    
                    function updateGeometry() {
                        const LEFT = 4
                        const RIGHT = 8
                        const TOP = 1
                        const BOTTOM = 2

                        const anchor = layerSurface.anchor

                        const left = (anchor & LEFT) !== 0
                        const right = (anchor & RIGHT) !== 0
                        const top = (anchor & TOP) !== 0
                        const bottom = (anchor & BOTTOM) !== 0

                        const w = item.implicitWidth
                        const h = item.implicitHeight

                        const containerWidth = layerSurfaceContainer.width
                        const containerHeight = layerSurfaceContainer.height

                        // Horizontal positioning
                        if (left && right) {
                            item.x = layerSurface.marginLeft
                        } else if (left) {
                            item.x = layerSurface.marginLeft
                        } else if (right) {
                            item.x = containerWidth - w - layerSurface.marginRight
                        } else {
                            item.x = (containerWidth - w) / 2
                        }

                        // Vertical positioning
                        if (top && bottom) {
                            item.y = layerSurface.marginTop
                        } else if (top) {
                            item.y = layerSurface.marginTop
                        } else if (bottom) {
                            item.y = containerHeight - h - layerSurface.marginBottom
                        } else {
                            item.y = (containerHeight - h) / 2
                        }

                        console.log(
                            "Layer geometry:",
                            "position =", item.x, item.y,
                            "size =", w, h,
                            "anchor =", anchor
                        )
                    }

                    layerSurface.stateChanged.connect(updateGeometry)
                    item.implicitWidthChanged.connect(updateGeometry)
                    item.implicitHeightChanged.connect(updateGeometry)

                    updateGeometry()

                    console.log("LayerShell: WaylandQuickItem created")
                }
                function onLayerSurfaceRemoved(layerSurface) {
                    const items = layerSurfaceContainer.layerSurfaceItems

                    for (let i = 0; i < items.length; i++) {
                        if (items[i].surface === layerSurface) {
                            const item = items[i].item

                            item.visible = false
                            item.surface = null
                            item.destroy()

                            items.splice(i, 1)
                            break
                        }
                    }

                    console.log(
                        "LayerShell: WaylandQuickItem removed, remaining:",
                        items.length
                    )
                }
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
