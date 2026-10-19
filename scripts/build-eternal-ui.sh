#!/usr/bin/env bash
set -Eeuo pipefail

ROOT="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)"
PATCH_DIR="$ROOT/patches"

QT_VERSION="v6.11.2"
QT_SOURCE="$ROOT/build/qtwayland-src"
QT_BUILD="$ROOT/build/qtwayland-build"
QT_PREFIX="$ROOT/build/qtwayland"
APP_BUILD="$ROOT/build"
JOBS="${JOBS:-4}"

PATCHES=(
    "$PATCH_DIR/qtwayland-6.11.1-keyboard-resources.patch"
    "$PATCH_DIR/qtwayland-6.11.2-seat-v5.patch"
    "$PATCH_DIR/qtwayland-6.11.2-data-device-v3.patch"
)

die() {
    echo "ERROR: $*" >&2
    exit 1
}

echo "==> EternalOS build"
echo "    Project: $ROOT"
echo "    QtWayland: $QT_VERSION"
echo "    Jobs: $JOBS"

# Check required patches before doing any work.
for patch in "${PATCHES[@]}"; do
    [[ -f "$patch" ]] || die "Missing patch: $patch"
done

mkdir -p "$ROOT/build"

# Create an isolated source checkout. Never modify the user's existing
# ~/qt6-wayland/qtwayland-6.11.2 checkout.
if [[ ! -d "$QT_SOURCE/.git" ]]; then
    if [[ -e "$QT_SOURCE" ]]; then
        die "$QT_SOURCE exists but is not a Git checkout; move it aside first"
    fi

    echo "==> Cloning QtWayland $QT_VERSION"
    git clone \
        --branch "$QT_VERSION" \
        --depth 1 \
        https://code.qt.io/qt/qtwayland.git \
        "$QT_SOURCE"
fi

ACTUAL_VERSION="$(git -C "$QT_SOURCE" describe --tags --exact-match HEAD 2>/dev/null || true)"
[[ "$ACTUAL_VERSION" == "$QT_VERSION" ]] ||
    die "Expected QtWayland $QT_VERSION at $QT_SOURCE, found '${ACTUAL_VERSION:-unknown}'"

# Apply each patch only if it has not already been applied.
apply_patch_once() {
    local patch="$1"
    local name
    name="$(basename "$patch")"

    if git -C "$QT_SOURCE" apply --reverse --check "$patch" >/dev/null 2>&1; then
        echo "==> Already applied: $name"
    elif git -C "$QT_SOURCE" apply --check "$patch"; then
        echo "==> Applying: $name"
        git -C "$QT_SOURCE" apply "$patch"
    else
        die "Patch is neither cleanly applicable nor already applied: $name"
    fi
}

for patch in "${PATCHES[@]}"; do
    apply_patch_once "$patch"
done

# Build QtWayland privately. Do not install into /usr.
echo "==> Configuring patched QtWayland"
cmake -S "$QT_SOURCE" -B "$QT_BUILD" -G Ninja \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX="$QT_PREFIX"

echo "==> Building patched QtWayland"
cmake --build "$QT_BUILD" --parallel "$JOBS"

echo "==> Installing QtWayland into $QT_PREFIX"
cmake --install "$QT_BUILD"

QT_CONFIG="$QT_PREFIX/lib/cmake/Qt6WaylandCompositor/Qt6WaylandCompositorConfig.cmake"
QT_LIBRARY="$QT_PREFIX/lib/libQt6WaylandCompositor.so.6"

[[ -f "$QT_CONFIG" ]] || die "Private QtWayland CMake config was not installed"
[[ -e "$QT_LIBRARY" ]] || die "Private QtWayland library was not installed"

# Configure EternalOS against the private QtWayland package.
# Keep the existing generator of the project's build directory.
echo "==> Configuring eternal-ui"
cmake -S "$ROOT" -B "$APP_BUILD" \
    -DQt6WaylandCompositor_DIR="$QT_PREFIX/lib/cmake/Qt6WaylandCompositor" \
    -DCMAKE_BUILD_RPATH='$ORIGIN/qtwayland/lib' \
    -DCMAKE_INSTALL_RPATH='$ORIGIN/../lib'

echo "==> Building eternal-ui"
cmake --build "$APP_BUILD" --parallel "$JOBS"

APP="$APP_BUILD/eternal-ui"
[[ -x "$APP" ]] || die "Executable not found: $APP"

# Verify that the executable resolves the private QtWayland library.
echo "==> Verifying runtime library"
LIB_PATH="$(ldd "$APP" | awk '/libQt6WaylandCompositor\.so\.6 =>/ {print $3; exit}')"

[[ -n "$LIB_PATH" && -e "$LIB_PATH" ]] ||
    die "Could not resolve libQt6WaylandCompositor.so.6"

case "$LIB_PATH" in
    "$QT_PREFIX"/lib/*)
        echo "    OK: $LIB_PATH"
        ;;
    *)
        die "eternal-ui is using a non-private QtWayland library: $LIB_PATH"
        ;;
esac

echo
echo "SUCCESS: eternal-ui built with all three QtWayland patches."
echo "Run it with:"
echo "  cd \"$ROOT\" && ./build/eternal-ui"
