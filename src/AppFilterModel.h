#pragma once
#include <QSortFilterProxyModel>
#include <QString>

class AppFilterModel : public QSortFilterProxyModel {
    Q_OBJECT
    // Властивість, яку ми будемо прив'язувати до текстового поля в QML
    Q_PROPERTY(QString filterText READ filterText WRITE setFilterText NOTIFY filterTextChanged)
    
public:
    explicit AppFilterModel(QObject *parent = nullptr);
    
    QString filterText() const;
    void setFilterText(const QString &text);

signals:
    void filterTextChanged();

private:
    QString m_filterText;
};