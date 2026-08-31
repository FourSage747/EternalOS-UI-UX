#include "WaylandManager.h"
#include <QWaylandCompositor>
#include <QWaylandSeat>
#include <QDebug>

WaylandManager::WaylandManager(QObject *parent) : QObject(parent) {}

void WaylandManager::setupProtocols(QObject *compositorObj) {
    auto *compositor = qobject_cast<QWaylandCompositor*>(compositorObj);
    if (!compositor) {
        qWarning() << "Помилка: Переданий об'єкт не є WaylandCompositor";
        return;
    }

    // Створюємо робоче місце (Seat). 
    // У QtWayland це автоматично активує пристрої вводу та базовий буфер обміну.
    QWaylandSeat *seat = new QWaylandSeat(compositor);
    
    qDebug() << "Wayland-протоколи успішно ініціалізовано";
}