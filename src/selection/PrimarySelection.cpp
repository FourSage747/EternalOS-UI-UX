#include "PrimarySelection.h"

#include <QDebug>

PrimarySelection::PrimarySelection(QWaylandCompositor *compositor)
    : QWaylandCompositorExtensionTemplate<PrimarySelection>(compositor),
      m_compositor(compositor)
{
}

void PrimarySelection::setCompositor(QWaylandCompositor *compositor)
{
    m_compositor = compositor;
}

void PrimarySelection::initialize()
{
    qDebug() << "PrimarySelection::initialize()";
    qDebug() << "m_compositor =" << m_compositor;

    if (!m_compositor) {
        qDebug() << "PrimarySelection: no compositor";
        return;
    }

    if (isInitialized())
        return;

    QtWaylandServer::zwp_primary_selection_device_manager_v1::init(
        m_compositor->display(),
        1
    );

    QWaylandCompositorExtension::initialize();

    qDebug() << "PrimarySelection: initialized";
    qDebug() << "isGlobal =" << isGlobal();
    qDebug() << "isInitialized =" << isInitialized();
}

void PrimarySelection::zwp_primary_selection_device_manager_v1_create_source(
    Resource *resource,
    uint32_t id)
{
    auto *source = new QtWaylandServer::zwp_primary_selection_source_v1();

    source->init(resource->client(), id, 1);

    qDebug() << "PrimarySelection: source created, id =" << id;
}

void PrimarySelection::zwp_primary_selection_device_manager_v1_get_device(
    Resource *resource,
    uint32_t id,
    struct ::wl_resource *seat)
{
    if (!seat) {
        qWarning() << "PrimarySelection: get_device received null seat";
        return;
    }

    auto *device = new PrimarySelectionDevice();

    device->init(resource->client(), id, 1);

    qDebug() << "PrimarySelection: device created, id =" << id
             << "seat =" << seat;
}

void PrimarySelection::zwp_primary_selection_device_manager_v1_destroy(
    Resource *resource)
{
    Q_UNUSED(resource);

    qDebug() << "PrimarySelection: destroy";
}

void PrimarySelectionDevice::zwp_primary_selection_device_v1_set_selection(
    Resource *resource,
    struct ::wl_resource *source,
    uint32_t serial)
{
    Q_UNUSED(resource);

    qDebug() << "PrimarySelection: set_selection"
             << "source =" << source
             << "serial =" << serial;
}