#include "DataDeviceManager.h"

#include <QDebug>

DataDeviceManager::DataDeviceManager(QWaylandCompositor *compositor)
    : QObject(compositor),
      m_compositor(compositor)
{
}

void DataDeviceManager::initialize()
{
    if (!m_compositor) {
        qDebug() << "DataDeviceManager: no compositor";
        return;
    }

    if (isGlobal()) {
        qDebug() << "DataDeviceManager: already initialized";
        return;
    }

    QtWaylandServer::wl_data_device_manager::init(
        m_compositor->display(),
        3
    );

    qDebug() << "DataDeviceManager: initialized with version 3";
}

void DataDeviceManager::data_device_manager_create_data_source(
    Resource *resource,
    uint32_t id)
{
    Q_UNUSED(resource);
    Q_UNUSED(id);

    qDebug() << "DataDeviceManager: create_data_source";
}

void DataDeviceManager::data_device_manager_get_data_device(
    Resource *resource,
    uint32_t id,
    struct ::wl_resource *seat)
{
    Q_UNUSED(resource);
    Q_UNUSED(id);
    Q_UNUSED(seat);

    qDebug() << "DataDeviceManager: get_data_device";
}

void DataDeviceManager::data_device_manager_release(
    Resource *resource)
{
    Q_UNUSED(resource);

    qDebug() << "DataDeviceManager: release";
}