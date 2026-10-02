#pragma once

#include <QObject>

class QWaylandTextInputManagerV3;
class QWaylandSurface;
class QWaylandSeat;

class WaylandManager : public QObject {
    Q_OBJECT

public:
    explicit WaylandManager(QObject *parent = nullptr);

    Q_INVOKABLE void setupProtocols(QObject *compositorObj);
    Q_INVOKABLE void setKeyboardFocus(QWaylandSurface *surface);
    Q_INVOKABLE void traceFocus(const QString &reason, QWaylandSurface *surface,
                               QObject *item = nullptr) const;

private:
    QWaylandTextInputManagerV3 *m_textInputManagerV3 = nullptr;
    QWaylandSeat *m_seat = nullptr;
};
