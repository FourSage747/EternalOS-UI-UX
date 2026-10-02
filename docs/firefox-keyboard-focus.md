# Firefox keyboard input: investigation and fix

Verified on 2026-09-25 with Qt Wayland Compositor 6.11.1 and Firefox 153.0.

## Root cause

The first failing transition is **Qt keyboard focus → delivery to the client's
keyboard resources**, inside `QWaylandKeyboardPrivate::focused()` in
`qtwayland/src/compositor/compositor_api/qwaylandkeyboard.cpp`.

Firefox binds the same seat twice: GTK owns one `wl_keyboard`, and Mozilla's
`nsWaylandDisplay` owns another. Qt selected just one resource with
`resourceMap().value(surface->waylandClient())`, retained it in `focusResource`,
and sent enter, leave, keys, and modifiers only to that resource. In the failing
traces, this was Mozilla's later binding. GTK's keyboard never acquired focus.

Mozilla's extra listener updates serial/repeat bookkeeping. GTK's listener
sets `keyboard_focus`, emits the focus-in event, and delivers key events to the
window. Receiving keys on Mozilla's extra binding does not replace GTK input
delivery. This explains why a single-binding application can work and Firefox
can receive protocol key events without accepting typed text.

Sources inspected:

- [Qt 6.11.1 keyboard implementation](https://github.com/qt/qtwayland/blob/v6.11.1/src/compositor/compositor_api/qwaylandkeyboard.cpp)
- [Qt 6.11.1 seat implementation](https://github.com/qt/qtwayland/blob/v6.11.1/src/compositor/compositor_api/qwaylandseat.cpp)
- [Qt 6.11.1 Quick item implementation](https://github.com/qt/qtwayland/blob/v6.11.1/src/compositor/compositor_api/qwaylandquickitem.cpp)
- [Qt 6.11.1 XDG shell implementation](https://github.com/qt/qtwayland/blob/v6.11.1/src/compositor/extensions/qwaylandxdgshell.cpp)
- [Mozilla keyboard listeners](https://github.com/mozilla-firefox/firefox/blob/main/widget/gtk/nsWaylandDisplay.cpp)
- [GTK keyboard listeners](https://github.com/GNOME/gtk/blob/gtk-3-24/gdk/wayland/gdkdevice-wayland.c)

The supplied `firefox-*-wayland.log` names were absent. The existing
`/tmp/firefox-hyprland-full.log`, `/tmp/firefox-eternal-full.log`, and
`/tmp/firefox-input-test.log` contained the relevant evidence:

| Trace | Enter and key recipients |
| --- | --- |
| Hyprland main browser | Both `wl_keyboard#31` and `#54` |
| Eternal input test main browser | Only `wl_keyboard#29`; GTK's `#16` received its keymap but no enter or keys |

Hyprland also sends keyboard enter before its subsequent activated configure.
Keyboard enter preceding pointer enter is therefore insufficient to identify a
bug. A controlled Eternal test with focus assigned **by a click after mapping**
still failed with system Qt. Changing only the Qt library made the same test
accept input. This isolates the resource-delivery bug from startup ordering.

## Complete focus path

1. `AppXdgShell.onToplevelCreated` creates a `WaylandWindow` with the actual
   `QWaylandXdgSurface`. `ShellSurfaceItem` uses that XDG surface's `wl_surface`,
   rather than Firefox's rendering subsurface.
2. Creating the item no longer assigns focus. The first `hasContent` transition
   after a buffer commit invokes `WaylandWindow.activate("mapped")`.
3. `takeFocus()` sets Qt Quick active focus and the seat's keyboard surface.
   Qt's `updateFocus()` can call the seat as well. The seat returns immediately
   when the requested surface already has focus, so this produces one transition.
4. `QWaylandSeat::setKeyboardFocus()` rejects destroyed surfaces, updates the
   keyboard and data-device focus, and emits `keyboardFocusChanged`. It has no
   mapping/activation prerequisite and does not choose or focus a QML item.
5. `QWaylandKeyboard::setFocus()` sends leave for the old surface and enter for
   the new surface. The patch delivers these events to **all resources of the
   corresponding client**. Other clients receive no keys.
6. Qt's XDG shell handles `keyboardFocusChanged` and schedules the activated
   configure. `toplevel.activated` reflects acknowledged configure state; it
   can still be false in the synchronous seat-change diagnostic.
7. `XdgActivation::xdg_activation_v1_activate()` emits `activationRequested`.
   `WindowManager.activateSurface()` matches the exact surface and activates
   its `ShellSurfaceItem`. Requests before mapping are fulfilled when content
   arrives. Later requests synchronize QML focus, raising, and seat focus.
8. Qt's normal `focusOnClick` path invokes `takeFocus()` before the pointer
   button event. Key presses go through the active Quick item to the seat.
   `QWaylandView` supplies pointer coordinates and rendering context; keyboard
   focus is a `QWaylandSurface`.

At the start of this investigation, click focus, creation focus, taskbar focus,
and the direct activation focus call were all disabled in the working tree.
The patch restores click/taskbar activation and replaces the direct seat-only
activation path with item activation. It does not add `focus: true` or focus
calls during component creation.

## Patch and build

The exact Qt implementation and regression-test changes are in
[`qtwayland-6.11.1-keyboard-resources.patch`](../patches/qtwayland-6.11.1-keyboard-resources.patch).
They cover focus, keys, every modifier delivery path, late binding, and the
focused-client lookup after a keyboard resource is destroyed. Qt's private
object layout is preserved.

Run from the repository:

```sh
./scripts/build-keyboard-fix.sh
./build/eternal-ui
```

The script verifies the source archive checksum, applies the version-specific
patch, builds the local Qt compositor library, runs the keyboard tests, and
builds EternalOS with a RUNPATH to `build/qtwayland-build/lib`. Subsequent
`cmake --build build -j4` builds retain that path. `ldd build/eternal-ui` was
checked and resolves `libQt6WaylandCompositor.so.6` to this local library.

This is a local build dependency. An installed/distributed EternalOS package
must ship the patched Qt library or apply this patch to its Qt package. The
script deliberately requires the matching Qt 6.11.1 development installation;
the patch must be rebased and tested when Qt changes.

## Runtime evidence

Tests used native Wayland Firefox with a separate profile on an isolated
`eternal-0` socket. EternalOS ran nested through Qt's XCB backend on the host's
Xwayland. Keys and clicks were sent to the compositor using `xdotool`;
Marionette only read browser/DOM state and loaded the local HTML fixture.

| Test | Observed result |
| --- | --- |
| System Qt; post-map click, Ctrl+L, type `eternalbefore` | Address bar remained empty; only `wl_keyboard#32` received keys |
| Same compositor binary, patched Qt; repeat interaction | Address bar contained `eternalafter`; both `#32` and `#18` received keys |
| Final compositor; address bar | `addressbar-ok` |
| Ordinary HTML field; typing then Ctrl+A and replacement | `replacement-ok`, 24 DOM input events |
| Ctrl+T, then Ctrl+W | Tab count changed 1 → 2 → 1 |
| HTML button click | Button text became `mouse-ok` |
| Two Firefox windows, taskbar switch | Address bars retained `first-window` and `second-window` respectively |
| Final executable without a library environment override | Address bar contained `packaged-ok` |

Final protocol sequence, from `/tmp/eternal-focus-test/final-firefox.log`:

```text
349223.076  wl_surface#43.attach(wl_buffer#58, 0, 0)
349223.231  wl_surface#43.commit()
349235.704  wl_keyboard#32.enter(4, wl_surface#43, array[0])
349235.742  wl_keyboard#18.enter(5, wl_surface#43, array[0])
349364.631  xdg_activation_v1#28.activate(..., wl_surface#43)
```

That activation request did not generate another enter. Creating a second
window produced leave on both bindings for `wl_surface#43`, then enter on both
for `wl_surface#60`. Taskbar switching produced the reverse sequence.

Firefox's final internal log, `packaged-widget.log.moz_log`, contains:

```text
nsWindowWayland::TransferFocusTo() gFocusWindow 0
RequestWaylandFocusPromise() no source window, requesting bare token for workspace placement
RequestWaylandFocusPromise() XDG Token sent
OnContainerFocusInEvent
nsWindowWayland::FocusWaylandWindow(...)
OnKeyPressEvent
OnKeyReleaseEvent
```

Thus an initial `gFocusWindow 0` also occurs in the working startup sequence.
The successful focus-in and key callbacks establish that GTK/Firefox now
consume the input.

Saved evidence is under `/tmp/eternal-focus-test/`: `before-firefox.log`,
`qtfix-only-firefox.log`, `final-firefox.log`, `final-focus-journal.log`,
`packaged-compositor.log`, and `packaged-widget.log.moz_log`.
The nested test does not validate a direct DRM/TTY session.

The new Qt test fails on the original system library with
`first->m_enteredSurface == nullptr`. With the patch, the multi-binding test and
the existing simple keyboard, keymap, layout-switching, and grab tests pass:
**7 passed, 0 failed**, including initialization and cleanup. Logs are in
`build/qtwayland-unpatched-regression.log` and `build/keyboard-fix-build.log`.

## Capture another trace

```sh
ETERNAL_FOCUS_DEBUG=1 QT_FORCE_STDERR_LOGGING=1 WAYLAND_DEBUG=server \
  ./build/eternal-ui 2>/tmp/eternal-focus.log
```

In another terminal, using a separate Firefox profile:

```sh
mkdir -p /tmp/eternal-firefox-profile
WAYLAND_DISPLAY=eternal-0 MOZ_ENABLE_WAYLAND=1 WAYLAND_DEBUG=1 \
  MOZ_LOG=Widget:5 MOZ_LOG_FILE=/tmp/eternal-firefox-widget.log \
  firefox --no-remote --profile /tmp/eternal-firefox-profile about:blank \
  2>/tmp/eternal-firefox-wayland.log
```

Focus diagnostics include epoch milliseconds, request reason, surface address
and protocol ID, client PID, content/mapping state, app ID, title, acknowledged
activation state, previous/current seat focus, and QML focus/activeFocus.
`QT_FORCE_STDERR_LOGGING=1` keeps Qt's diagnostics in the redirected trace;
otherwise this host sends them to the journal.
