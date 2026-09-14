#pragma once

#include <QWaylandCompositorExtension>
#include <QWaylandSeat>
#include <QWaylandSurface>
#include <QHash>
#include <QString>
#include <QWaylandCompositor>

#include "qwayland-server-xdg-activation-v1.h"

class XdgActivation
    : public QWaylandCompositorExtensionTemplate<XdgActivation>,
      public QtWaylandServer::xdg_activation_v1
{
    Q_OBJECT

public:
    explicit XdgActivation(QWaylandObject *parent = nullptr);

    Q_INVOKABLE void initialize() override;
    Q_INVOKABLE void setCompositor(QWaylandCompositor *compositor);

protected:
    void xdg_activation_v1_get_activation_token(
        Resource *resource,
        uint32_t id) override;

    void xdg_activation_v1_activate(
        Resource *resource,
        const QString &token,
        struct ::wl_resource *surface) override;

private:
    struct Token {
        struct ::wl_resource *surface = nullptr;
        struct ::wl_resource *seat = nullptr;
        uint32_t serial = 0;
        QString appId;
        bool committed = false;
    };

    class ActivationToken
        : public QtWaylandServer::xdg_activation_token_v1
    {
    public:
        ActivationToken(
            XdgActivation *owner,
            struct ::wl_client *client,
            uint32_t id,
            int version,
            const QString &token);

        XdgActivation *owner;
        QString token;

    protected:
        void xdg_activation_token_v1_set_serial(
            Resource *resource,
            uint32_t serial,
            struct ::wl_resource *seat) override;

        void xdg_activation_token_v1_set_app_id(
            Resource *resource,
            const QString &appId) override;

        void xdg_activation_token_v1_set_surface(
            Resource *resource,
            struct ::wl_resource *surface) override;

        void xdg_activation_token_v1_commit(
            Resource *resource) override;

        void xdg_activation_token_v1_destroy(
            Resource *resource) override;
    };

    QHash<QString, Token> m_tokens;
    QWaylandCompositor *m_compositor = nullptr;
};