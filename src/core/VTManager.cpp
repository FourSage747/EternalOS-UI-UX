#include "VTManager.h"
#include <fcntl.h>
#include <linux/vt.h>
#include <linux/kd.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <QDebug>

VTManager::VTManager(QObject *parent) : QObject(parent) {
    takeControl();
}

VTManager::~VTManager() {
    releaseControl();
}

void VTManager::takeControl() {
    // Відкриваємо поточний віртуальний термінал, на якому нас запустив SDDM
    ttyFd = open("/dev/tty", O_RDWR | O_NDELAY);
    if (ttyFd < 0) {
        qWarning() << "VTManager: Не вдалося відкрити /dev/tty";
        return;
    }

    // Зберігаємо старі налаштування системи
    ioctl(ttyFd, KDGKBMODE, &oldKbMode);
    ioctl(ttyFd, KDGETMODE, &oldKdMode);

    // 1. Переводимо термінал у графічний режим (щоб текст не малювався поверх Wayland)
    ioctl(ttyFd, KDSETMODE, KD_GRAPHICS);

    // 2. Переводимо клавіатуру в сирий режим (K_RAW). 
    // Це забороняє TTY-драйверу перетворювати Ctrl+C на вбивчий сигнал SIGINT!
    ioctl(ttyFd, KDSKBMODE, K_RAW);

    qDebug() << "VTManager: Успішно захоплено контроль над TTY.";
}

void VTManager::releaseControl() {
    if (ttyFd >= 0) {
        // Повертаємо текстовий режим при виході з композитора
        ioctl(ttyFd, KDSETMODE, oldKdMode);
        ioctl(ttyFd, KDSKBMODE, oldKbMode);
        close(ttyFd);
        ttyFd = -1;
        qDebug() << "VTManager: Контроль над TTY відпущено.";
    }
}