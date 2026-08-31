#include "AppFilterModel.h"
#include "AppModel.h"

AppFilterModel::AppFilterModel(QObject *parent) : QSortFilterProxyModel(parent) {
    // Вказуємо, що шукати треба саме за назвою програми (NameRole)
    setFilterRole(AppModel::NameRole);
    // Робимо пошук нечутливим до регістру ("fire" знайде "Firefox")
    setFilterCaseSensitivity(Qt::CaseInsensitive);
}

QString AppFilterModel::filterText() const {
    return m_filterText;
}

void AppFilterModel::setFilterText(const QString &text) {
    if (m_filterText == text) return;
    
    m_filterText = text;
    setFilterFixedString(text); // Вбудована функція Qt для фільтрації
    emit filterTextChanged();
}