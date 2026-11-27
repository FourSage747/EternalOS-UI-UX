#include <QWaylandCompositor>
#include <QDebug>
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QIcon>
#include "AppLauncher.h"
#include "WaylandManager.h"
#include "AppModel.h"
#include "IconImageProvider.h"
#include "AppFilterModel.h"
#include "core/VTManager.h"
#include "XdgActivation.h"
#include "selection/PrimarySelection.h"
#include "textinput/TextInputV1.h"
#include "data/DataDeviceManager.h"
#include "layershell/LayerShell.h"
#include "XdgOutputManager.h"
#include "EonProcessManager.h"

int main(int argc, char *argv[])
{
    // Захоплюємо контроль над віртуальним терміналом ядра
    VTManager vtManager;
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
    qmlRegisterType<XdgActivation>("Eternal.Core", 1, 0, "XdgActivation");
    qmlRegisterType<PrimarySelection>("Eternal.Core", 1, 0, "PrimarySelection");
    qmlRegisterType<TextInputV1>("Eternal.Core", 1, 0, "TextInputV1");
    qmlRegisterType<DataDeviceManager>("Eternal.Core", 1, 0, "DataDeviceManager");
    qmlRegisterType<LayerShell>("Eternal.Core", 1, 0, "LayerShell");
    qmlRegisterType<XdgOutputManager>("Eternal.Core", 1, 0, "XdgOutputManager");

    QQmlApplicationEngine engine;
    engine.addImageProvider(QLatin1String("icons"), new IconImageProvider);

    // Захист від зависання у tty: миттєвий вихід при синтаксичній помилці
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreationFailed,
        &app, []() { QCoreApplication::exit(-1); },
        Qt::QueuedConnection);

    // Динамічне завантаження модуля - тепер воно працюватиме всюди!
    engine.loadFromModule("Eternal", "Main");

    if (engine.rootObjects().isEmpty()) {
        qCritical() << "Failed to load Eternal compositor";
        return -1;
    }

    auto *compositor = qobject_cast<QWaylandCompositor *>(
        engine.rootObjects().constFirst()
    );

    if (!compositor) {
        qCritical() << "QML root object is not a WaylandCompositor";
        return -1;
    }

    EonProcessManager eonProcessManager(&app);
    eonProcessManager.setCompositor(compositor);

    return app.exec();
}