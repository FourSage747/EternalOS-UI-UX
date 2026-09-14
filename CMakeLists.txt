cmake_minimum_required(VERSION 3.16)
project(eternal-ui)

# Використовуємо сучасний стандарт C++
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Умикаємо AUTOMOC (це виправить головну помилку)
set(CMAKE_AUTOMOC ON)

# 1. Спочатку завантажуємо модулі Qt6 (щоб CMake дізнався про команду qt_policy)
find_package(Qt6 REQUIRED COMPONENTS Core Gui Qml Quick WaylandCompositor WaylandScannerTools)

# 2. Встановлюємо нові політики Qt для правильного пакування QML
qt_policy(SET QTP0001 NEW)
qt_policy(SET QTP0004 NEW)

# Створюємо виконуваний файл
qt_add_executable(eternal-ui
    src/main.cpp
    src/AppLauncher.cpp
    src/WaylandManager.cpp
    src/AppModel.cpp
    src/IconImageProvider.h
    src/AppFilterModel.cpp
    src/core/VTManager.cpp
    src/XdgActivation.cpp
    src/XdgActivation.h
)

qt_generate_wayland_protocol_server_sources(eternal-ui
    FILES
       ${CMAKE_CURRENT_SOURCE_DIR}/protocols/xdg-activation-v1.xml
)
# Підключаємо QML файли як модуль
qt_add_qml_module(eternal-ui
    URI Eternal
    VERSION 1.0
    QML_FILES 
        qml/Main.qml
        qml/Taskbar.qml
        qml/Launcher.qml
        qml/appmodel/RunningAppsModel.qml
        qml/background/DesktopBackground.qml
        qml/window/AppXdgShell.qml
        qml/window/WaylandWindow.qml
        qml/window/WindowManager.qml
)

qt_add_resources(eternal-ui "assets"
    PREFIX "/"
    FILES 
        res/images/background.png
        res/images/logo.png
)

# Лінкуємо бібліотеки
target_link_libraries(eternal-ui PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Qml
    Qt6::Quick
    Qt6::WaylandCompositor
)

install(TARGETS eternal-ui DESTINATION bin)