#include "TextInputV1.h"

#include <QDebug>

TextInputV1::TextInputV1(QWaylandCompositor *compositor)
    : QObject(compositor),
      m_compositor(compositor)
{
}

void TextInputV1::setCompositor(QWaylandCompositor *compositor)
{
    m_compositor = compositor;
}

void TextInputV1::initialize()
{
    if (!m_compositor) {
        qDebug() << "TextInputV1: no compositor";
        return;
    }

    if (isGlobal()) {
        qDebug() << "TextInputV1: already initialized";
        return;
    }

    QtWaylandServer::zwp_text_input_manager_v1::init(
        m_compositor->display(),
        1
    );

    qDebug() << "TextInputV1: initialized";
}

void TextInputV1::zwp_text_input_manager_v1_create_text_input(
    Resource *resource,
    uint32_t id)
{
    Q_UNUSED(resource);
    Q_UNUSED(id);

    qDebug() << "TextInputV1: create_text_input";
}