#pragma once
#include <QQuickImageProvider>
#include <QIcon>
#include <QPixmap>
#include <QDebug>

class IconImageProvider : public QQuickImageProvider {
public:
    IconImageProvider() : QQuickImageProvider(QQuickImageProvider::Image) {}

    QImage requestImage(const QString &id, QSize *size, const QSize &requestedSize) override {
        // Пробуємо знайти іконку за переданим Wayland ID
        QIcon icon = QIcon::fromTheme(id);
        
        // Якщо іконку не знайдено або id порожній, ставимо системну заглушку
        if (icon.isNull() || id.isEmpty()) {
            qDebug() << "Не знайдено іконку для вікна з appId:" << id << "-> підставляємо заглушку";
            icon = QIcon::fromTheme("application-x-executable");
        }

        int targetSize = (requestedSize.width() > 0) ? requestedSize.width() : 24;
        QPixmap pixmap = icon.pixmap(targetSize, targetSize);
        if (size) *size = pixmap.size();
        
        return pixmap.toImage();
    }
};