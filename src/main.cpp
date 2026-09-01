#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QIcon>
#include "AppLauncher.h"
#include "WaylandManager.h"
#include "AppModel.h"
#include "IconImageProvider.h"
#include "AppFilterModel.h"

int main(int argc, char *argv[])
{
    qputenv("QT_WAYLAND_CLIENT_BUFFER_INTEGRATION", "wayland-egl");
    qputenv("QT_WAYLAND_HARDWARE_INTEGRATION", "wayland-egl");
    QIcon::setThemeSearchPaths({"/usr/share/icons"});
    QGuiApplication app(argc, argv);
    QIcon::setThemeName("hicolor");
    QCoreApplication::setOrganizationName("EternalOS");
    QCoreApplication::setOrganizationDomain("eternal-os.org");

    qmlRegisterType<AppLauncher>("Eternal.Core", 1, 0, "AppLauncher");
    qmlRegisterType<WaylandManager>("Eternal.Core", 1, 0, "WaylandManager");
    qmlRegisterType<AppModel>("Eternal.Core", 1, 0, "AppModel");
    qmlRegisterType<AppFilterModel>("Eternal.Core", 1, 0, "AppFilterModel");

    QQmlApplicationEngine engine;
    engine.addImageProvider(QLatin1String("icons"), new IconImageProvider);

    // Захист від зависання у tty: миттєвий вихід при синтаксичній помилці
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreationFailed,
        &app, []() { QCoreApplication::exit(-1); },
        Qt::QueuedConnection);

    // Динамічне завантаження модуля - тепер воно працюватиме всюди!
    engine.loadFromModule("Eternal", "Main");

    return app.exec();
}