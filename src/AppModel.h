#pragma once
#include <QAbstractListModel>
#include <QList>
#include <QString>

// Структура для зберігання даних однієї програми
struct AppItem {
    QString name;
    QString exec;
    QString icon;
};

class AppModel : public QAbstractListModel {
    Q_OBJECT
public:
    enum AppRoles {
        NameRole = Qt::UserRole + 1,
        ExecRole,
        IconRole
    };

    explicit AppModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

private:
    void loadApps();
    QList<AppItem> m_apps;
};