#pragma once
#include <QList>
#include <QPointer>
#include <QObject>
#include <QWaylandCompositor>
#include <QWaylandCompositorExtension>
#include <QWaylandSurface>
#include <QWaylandOutput>

#include "qwayland-server-wlr-layer-shell-unstable-v1.h"

class LayerShell;

class LayerSurface
    : public QObject,
      public QtWaylandServer::zwlr_layer_surface_v1
{
    Q_OBJECT

    Q_PROPERTY(QWaylandSurface *surface READ surface CONSTANT)
    Q_PROPERTY(QWaylandOutput *output READ output CONSTANT)

    Q_PROPERTY(uint32_t requestedWidth READ requestedWidth NOTIFY stateChanged)
    Q_PROPERTY(uint32_t requestedHeight READ requestedHeight NOTIFY stateChanged)
    Q_PROPERTY(uint32_t anchor READ anchor NOTIFY stateChanged)

    Q_PROPERTY(int32_t marginTop READ marginTop NOTIFY stateChanged)
    Q_PROPERTY(int32_t marginRight READ marginRight NOTIFY stateChanged)
    Q_PROPERTY(int32_t marginBottom READ marginBottom NOTIFY stateChanged)
    Q_PROPERTY(int32_t marginLeft READ marginLeft NOTIFY stateChanged)

public:
    LayerSurface(
        LayerShell *owner,
        struct ::wl_client *client,
        uint32_t id,
        int version,
        QWaylandSurface *surface,
        QWaylandOutput *output,
        uint32_t layer,
        const QString &surfaceNamespace
    );

    ~LayerSurface() override;

    static QWaylandSurfaceRole *role();

    QWaylandSurface *surface() const;
    QWaylandOutput *output() const;
    uint32_t requestedWidth() const { return m_width; }
    uint32_t requestedHeight() const { return m_height; }
    uint32_t anchor() const { return m_anchor; }

    int32_t marginTop() const { return m_marginTop; }
    int32_t marginRight() const { return m_marginRight; }
    int32_t marginBottom() const { return m_marginBottom; }
    int32_t marginLeft() const { return m_marginLeft; }
    int32_t exclusiveZone() const { return m_exclusiveZone; }
    uint32_t layer() const { return m_layer; }

signals:
    void stateChanged();

private:
    LayerShell *m_owner = nullptr;
    QWaylandSurface *m_surface = nullptr;
    QWaylandOutput *m_output = nullptr;

    uint32_t m_lastConfigureSerial = 0;
    bool m_configureSent = false;
    bool m_configurePending = false;

    uint32_t m_layer = 0;
    QString m_namespace;

    uint32_t m_width = 0;
    uint32_t m_height = 0;
    uint32_t m_anchor = 0;

    int32_t m_exclusiveZone = 0;
    int32_t m_marginTop = 0;
    int32_t m_marginRight = 0;
    int32_t m_marginBottom = 0;
    int32_t m_marginLeft = 0;

    uint32_t m_keyboardInteractivity = 0;
        // Pending state — застосовується під час wl_surface.commit
    bool m_pendingState = false;

    uint32_t m_pendingLayer = 0;
    uint32_t m_pendingWidth = 0;
    uint32_t m_pendingHeight = 0;
    uint32_t m_pendingAnchor = 0;

    int32_t m_pendingExclusiveZone = 0;
    int32_t m_pendingMarginTop = 0;
    int32_t m_pendingMarginRight = 0;
    int32_t m_pendingMarginBottom = 0;
    int32_t m_pendingMarginLeft = 0;

    uint32_t m_pendingKeyboardInteractivity = 0;
    void applyPendingState();
protected:
    void zwlr_layer_surface_v1_destroy_resource(
        Resource *resource
    ) override;

    void zwlr_layer_surface_v1_set_size(
        Resource *resource,
        uint32_t width,
        uint32_t height
    ) override;

    void zwlr_layer_surface_v1_set_anchor(
        Resource *resource,
        uint32_t anchor
    ) override;

    void zwlr_layer_surface_v1_set_exclusive_zone(
        Resource *resource,
        int32_t zone
    ) override;

    void zwlr_layer_surface_v1_set_margin(
        Resource *resource,
        int32_t top,
        int32_t right,
        int32_t bottom,
        int32_t left
    ) override;

    void zwlr_layer_surface_v1_set_keyboard_interactivity(
        Resource *resource,
        uint32_t keyboardInteractivity
    ) override;

    void zwlr_layer_surface_v1_get_popup(
        Resource *resource,
        struct ::wl_resource *popup
    ) override;

    void zwlr_layer_surface_v1_ack_configure(
        Resource *resource,
        uint32_t serial
    ) override;

    void zwlr_layer_surface_v1_destroy(
        Resource *resource
    ) override;

    void zwlr_layer_surface_v1_set_layer(
        Resource *resource,
        uint32_t layer
    ) override;
};


class LayerShell
    : public QWaylandCompositorExtensionTemplate<LayerShell>,
      public QtWaylandServer::zwlr_layer_shell_v1
{
    Q_OBJECT

public:
    explicit LayerShell(QWaylandObject *parent = nullptr);

    Q_INVOKABLE void initialize() override;
    Q_INVOKABLE void setCompositor(QWaylandCompositor *compositor);
    QWaylandCompositor *compositor() const;

signals:
    void layerSurfaceCreated(LayerSurface *surface);
    void layerSurfaceRemoved(LayerSurface *surface);

protected:
    void zwlr_layer_shell_v1_get_layer_surface(
        Resource *resource,
        uint32_t id,
        struct ::wl_resource *surface,
        struct ::wl_resource *output,
        uint32_t layer,
        const QString &surfaceNamespace
    ) override;

    void zwlr_layer_shell_v1_destroy(
        Resource *resource
    ) override;

private:
    QWaylandCompositor *m_compositor = nullptr;

    QList<QPointer<LayerSurface>> m_layerSurfaces;

    void updateAvailableGeometry(QWaylandOutput *output);
};
