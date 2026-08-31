import QtQuick
import QtCore

Item {
    anchors.fill: parent

    Settings {
        id: appearanceSettings
        category: "Appearance"
        property string wallpaperPath: "qrc:/res/images/background.png" 
    }

    Image {
        anchors.fill: parent
        source: appearanceSettings.wallpaperPath
        fillMode: Image.PreserveAspectCrop 
        asynchronous: true 
        smooth: true
        mipmap: true
    }
}