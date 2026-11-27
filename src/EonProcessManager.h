#pragma once

#include <QObject>
#include <QProcess>
#include <QPointer>

class QWaylandCompositor;

class EonProcessManager : public QObject
{
    Q_OBJECT

public:
    explicit EonProcessManager(QObject *parent = nullptr);

    void setCompositor(QWaylandCompositor *compositor);

private:
    void startEon();
    void stopEon();

    QPointer<QWaylandCompositor> m_compositor;
    QProcess m_process;
    bool m_started = false;
    bool m_stopping = false;
    int m_restartAttempts = 0;
    static constexpr int MaxRestartAttempts = 3;
};