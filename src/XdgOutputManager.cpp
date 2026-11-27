#include <QWaylandClient>
#include "XdgOutputManager.h"
#include <wayland-server-protocol.h>
#include <QDebug>
#include <QRect>

XdgOutput::XdgOutput(
    XdgOutputManager *manager,
    wl_client *client,
    uint32_t id,
    int version,
    QWaylandOutput *output
)
    : QObject(manager),
      m_output(output)
{
    init(client, id, version);

    if (m_output)
        sendInitialState();
}

XdgOutput::~XdgOutput() = default;

void XdgOutput::sendInitialState()
{
    if (!m_output)
        return;

    const QRect geometry = m_output->geometry();
    const QPoint position = m_output->position();

    send_logical_position(position.x(), position.y());
    send_logical_size(geometry.width(), geometry.height());

    if (resource()->version() >= 2) {
        QString name = m_output->model();

        if (name.isEmpty())
            name = QStringLiteral("Eternal-1");

        send_name(name);

        const QString description =
            QStringLiteral("%1 %2")
                .arg(m_output->manufacturer(), m_output->model())
                .trimmed();

        if (!description.isEmpty())
            send_description(description);
    }

    if (resource()->version() >= 3) {
        auto *client = QWaylandClient::fromWlClient(
            m_output->compositor(),
            resource()->client()
        );

        auto *outputResource = client
            ? m_output->resourceForClient(client)
            : nullptr;

        if (outputResource)
            wl_output_send_done(outputResource);
    } else {
        send_done();
    }
}

void XdgOutput::zxdg_output_v1_destroy(Resource *resource)
{
    wl_resource_destroy(resource->handle);
}

void XdgOutput::zxdg_output_v1_destroy_resource(Resource *resource)
{
    Q_UNUSED(resource);
    deleteLater();
}


// ============================================================
// XdgOutputManager
// ============================================================

XdgOutputManager::XdgOutputManager(QWaylandObject *parent)
    : QWaylandCompositorExtensionTemplate<XdgOutputManager>(parent)
{
    m_compositor = qobject_cast<QWaylandCompositor *>(parent);
}

void XdgOutputManager::setCompositor(QWaylandCompositor *compositor)
{
    m_compositor = compositor;
}

void XdgOutputManager::initialize()
{
    if (!m_compositor) {
        qWarning() << "XdgOutputManager: compositor is not set";
        return;
    }

    if (isInitialized())
        return;

    QtWaylandServer::zxdg_output_manager_v1::init(
        m_compositor->display(),
        3
    );

    QWaylandCompositorExtension::initialize();

    qDebug() << "XdgOutputManager initialized";
}

void XdgOutputManager::zxdg_output_manager_v1_get_xdg_output(
    Resource *resource,
    uint32_t id,
    wl_resource *outputResource
)
{
    QWaylandOutput *output =
        QWaylandOutput::fromResource(outputResource);

    if (!output) {
        qWarning() << "XdgOutputManager: invalid wl_output resource";
        return;
    }

    new XdgOutput(
        this,
        resource->client(),
        id,
        resource->version(),
        output
    );

    qDebug() << "XdgOutputManager: created xdg_output for"
             << output->manufacturer()
             << output->model();
}

void XdgOutputManager::zxdg_output_manager_v1_destroy(
    Resource *resource
)
{
    wl_resource_destroy(resource->handle);
}