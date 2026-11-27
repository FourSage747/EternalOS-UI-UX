#include "EonProcessManager.h"

#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QProcessEnvironment>
#include <QWaylandCompositor>

EonProcessManager::EonProcessManager(QObject *parent)
    : QObject(parent)
{
    connect(&m_process, &QProcess::errorOccurred,
            this, [this](QProcess::ProcessError error) {
        m_started = false;

        qCritical() << "EON process error:" << error
                    << m_process.errorString();
    });

    connect(&m_process, &QProcess::started,
            this, [this]() {
        m_started = true;
        qInfo() << "EON Shell process started.";
    });

    connect(&m_process,
            qOverload<int, QProcess::ExitStatus>(&QProcess::finished),
            this, [this](int exitCode, QProcess::ExitStatus status) {
        m_started = false;

        qInfo() << "EON Shell finished. Exit code:" << exitCode
                << "Status:" << status;

        if (m_stopping || !m_compositor || !m_compositor->isCreated())
            return;

        if (m_restartAttempts >= MaxRestartAttempts) {
            qCritical() << "EON Shell: maximum restart attempts reached.";
            return;
        }

        ++m_restartAttempts;

        qWarning() << "EON Shell stopped unexpectedly. Restart attempt:"
                << m_restartAttempts << "of" << MaxRestartAttempts;

        startEon();
    });

    connect(qApp, &QCoreApplication::aboutToQuit,
            this, &EonProcessManager::stopEon);
}

void EonProcessManager::setCompositor(QWaylandCompositor *compositor)
{
    if (m_compositor == compositor)
        return;

    if (m_compositor)
        disconnect(m_compositor, nullptr, this, nullptr);

    m_compositor = compositor;

    if (!m_compositor)
        return;

    m_stopping = false;
    m_restartAttempts = 0;

    connect(m_compositor, &QWaylandCompositor::createdChanged,
            this, [this]() {
        if (m_compositor && m_compositor->isCreated())
            startEon();
    });

    connect(m_compositor, &QObject::destroyed,
            this, [this]() {
        stopEon();
        m_compositor = nullptr;
    });

    if (m_compositor->isCreated())
        startEon();
}

void EonProcessManager::startEon()
{
    if (m_started || !m_compositor)
        return;

    const QString appDir = QCoreApplication::applicationDirPath();

    const QStringList executablePaths = {
        QDir::cleanPath(appDir + "/../lib/eternal-ui/eon-shell"),
        QDir::cleanPath(appDir + "/eon-target/release/eon-shell")
    };

    QString executable;

    for (const QString &path : executablePaths) {
        QFileInfo fileInfo(path);

        if (fileInfo.isExecutable()) {
            executable = path;
            break;
        }
    }

    if (executable.isEmpty()) {
        qCritical() << "EON Shell executable not found.";
        return;
    }

    QProcessEnvironment environment =
        QProcessEnvironment::systemEnvironment();

    environment.insert("WAYLAND_DISPLAY", m_compositor->socketName());

    m_process.setProcessEnvironment(environment);
    m_process.setProcessChannelMode(QProcess::ForwardedChannels);
    m_process.setProgram(executable);

    qInfo() << "Starting EON Shell:" << executable;

    m_started = false;
    m_process.start();
}

void EonProcessManager::stopEon()
{
    m_stopping = true;

    if (m_process.state() == QProcess::NotRunning)
        return;

    qInfo() << "Stopping EON Shell...";
    m_process.terminate();

    if (!m_process.waitForFinished(3000)) {
        m_process.kill();
        m_process.waitForFinished();
    }

    m_started = false;
}