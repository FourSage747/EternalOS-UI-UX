#include "AppLauncher.h"
#include <QProcess>
#include <QProcessEnvironment>
#include <QDebug>

AppLauncher::AppLauncher(QObject *parent) : QObject(parent) {}

void AppLauncher::launch(const QString &command) {
    qDebug() << "Запускаємо:" << command;
    
    // Налаштовуємо середовище, щоб програми автоматично відкривалися в нашому композиторі
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    env.insert("WAYLAND_DISPLAY", "eternal-0");
    env.insert("MOZ_ENABLE_WAYLAND", "1");      // Примусовий Wayland для Firefox
    env.remove("DISPLAY");
    // ДОДАЄМО ЦЕ: Примусово вказуємо програмам бути клієнтами!
    env.insert("QT_QPA_PLATFORM", "wayland");
    QProcess process;
    process.setProcessEnvironment(env);
    
    // Використовуємо 'sh -c', щоб можна було передавати команди з аргументами
    process.setProgram("sh");
    process.setArguments({"-c", command});
    
    // Запускаємо процес відв'язано від нашого UI (щоб він не закрився разом з композитором)
    process.startDetached();
}