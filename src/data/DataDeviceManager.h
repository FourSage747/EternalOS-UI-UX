#pragma once

#include <QObject>
#include <QWaylandCompositor>

#include "qwayland-server-wayland.h"

class DataDeviceManager : public QObject,
                          public QtWaylandServer::wl_data_device_manager
{
    Q_OBJECT

public:
    explicit DataDeviceManager(QWaylandCompositor *compositor);

    void initialize();

protected:
    void data_device_manager_create_data_source(
        Resource *resource,
        uint32_t id) override;

    void data_device_manager_get_data_device(
        Resource *resource,
        uint32_t id,
        struct ::wl_resource *seat) override;

    void data_device_manager_release(
        Resource *resource) override;

private:
    QWaylandCompositor *m_compositor = nullptr;
};