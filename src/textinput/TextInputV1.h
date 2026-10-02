#pragma once

#include <QWaylandCompositor>

#include "qwayland-server-text-input-unstable-v1.h"

class TextInputV1 : public QObject,
                    public QtWaylandServer::zwp_text_input_manager_v1
{
    Q_OBJECT

public:
    explicit TextInputV1(QWaylandCompositor *compositor = nullptr);

    Q_INVOKABLE void initialize();
    Q_INVOKABLE void setCompositor(QWaylandCompositor *compositor);

protected:
    void zwp_text_input_manager_v1_create_text_input(
        Resource *resource,
        uint32_t id) override;

private:
    QWaylandCompositor *m_compositor = nullptr;
};