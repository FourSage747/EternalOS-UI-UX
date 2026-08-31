#include "AppModel.h"
#include <QDir>
#include <QSettings>
#include <QRegularExpression>

AppModel::AppModel(QObject *parent) : QAbstractListModel(parent) {
    loadApps();
}

int AppModel::rowCount(const QModelIndex &parent) const {
    if (parent.isValid()) return 0;
    return m_apps.count();
}

QVariant AppModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid() || index.row() >= m_apps.size()) return QVariant();
    
    const AppItem &item = m_apps[index.row()];
    if (role == NameRole) return item.name;
    if (role == ExecRole) return item.exec;
    if (role == IconRole) return item.icon;
    
    return QVariant();
}

QHash<int, QByteArray> AppModel::roleNames() const {
    QHash<int, QByteArray> roles;
    roles[NameRole] = "appName";
    roles[ExecRole] = "appExec";
    roles[IconRole] = "appIcon";
    return roles;
}

void AppModel::loadApps() {
    QDir dir("/usr/share/applications");
    dir.setNameFilters({"*.desktop"});

    for (const QFileInfo &fileInfo : dir.entryInfoList()) {
        QSettings desktopFile(fileInfo.absoluteFilePath(), QSettings::IniFormat);
        desktopFile.beginGroup("Desktop Entry");
        
        // Пропускаємо приховані програми та системні віджети
        if (desktopFile.value("NoDisplay", false).toBool() || 
            desktopFile.value("Type").toString() != "Application") {
            continue;
        }

        QString name = desktopFile.value("Name").toString();
        QString exec = desktopFile.value("Exec").toString();
        QString icon = desktopFile.value("Icon").toString();

        if (!name.isEmpty() && !exec.isEmpty()) {
            // Очищаємо команду від аргументів на кшталт %U або %f
            exec.remove(QRegularExpression(" %[a-zA-Z]"));
            m_apps.append({name, exec, icon});
        }
        desktopFile.endGroup();
    }
}