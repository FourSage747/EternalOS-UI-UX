cmake_minimum_required(VERSION 3.16)
project(eternal-ui)

# EON Desktop
include(FetchContent)

find_package(Git REQUIRED)
find_program(CARGO_EXECUTABLE cargo REQUIRED)

# GitHub URL репозиторію EON
set(EON_GIT_REPOSITORY "https://github.com/Alkansius/EON-launcher")

FetchContent_Declare(
    eon
    GIT_REPOSITORY "${EON_GIT_REPOSITORY}"
    GIT_TAG main
    GIT_SHALLOW TRUE
)

FetchContent_GetProperties(eon)
if(NOT eon_POPULATED)
    FetchContent_Populate(eon)
endif()

set(EON_CARGO_TARGET_DIR "${CMAKE_BINARY_DIR}/eon-target")

add_custom_target(eon-shell-build
    COMMAND
        ${CMAKE_COMMAND} -E env
        "CARGO_TARGET_DIR=${EON_CARGO_TARGET_DIR}"
        "${CARGO_EXECUTABLE}" build
        --release
        --package eon-shell
        --manifest-path "${eon_SOURCE_DIR}/Cargo.toml"
    WORKING_DIRECTORY "${eon_SOURCE_DIR}"
    COMMENT "Building EON Desktop Shell"
    VERBATIM
)

# Використовуємо сучасний стандарт C++
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Умикаємо AUTOMOC (це виправить головну помилку)
set(CMAKE_AUTOMOC ON)

# Prefer project-local patched QtWayland during development.
set(LOCAL_QTWAYLAND_DIR
    "${CMAKE_CURRENT_SOURCE_DIR}/build/qtwayland/lib/cmake/Qt6WaylandCompositor"
)

if(EXISTS "${LOCAL_QTWAYLAND_DIR}/Qt6WaylandCompositorConfig.cmake")
    set(Qt6WaylandCompositor_DIR
        "${LOCAL_QTWAYLAND_DIR}"
        CACHE PATH "Patched QtWayland Compositor package" FORCE
    )
else()
    unset(Qt6WaylandCompositor_DIR CACHE)
    message(STATUS "Using system-installed QtWayland Compositor")
endif()

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
    src/selection/PrimarySelection.cpp
    src/selection/PrimarySelection.h
    src/data/DataDeviceManager.cpp
    src/data/DataDeviceManager.h
    src/textinput/TextInputV1.cpp
    src/textinput/TextInputV1.h
    src/layershell/LayerShell.cpp
    src/layershell/LayerShell.h
    src/XdgOutputManager.cpp
    src/XdgOutputManager.h
    src/EonProcessManager.cpp
    src/EonProcessManager.h
)

add_dependencies(eternal-ui eon-shell-build)

qt_generate_wayland_protocol_server_sources(eternal-ui
    FILES
       ${CMAKE_CURRENT_SOURCE_DIR}/protocols/xdg-activation-v1.xml
       ${CMAKE_CURRENT_SOURCE_DIR}/protocols/primary-selection-unstable-v1.xml
       ${CMAKE_CURRENT_SOURCE_DIR}/protocols/text-input-unstable-v1.xml
       ${CMAKE_CURRENT_SOURCE_DIR}/protocols/wayland.xml
       ${CMAKE_CURRENT_SOURCE_DIR}/protocols/wlr-layer-shell-unstable-v1.xml
       ${CMAKE_CURRENT_SOURCE_DIR}/protocols/xdg-shell.xml
       ${CMAKE_CURRENT_SOURCE_DIR}/protocols/xdg-output-unstable-v1.xml
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
        qml/window/TitleBar.qml
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

# Find the private QtWayland library at runtime, without LD_LIBRARY_PATH.
set_target_properties(eternal-ui PROPERTIES
    BUILD_RPATH "$ORIGIN/qtwayland/lib"
    INSTALL_RPATH "$ORIGIN/../lib"
)

install(TARGETS eternal-ui DESTINATION bin)
install(
    PROGRAMS "${EON_CARGO_TARGET_DIR}/release/eon-shell"
    DESTINATION lib/eternal-ui
)