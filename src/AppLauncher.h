#pragma once
#include <QObject>
#include <QString>

class AppLauncher : public QObject {
    Q_OBJECT
public:
    explicit AppLauncher(QObject *parent = nullptr);
    
    // Q_INVOKABLE робить цю функцію доступною зсередини QML
    Q_INVOKABLE void launch(const QString &command);
};