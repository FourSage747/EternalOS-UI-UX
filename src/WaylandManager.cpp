#include "WaylandManager.h"

#include <QWaylandCompositor>
#include <QWaylandSeat>
#include <QWaylandKeyboard>
#include <QWaylandPointer>
#include <QWaylandSurface>
#include <QWaylandClient>
#include <QDebug>

WaylandManager::WaylandManager(QObject *parent)
    : QObject(parent)
{
}

void WaylandManager::setupProtocols(QObject *compositorObj)
{
    auto *compositor =
        qobject_cast<QWaylandCompositor *>(compositorObj);

    if (!compositor) {
        qDebug() << "WaylandManager: ERROR: compositor == nullptr";
        return;
    }

    qDebug() << "========== WAYLAND SEAT DEBUG ==========";
    qDebug() << "Compositor:" << compositor;
    qDebug() << "Socket:" << compositor->socketName();

    QWaylandSeat *seat = compositor->defaultSeat();

    qDebug() << "defaultSeat:" << seat;

    if (!seat) {
        qDebug() << "WaylandManager: ERROR: defaultSeat == nullptr";
        return;
    }

    qDebug() << "Seat compositor:" << seat->compositor();
    qDebug() << "Seat initialized:" << seat->isInitialized();
    qDebug() << "Seat capabilities:" << seat->capabilities();

    qDebug() << "Seat keyboard:" << seat->keyboard();
    qDebug() << "Seat pointer:" << seat->pointer();

    if (!seat->keyboard()) {
        qDebug() << "Creating QWaylandKeyboard...";
        new QWaylandKeyboard(seat);
    }

    if (!seat->pointer()) {
        qDebug() << "Creating QWaylandPointer...";
        new QWaylandPointer(seat);
    }

    qDebug() << "After device initialization:";
    qDebug() << "Keyboard:" << seat->keyboard();
    qDebug() << "Pointer:" << seat->pointer();

    if (seat->keyboard()) {
        qDebug() << "Keyboard->seat:" << seat->keyboard()->seat();
        qDebug() << "Keyboard focus:"
                 << seat->keyboard()->focus();
        qDebug() << "Keyboard focus client:"
                 << seat->keyboard()->focusClient();
    }

    qDebug() << "Seat keyboardFocus:"
             << seat->keyboardFocus();

    qDebug() << "Seat mouseFocus:"
             << seat->mouseFocus();

    /*
     * Найважливіша діагностика:
     * кожного разу, коли compositor змінює keyboard focus,
     * показуємо старий і новий surface та клієнта.
     */
    connect(
        seat,
        &QWaylandSeat::keyboardFocusChanged,
        this,
        [seat](QWaylandSurface *newFocus,
               QWaylandSurface *oldFocus)
        {
            qDebug() << "";
            qDebug() << "========== KEYBOARD FOCUS CHANGED ==========";
            qDebug() << "OLD surface:" << oldFocus;
            qDebug() << "NEW surface:" << newFocus;

            if (newFocus) {
                qDebug() << "NEW surface client:"
                         << newFocus->client();

                qDebug() << "Seat keyboardFocus now:"
                         << seat->keyboardFocus();

                if (seat->keyboard()) {
                    qDebug() << "Keyboard focus now:"
                             << seat->keyboard()->focus();

                    qDebug() << "Keyboard focus client now:"
                             << seat->keyboard()->focusClient();
                }
            }

            qDebug() << "============================================";
        }
    );

    /*
     * Аналогічно відстежуємо mouse focus.
     * Це дозволить порівняти ситуацію:
     * mouse focus є -> keyboard focus є / немає.
     */
    connect(
        seat,
        &QWaylandSeat::mouseFocusChanged,
        this,
        [seat](QWaylandView *newFocus,
               QWaylandView *oldFocus)
        {
            qDebug() << "";
            qDebug() << "========== MOUSE FOCUS CHANGED ==========";
            qDebug() << "OLD view:" << oldFocus;
            qDebug() << "NEW view:" << newFocus;
            qDebug() << "Seat mouseFocus now:"
                     << seat->mouseFocus();
            qDebug() << "Seat keyboardFocus now:"
                     << seat->keyboardFocus();
            qDebug() << "==========================================";
        }
    );

    qDebug() << "WaylandManager: input devices ready.";
    qDebug() << "=========================================";
}