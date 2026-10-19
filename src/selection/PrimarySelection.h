#pragma once

#include <QWaylandCompositorExtension>
#include <QWaylandCompositor>

#include "qwayland-server-primary-selection-unstable-v1.h"

class PrimarySelectionDevice
    : public QtWaylandServer::zwp_primary_selection_device_v1
{
protected:
    void zwp_primary_selection_device_v1_set_selection(
        Resource *resource,
        struct ::wl_resource *source,
        uint32_t serial) override;
};

class PrimarySelection
    : public QWaylandCompositorExtensionTemplate<PrimarySelection>,
      public QtWaylandServer::zwp_primary_selection_device_manager_v1
{
    Q_OBJECT

public:
    explicit PrimarySelection(QWaylandCompositor *compositor = nullptr);

    Q_INVOKABLE void initialize() override;
    Q_INVOKABLE void setCompositor(QWaylandCompositor *compositor);

protected:
    void zwp_primary_selection_device_manager_v1_create_source(
        Resource *resource,
        uint32_t id) override;

    void zwp_primary_selection_device_manager_v1_get_device(
        Resource *resource,
        uint32_t id,
        struct ::wl_resource *seat) override;

    void zwp_primary_selection_device_manager_v1_destroy(
        Resource *resource) override;

private:
    QWaylandCompositor *m_compositor = nullptr;
};