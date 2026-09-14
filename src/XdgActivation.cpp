#include "XdgActivation.h"

#include <QUuid>

XdgActivation::XdgActivation(QWaylandObject *parent)
    : QWaylandCompositorExtensionTemplate<XdgActivation>(parent)
{
    m_compositor = qobject_cast<QWaylandCompositor *>(parent);
}

void XdgActivation::setCompositor(QWaylandCompositor *compositor)
{
    m_compositor = compositor;
}

void XdgActivation::initialize()
{
    qDebug() << "XdgActivation::initialize()";
    qDebug() << "m_compositor =" << m_compositor;

    if (!m_compositor)
        return;

    qDebug() << "display =" << m_compositor->display();
    qDebug() << "before init, isGlobal =" << isGlobal();

    if (isInitialized())
        return;

    QtWaylandServer::xdg_activation_v1::init(
        m_compositor->display(),
        1
    );

    qDebug() << "after protocol init, isGlobal =" << isGlobal();

    QWaylandCompositorExtension::initialize();

    qDebug() << "after extension init, isInitialized =" << isInitialized();
}

void XdgActivation::xdg_activation_v1_get_activation_token(
    Resource *resource,
    uint32_t id)
{
    const QString token =
        QUuid::createUuid().toString(QUuid::WithoutBraces);

    qDebug() << "XdgActivation: GET ACTIVATION TOKEN";
    qDebug() << "token =" << token;
    qDebug() << "id =" << id;
    qDebug() << "client =" << resource->client();

    m_tokens.insert(token, Token{});
    qDebug() << "XdgActivation instance =" << this;
    qDebug() << "XdgActivation: token stored";
    qDebug() << "tokens count =" << m_tokens.size();

    new ActivationToken(
        this,
        resource->client(),
        id,
        1,
        token
    );
}

void XdgActivation::xdg_activation_v1_activate(
    Resource *resource,
    const QString &token,
    struct ::wl_resource *surface)
{
    Q_UNUSED(resource);

    qDebug() << "XdgActivation: ACTIVATE";
    qDebug() << "token =" << token;
    qDebug() << "surface =" << surface;

    if (!m_compositor) {
        qDebug() << "XdgActivation: NO COMPOSITOR";
        return;
    }
    qDebug() << "XdgActivation instance =" << this;
    qDebug() << "tokens keys =" << m_tokens.keys();
    auto it = m_tokens.find(token);

    if (it == m_tokens.end()) {
        qDebug() << "XdgActivation: TOKEN NOT FOUND";
        return;
    }

    qDebug() << "XdgActivation: token found";
    qDebug() << "committed =" << it->committed;
    qDebug() << "token surface =" << it->surface;
    qDebug() << "token seat =" << it->seat;
    qDebug() << "token serial =" << it->serial;

    if (!it->committed) {
        qDebug() << "XdgActivation: TOKEN NOT COMMITTED";
        return;
    }

    QWaylandSurface *waylandSurface =
        QWaylandSurface::fromResource(surface);

    if (!waylandSurface) {
        qDebug() << "XdgActivation: SURFACE NOT FOUND";
        return;
    }

    qDebug() << "XdgActivation: surface found =" << waylandSurface;

    QWaylandSeat *seat = m_compositor->defaultSeat();

    if (!seat) {
        qDebug() << "XdgActivation: NO SEAT";
        return;
    }

    qDebug() << "XdgActivation: defaultSeat =" << seat;
    qDebug() << "XdgActivation: token seat =" << it->seat;
    qDebug() << "XdgActivation: token serial =" << it->serial;

    qDebug() << "XdgActivation: setting keyboard focus";

    seat->setKeyboardFocus(waylandSurface);
    
    qDebug() << "XdgActivation: keyboard focus after set ="
            << seat->keyboardFocus();
    qDebug() << "XdgActivation: activation request completed";
    m_tokens.erase(it);

    qDebug() << "XdgActivation: keyboard focus set";
}


// ------------------------------------------------------------
// ActivationToken
// ------------------------------------------------------------

XdgActivation::ActivationToken::ActivationToken(
    XdgActivation *owner,
    struct ::wl_client *client,
    uint32_t id,
    int version,
    const QString &token)
    : QtWaylandServer::xdg_activation_token_v1(client, id, version),
      owner(owner),
      token(token)
{
}

void XdgActivation::ActivationToken::xdg_activation_token_v1_set_serial(
    Resource *resource,
    uint32_t serial,
    struct ::wl_resource *seat)
{
    Q_UNUSED(resource);

    if (!owner || !owner->m_tokens.contains(token))
        return;

    auto &data = owner->m_tokens[token];

    data.serial = serial;
    data.seat = seat;
}

void XdgActivation::ActivationToken::xdg_activation_token_v1_set_app_id(
    Resource *resource,
    const QString &appId)
{
    Q_UNUSED(resource);

    if (!owner || !owner->m_tokens.contains(token))
        return;

    owner->m_tokens[token].appId = appId;
}

void XdgActivation::ActivationToken::xdg_activation_token_v1_set_surface(
    Resource *resource,
    struct ::wl_resource *surface)
{
    Q_UNUSED(resource);

    if (!owner || !owner->m_tokens.contains(token))
        return;

    owner->m_tokens[token].surface = surface;
}

void XdgActivation::ActivationToken::xdg_activation_token_v1_commit(
    Resource *resource)
{
    Q_UNUSED(resource);

    if (!owner || !owner->m_tokens.contains(token))
        return;

    owner->m_tokens[token].committed = true;

    send_done(token);
}

void XdgActivation::ActivationToken::xdg_activation_token_v1_destroy(
    Resource *resource)
{
    Q_UNUSED(resource);

    delete this;
}