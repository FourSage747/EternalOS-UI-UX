#pragma once
#include <QObject>

class VTManager : public QObject {
    Q_OBJECT
public:
    explicit VTManager(QObject *parent = nullptr);
    ~VTManager();

private:
    void takeControl();
    void releaseControl();

    int ttyFd = -1;
    int oldKbMode = 0;
    int oldKdMode = 0;
};