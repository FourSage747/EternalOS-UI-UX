cmake_minimum_required(VERSION 3.16)
project(eternal-ui)

# Використовуємо сучасний стандарт C++
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Умикаємо AUTOMOC (це виправить головну помилку)
set(CMAKE_AUTOMOC ON)

# Встановлюємо нову політику Qt для QML (це прибере попередження)
if(COMMAND qt_policy)
    qt_policy(SET QTP0001 NEW)
endif()

# Шукаємо необхідні модулі Qt6
find_package(Qt6 REQUIRED COMPONENTS Core Gui Qml Quick WaylandCompositor)

# Створюємо виконуваний файл
qt_add_executable(eternal-ui
    src/main.cpp
    src/AppLauncher.cpp
    src/WaylandManager.cpp
    src/AppModel.cpp
    src/IconImageProvider.h
    src/AppFilterModel.cpp
)

# Підключаємо QML файли як модуль
qt_add_qml_module(eternal-ui
    URI Eternal
    VERSION 1.0
    QML_FILES 
        qml/Main.qml
        qml/Taskbar.qml
        qml/Launcher.qml
        qml/appmodel/AppModel.qml
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