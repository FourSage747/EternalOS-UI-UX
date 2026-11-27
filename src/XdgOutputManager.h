#pragma once

#include <QObject>
#include <QWaylandCompositor>
#include <QWaylandCompositorExtension>

#include "qwayland-server-xdg-output-unstable-v1.h"

class XdgOutputManager;

class XdgOutput
    : public QObject,
      public QtWaylandServer::zxdg_output_v1
{
    Q_OBJECT

public:
    XdgOutput(
        XdgOutputManager *manager,
        wl_client *client,
        uint32_t id,
        int version,
        QWaylandOutput *output
    );

    ~XdgOutput() override;

    void sendInitialState();

private:
    QWaylandOutput *m_output = nullptr;

protected:
    void zxdg_output_v1_destroy(Resource *resource) override;
    void zxdg_output_v1_destroy_resource(Resource *resource) override;
};

class XdgOutputManager
    : public QWaylandCompositorExtensionTemplate<XdgOutputManager>,
      public QtWaylandServer::zxdg_output_manager_v1
{
    Q_OBJECT

public:
    explicit XdgOutputManager(QWaylandObject *parent = nullptr);

    Q_INVOKABLE void setCompositor(QWaylandCompositor *compositor);
    Q_INVOKABLE void initialize() override;

private:
    QWaylandCompositor *m_compositor = nullptr;

protected:
    void zxdg_output_manager_v1_get_xdg_output(
        Resource *resource,
        uint32_t id,
        wl_resource *outputResource
    ) override;

    void zxdg_output_manager_v1_destroy(Resource *resource) override;
};