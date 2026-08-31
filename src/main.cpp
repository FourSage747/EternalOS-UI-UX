#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include "AppLauncher.h"
#include "WaylandManager.h"
#include "AppModel.h"
#include "IconImageProvider.h"
#include "AppFilterModel.h"

int main(int argc, char *argv[])
{
    // Обов'язково для роботи Wayland-композитора всередині вікна (для тестування)
    qputenv("QT_QPA_PLATFORM", "wayland;xcb");
    qputenv("QT_WAYLAND_CLIENT_BUFFER_INTEGRATION", "wayland-egl");
    qputenv("QT_WAYLAND_HARDWARE_INTEGRATION", "wayland-egl");

    QGuiApplication app(argc, argv);
    QCoreApplication::setOrganizationName("EternalOS");
    QCoreApplication::setOrganizationDomain("eternal-os.org");
    // Реєструємо клас як QML-тип "AppLauncher" у модулі "Eternal.Core" версії 1.0
    qmlRegisterType<AppLauncher>("Eternal.Core", 1, 0, "AppLauncher");
    qmlRegisterType<WaylandManager>("Eternal.Core", 1, 0, "WaylandManager");
    qmlRegisterType<AppModel>("Eternal.Core", 1, 0, "AppModel");
    qmlRegisterType<AppFilterModel>("Eternal.Core", 1, 0, "AppFilterModel");
    QQmlApplicationEngine engine;
    engine.addImageProvider(QLatin1String("icons"), new IconImageProvider);
    // Завантажуємо наш QML модуль, який ми вказали в CMake
    engine.loadFromModule("Eternal", "Main");

    return app.exec();
}