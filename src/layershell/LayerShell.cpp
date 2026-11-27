#include "LayerShell.h"

#include <QDebug>
#include <QtGlobal>

#include <wayland-server-core.h>

// ============================================================
// LayerSurface
// ============================================================

QWaylandSurfaceRole *LayerSurface::role()
{
    static QWaylandSurfaceRole surfaceRole(
        QByteArrayLiteral("zwlr_layer_surface_v1")
    );

    return &surfaceRole;
}


LayerSurface::LayerSurface(
    LayerShell *owner,
    struct ::wl_client *client,
    uint32_t id,
    int version,
    QWaylandSurface *surface,
    QWaylandOutput *output,
    uint32_t layer,
    const QString &surfaceNamespace
)
    : QObject(owner),
      m_owner(owner),
      m_surface(surface),
      m_output(output),
      m_layer(layer),
      m_namespace(surfaceNamespace)
{
    // Початковий стан
    m_pendingLayer = layer;
    m_pendingWidth = 0;
    m_pendingHeight = 0;
    m_pendingAnchor = 0;
    m_pendingExclusiveZone = 0;
    m_pendingKeyboardInteractivity = 0;

    init(client, id, version);

    if (m_surface) {
        connect(
            m_surface,
            &QWaylandSurface::redraw,
            this,
            [this]() {
                if (!m_owner || !m_surface)
                    return;

                // Застосовуємо стан, отриманий до цього commit.
                if (m_pendingState)
                    applyPendingState();

                if (m_configureSent && !m_configurePending)
                    return;

                QWaylandCompositor *comp = m_owner->compositor();

                if (!comp)
                    return;

                QRect geometry;

                if (m_output)
                    geometry = m_output->geometry();

                const bool left = m_anchor & anchor_left;
                const bool right = m_anchor & anchor_right;
                const bool top = m_anchor & anchor_top;
                const bool bottom = m_anchor & anchor_bottom;

                int width = static_cast<int>(m_width);
                int height = static_cast<int>(m_height);

                if (width == 0 && left && right) {
                    width = geometry.width()
                        - m_marginLeft - m_marginRight;
                }

                if (height == 0 && top && bottom) {
                    height = geometry.height()
                        - m_marginTop - m_marginBottom;
                }

                width = qMax(1, width);
                height = qMax(1, height);

                m_lastConfigureSerial = comp->nextSerial();

                send_configure(
                    m_lastConfigureSerial,
                    static_cast<uint32_t>(width),
                    static_cast<uint32_t>(height)
                );

                m_configureSent = true;
                m_configurePending = false;

                qDebug()
                    << "LayerSurface: configure"
                    << m_namespace
                    << width << "x" << height
                    << "serial =" << m_lastConfigureSerial;
            }
        );

        connect(
            m_surface,
            &QWaylandSurface::surfaceDestroyed,
            this,
            &QObject::deleteLater
        );
    }

    qDebug()
        << "LayerSurface created:"
        << m_namespace
        << "layer =" << m_layer;
}

LayerSurface::~LayerSurface()
{
    qDebug() << "LayerSurface destroyed:" << m_namespace;

    if (m_owner)
        emit m_owner->layerSurfaceRemoved(this);
}

void LayerSurface::applyPendingState()
{
    if (!m_pendingState)
        return;

    m_layer = m_pendingLayer;
    m_width = m_pendingWidth;
    m_height = m_pendingHeight;
    m_anchor = m_pendingAnchor;

    m_exclusiveZone = m_pendingExclusiveZone;

    m_marginTop = m_pendingMarginTop;
    m_marginRight = m_pendingMarginRight;
    m_marginBottom = m_pendingMarginBottom;
    m_marginLeft = m_pendingMarginLeft;

    m_keyboardInteractivity = m_pendingKeyboardInteractivity;

    m_pendingState = false;
    m_configurePending = true;

    emit stateChanged();

    qDebug()
        << "LayerSurface: applied pending state"
        << m_namespace
        << "exclusive zone =" << m_exclusiveZone
        << "anchor =" << m_anchor;
}

QWaylandSurface *LayerSurface::surface() const
{
    return m_surface;
}

QWaylandOutput *LayerSurface::output() const
{
    return m_output;
}

void LayerSurface::zwlr_layer_surface_v1_set_size(
    Resource *resource,
    uint32_t width,
    uint32_t height
)
{
    Q_UNUSED(resource);

    m_pendingWidth = width;
    m_pendingHeight = height;
    m_pendingState = true;

    qDebug() << "LayerSurface pending set_size:" << width << height;
}

void LayerSurface::zwlr_layer_surface_v1_set_anchor(
    Resource *resource,
    uint32_t anchor
)
{
    constexpr uint32_t validAnchors =
        anchor_top | anchor_bottom | anchor_left | anchor_right;

    if (anchor & ~validAnchors) {
        wl_resource_post_error(
            resource->handle,
            error_invalid_anchor,
            "Invalid anchor value"
        );
        return;
    }

    m_pendingAnchor = anchor;
    m_pendingState = true;

    qDebug() << "LayerSurface pending set_anchor:" << anchor;
}

void LayerSurface::zwlr_layer_surface_v1_set_exclusive_zone(
    Resource *resource,
    int32_t zone
)
{
    Q_UNUSED(resource);

    m_pendingExclusiveZone = zone;
    m_pendingState = true;

    qDebug() << "LayerSurface pending exclusive zone:" << zone;
}

void LayerSurface::zwlr_layer_surface_v1_set_margin(
    Resource *resource,
    int32_t top,
    int32_t right,
    int32_t bottom,
    int32_t left
)
{
    Q_UNUSED(resource);

    m_pendingMarginTop = top;
    m_pendingMarginRight = right;
    m_pendingMarginBottom = bottom;
    m_pendingMarginLeft = left;

    m_pendingState = true;
}

void LayerSurface::zwlr_layer_surface_v1_set_keyboard_interactivity(
    Resource *resource,
    uint32_t keyboardInteractivity
)
{
    if (keyboardInteractivity > keyboard_interactivity_on_demand) {
        wl_resource_post_error(
            resource->handle,
            error_invalid_keyboard_interactivity,
            "Invalid keyboard interactivity"
        );
        return;
    }

    m_pendingKeyboardInteractivity = keyboardInteractivity;
    m_pendingState = true;

    qDebug()
        << "LayerSurface pending keyboard interactivity:"
        << keyboardInteractivity;
}

void LayerSurface::zwlr_layer_surface_v1_get_popup(
    Resource *resource,
    struct ::wl_resource *popup
)
{
    Q_UNUSED(resource);
    Q_UNUSED(popup);

    qWarning() << "LayerSurface: get_popup is not implemented yet";
}

void LayerSurface::zwlr_layer_surface_v1_ack_configure(
    Resource *resource,
    uint32_t serial
)
{
    Q_UNUSED(resource);

    qDebug() << "LayerSurface ack_configure:" << serial;

    if (serial == m_lastConfigureSerial)
        qDebug() << "LayerSurface: configure acknowledged";
}

void LayerSurface::zwlr_layer_surface_v1_destroy(
    Resource *resource
)
{
    qDebug() << "LayerSurface: client requested destroy";

    wl_resource_destroy(resource->handle);
}

void LayerSurface::zwlr_layer_surface_v1_set_layer(
    Resource *resource,
    uint32_t layer
)
{
    if (layer > 3u) {
        wl_resource_post_error(
            resource->handle,
            error_invalid_surface_state,
            "Invalid layer value"
        );
        return;
    }

    m_pendingLayer = layer;
    m_pendingState = true;

    qDebug() << "LayerSurface pending set_layer:" << layer;
}

void LayerSurface::zwlr_layer_surface_v1_destroy_resource(
    Resource *resource
)
{
    Q_UNUSED(resource);

    deleteLater();
}


// ============================================================
// LayerShell
// ============================================================

LayerShell::LayerShell(QWaylandObject *parent)
    : QWaylandCompositorExtensionTemplate<LayerShell>(parent)
{
    m_compositor = qobject_cast<QWaylandCompositor *>(parent);
}

void LayerShell::setCompositor(QWaylandCompositor *compositor)
{
    m_compositor = compositor;
}

QWaylandCompositor *LayerShell::compositor() const
{
    return m_compositor;
}

void LayerShell::initialize()
{
    if (!m_compositor) {
        qWarning() << "LayerShell: compositor is not set";
        return;
    }

    if (isInitialized())
        return;

    QtWaylandServer::zwlr_layer_shell_v1::init(
        m_compositor->display(),
        4
    );

    QWaylandCompositorExtension::initialize();

    qDebug() << "LayerShell initialized";
}

void LayerShell::zwlr_layer_shell_v1_get_layer_surface(
    Resource *resource,
    uint32_t id,
    struct ::wl_resource *surfaceResource,
    struct ::wl_resource *outputResource,
    uint32_t layer,
    const QString &surfaceNamespace
)
{
    if (!m_compositor)
        return;

    if (layer > layer_overlay) {
        wl_resource_post_error(
            resource->handle,
            error_invalid_layer,
            "Invalid layer value"
        );
        return;
    }

    QWaylandSurface *surface =
        QWaylandSurface::fromResource(surfaceResource);

    if (!surface) {
        qWarning() << "LayerShell: invalid wl_surface";
        return;
    }

    if (surface->hasContent()) {
        wl_resource_post_error(
            resource->handle,
            error_already_constructed,
            "Surface already has content"
        );
        return;
    }

    QWaylandOutput *output = nullptr;

    if (outputResource)
        output = QWaylandOutput::fromResource(outputResource);

    if (!output)
        output = m_compositor->defaultOutput();

    if (!surface->setRole(
            LayerSurface::role(),
            resource->handle,
            error_role
        )) {
        return;
    }

    auto *layerSurface = new LayerSurface(
        this,
        resource->client(),
        id,
        resource->version(),
        surface,
        output,
        layer,
        surfaceNamespace
    );

    m_layerSurfaces.append(layerSurface);

    connect(
        layerSurface,
        &LayerSurface::stateChanged,
        this,
        [this, layerSurface]() {
            updateAvailableGeometry(layerSurface->output());
        }
    );

    connect(
        layerSurface,
        &QObject::destroyed,
        this,
        [this, output]() {
            updateAvailableGeometry(output);
        }
    );

    emit layerSurfaceCreated(layerSurface);

    qDebug()
        << "LayerShell: new layer surface"
        << surfaceNamespace
        << "layer =" << layer
        << "output =" << output;
}

void LayerShell::zwlr_layer_shell_v1_destroy(
    Resource *resource
)
{
    wl_resource_destroy(resource->handle);
}

void LayerShell::updateAvailableGeometry(QWaylandOutput *output)
{
    if (!output)
        return;

    const QRect geometry = output->geometry();

    int reservedTop = 0;
    int reservedBottom = 0;
    int reservedLeft = 0;
    int reservedRight = 0;

    for (const auto &entry : m_layerSurfaces) {
        LayerSurface *surface = entry.data();

        if (!surface || surface->output() != output)
            continue;

        const int zone = surface->exclusiveZone();

        // Нуль і -1 не резервують робочу область.
        if (zone <= 0)
            continue;

        const uint32_t anchor = surface->anchor();

        const bool top = anchor & 1u;
        const bool bottom = anchor & 2u;
        const bool left = anchor & 4u;
        const bool right = anchor & 8u;

        const int topMargin = surface->marginTop();
        const int bottomMargin = surface->marginBottom();
        const int leftMargin = surface->marginLeft();
        const int rightMargin = surface->marginRight();

        // Один край або один край із двома перпендикулярними.
        if (top && !bottom && !left && !right) {
            reservedTop += zone;
        }
        else if (bottom && !top && !left && !right) {
            reservedBottom += zone;
        }
        else if (left && !right && !top && !bottom) {
            reservedLeft += zone;
        }
        else if (right && !left && !top && !bottom) {
            reservedRight += zone;
        }
        else if (top && left && right && !bottom) {
            reservedTop += zone;
        }
        else if (bottom && left && right && !top) {
            reservedBottom += zone;
        }
        else if (left && top && bottom && !right) {
            reservedLeft += zone;
        }
        else if (right && top && bottom && !left) {
            reservedRight += zone;
        }
    }

    const int width = qMax(
        1,
        geometry.width() - reservedLeft - reservedRight
    );

    const int height = qMax(
        1,
        geometry.height() - reservedTop - reservedBottom
    );

    const QRect available(
        reservedLeft,
        reservedTop,
        width,
        height
    );

    output->setAvailableGeometry(available);

    qDebug()
        << "LayerShell: available geometry"
        << available
        << "output =" << output;
}
