#pragma once
#include <QObject>

class WaylandManager : public QObject {
    Q_OBJECT
public:
    explicit WaylandManager(QObject *parent = nullptr);
    
    // Q_INVOKABLE дозволяє викликати цю функцію з QML
    Q_INVOKABLE void setupProtocols(QObject *compositorObj);
};