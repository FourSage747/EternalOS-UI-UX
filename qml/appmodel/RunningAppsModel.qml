import QtQuick

ListModel {
    id: rootModel

    // Масив закріплених програм. Додавай нові сюди.
    property var defaultPinnedApps: [
        { appId: "thunar", appTitle: "Files", appExec: "thunar" },
        { appId: "firefox", appTitle: "Firefox", appExec: "firefox" },
        { appId: "eternalterminal", appTitle: "Eternal Terminal", appExec: "eternal-terminal" }
    ]

    Component.onCompleted: {
        for (let i = 0; i < defaultPinnedApps.length; i++) {
            let app = defaultPinnedApps[i];
            append({
                winId: "",
                appId: app.appId,
                appTitle: app.appTitle,
                appExec: app.appExec,
                isRunning: false,
                isActive: false,
                isMinimized: false,
                isPinned: true
            });
        }
    }
}