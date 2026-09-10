#include "WaylandManager.h"
#include <QWaylandCompositor>
#include <QWaylandSeat>
#include <QWaylandKeyboard>
#include <QWaylandPointer>
#include <QDebug>

WaylandManager::WaylandManager(QObject *parent) : QObject(parent) {}

void WaylandManager::setupProtocols(QObject *compositorObj) {
    auto *compositor = qobject_cast<QWaylandCompositor*>(compositorObj);
    if (!compositor) return;

    // Беремо системне робоче місце, яке Qt 6 вже створив для нас
    QWaylandSeat *seat = compositor->defaultSeat();
    
    if (seat) {
        // Перевіряємо, чи ініціалізовано клавіатуру. Якщо ні — додаємо.
        if (!seat->keyboard()) {
            new QWaylandKeyboard(seat);
        }
        // Те саме для миші
        if (!seat->pointer()) {
            new QWaylandPointer(seat);
        }
        qDebug() << "WaylandManager: Пристрої вводу успішно прив'язані до defaultSeat!";
    }
}