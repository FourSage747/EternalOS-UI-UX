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
    Q_UNUSED(resource);
    Q_UNUSED(id);

    qDebug() << "PrimarySelection: create_source";
}

void PrimarySelection::zwp_primary_selection_device_manager_v1_get_device(
    Resource *resource,
    uint32_t id,
    struct ::wl_resource *seat)
{
    Q_UNUSED(resource);
    Q_UNUSED(id);
    Q_UNUSED(seat);

    qDebug() << "PrimarySelection: get_device";
}

void PrimarySelection::zwp_primary_selection_device_manager_v1_destroy(
    Resource *resource)
{
    Q_UNUSED(resource);

    qDebug() << "PrimarySelection: destroy";
}