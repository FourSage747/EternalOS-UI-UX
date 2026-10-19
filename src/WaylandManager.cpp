#include "WaylandManager.h"
#include <QWaylandCompositor>
#include <QWaylandSeat>
#include <QWaylandKeyboard>
#include <QWaylandPointer>
#include <QWaylandSurface>
#include <QWaylandClient>
#include <QWaylandTextInputManagerV3>
#include <QDebug>
#include <Qt>
#include <QDateTime>
#include <QQuickItem>
#include <QWaylandXdgSurface>
#include <wayland-server-core.h>

namespace {
QString describeSurface(QWaylandSurface *surface)
{
    if (!surface)
        return QStringLiteral("null");
    auto *xdg = QWaylandXdgSurface::findIn(surface);
    auto *toplevel = xdg ? xdg->toplevel() : nullptr;
    return QStringLiteral("%1 wl_surface#%2 pid=%3 hasContent=%4 appId=%5 title=%6 activated=%7")
        .arg(reinterpret_cast<quintptr>(surface), 0, 16)
        .arg(surface->resource() ? wl_resource_get_id(surface->resource()) : 0)
        .arg(surface->client() ? surface->client()->processId() : 0)
        .arg(surface->hasContent())
        .arg(toplevel ? toplevel->appId() : QString())
        .arg(toplevel ? toplevel->title() : QString())
        .arg(toplevel && toplevel->activated());
}
}

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

    // if (!m_dataDeviceManager) {
    //     m_dataDeviceManager = new DataDeviceManager(compositor);
    //     m_dataDeviceManager->initialize();
    // }

    if (!m_textInputManagerV3) {
        m_textInputManagerV3 =
            new QWaylandTextInputManagerV3(compositor);

        m_textInputManagerV3->initialize();

        qDebug() << "WaylandManager: zwp_text_input_manager_v3 initialized.";
    }

    qDebug() << "========== WAYLAND SEAT DEBUG ==========";
    qDebug() << "Compositor:" << compositor;
    qDebug() << "Socket:" << compositor->socketName();

    //QWaylandSeat *seat = compositor->defaultSeat();
    m_seat = compositor->defaultSeat();
    QWaylandSeat *seat = m_seat;

    qDebug() << "defaultSeat:" << seat;

    if (!seat) {
        qDebug() << "WaylandManager: ERROR: defaultSeat == nullptr";
        return;
    }

    qDebug() << "Seat compositor:" << seat->compositor();
    qDebug() << "Seat initialized:" << seat->isInitialized();
    qDebug() << "Seat capabilities:" << seat->capabilities();

    qDebug() << "Seat keyboard:" << seat->keyboard();
    //qDebug() << "Seat pointer:" << seat->pointer();

    if (!seat->keyboard()) {
        qDebug() << "Creating QWaylandKeyboard...";
        new QWaylandKeyboard(seat);
    }

    /*if (!seat->pointer()) {
        qDebug() << "Creating QWaylandPointer...";
        new QWaylandPointer(seat);
    }*/

    qDebug() << "After device initialization:";
    qDebug() << "Keyboard:" << seat->keyboard();
    //qDebug() << "Pointer:" << seat->pointer();

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
        [this, seat](QWaylandSurface *newFocus,
               QWaylandSurface *oldFocus)
        {
            if (qEnvironmentVariableIsSet("ETERNAL_FOCUS_DEBUG")) {
                qInfo().noquote() << "focus" << QDateTime::currentMSecsSinceEpoch()
                    << "seat.changed old={" << describeSurface(oldFocus)
                    << "} new={" << describeSurface(newFocus) << "}";
                traceFocus(QStringLiteral("seat.changed"), newFocus);
            }
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

void WaylandManager::setKeyboardFocus(QWaylandSurface *surface)
{
    if (!m_seat) {
        qDebug() << "Keyboard: seat is null";
        return;
    }

    traceFocus(QStringLiteral("WaylandManager.setKeyboardFocus"), surface);

    m_seat->setKeyboardFocus(surface);

    qDebug() << "Keyboard focus after:"
             << m_seat->keyboardFocus();
}

void WaylandManager::traceFocus(const QString &reason, QWaylandSurface *surface,
                               QObject *item) const
{
    if (!qEnvironmentVariableIsSet("ETERNAL_FOCUS_DEBUG"))
        return;
    auto *quickItem = qobject_cast<QQuickItem *>(item);
    qInfo().noquote() << "focus" << QDateTime::currentMSecsSinceEpoch() << reason
        << "target={" << describeSurface(surface) << "} seat={"
        << describeSurface(m_seat ? m_seat->keyboardFocus() : nullptr) << "}"
        << "item=" << item
        << "qml.focus=" << (quickItem && quickItem->hasFocus())
        << "qml.activeFocus=" << (quickItem && quickItem->hasActiveFocus());
}
