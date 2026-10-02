#!/usr/bin/env bash
# Build a local Qt Wayland dependency with correct multi-binding keyboard delivery.
set -euo pipefail

repo=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)
build=$(realpath -m -- "${1:-$repo/build}")
version=6.11.1
installed=$(pkg-config --modversion Qt6WaylandCompositor)
if [[ "$installed" != "$version" ]]; then
    echo "This patch is verified for Qt $version; installed Qt is $installed." >&2
    echo "Rebase and test the dependency patch before using a different Qt version." >&2
    exit 1
fi

mkdir -p "$build"
archive="$build/qtwayland-v$version.tar.gz"
source_dir="$build/qtwayland-$version"
qt_build="$build/qtwayland-build"
patch_file="$repo/patches/qtwayland-$version-keyboard-resources.patch"

if [[ ! -f "$archive" ]]; then
    curl -fL --retry 2 "https://github.com/qt/qtwayland/archive/refs/tags/v$version.tar.gz" \
        -o "$archive"
fi
printf '%s  %s\n' 203a0360b3731fd29405203e28bda17a910a900afa1e98d6fc8ec7ff43474ad5 "$archive" | sha256sum -c -
if [[ ! -d "$source_dir" ]]; then
    tar -xzf "$archive" -C "$build"
fi
if patch --dry-run --batch --forward -d "$source_dir" -p1 < "$patch_file" >/dev/null 2>&1; then
    patch --batch --forward -d "$source_dir" -p1 < "$patch_file"
elif ! patch --dry-run --batch --reverse -d "$source_dir" -p1 < "$patch_file" >/dev/null 2>&1; then
    echo "Qt sources differ from both the original and patched versions; inspect $source_dir." >&2
    exit 1
fi

cmake -S "$source_dir" -B "$qt_build" -G Ninja \
    -DCMAKE_BUILD_TYPE=RelWithDebInfo -DQT_BUILD_EXAMPLES=OFF -DQT_BUILD_TESTS=ON \
    -DCMAKE_INSTALL_PREFIX="$build/qtwayland-install"
cmake --build "$qt_build" --target WaylandCompositor tst_compositor --parallel "${CMAKE_BUILD_PARALLEL_LEVEL:-4}"
QT_QPA_PLATFORM=offscreen "$qt_build/tests/auto/compositor/compositor/tst_compositor" \
    multipleKeyboardResources simpleKeyboard keyboardKeymaps keyboardLayoutSwitching keyboardGrab

# Persist the dependency location in the application's build RUNPATH. Subsequent
# `cmake --build build` and `./build/eternal-ui` use this library without an env var.
cmake -S "$repo" -B "$build" -DCMAKE_BUILD_RPATH="$qt_build/lib"
cmake --build "$build" --parallel "${CMAKE_BUILD_PARALLEL_LEVEL:-4}"
echo "Built $build/eternal-ui with the local Qt keyboard fix."
