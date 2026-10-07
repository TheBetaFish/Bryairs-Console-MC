# Bryair's Console Minecraft — Web Edition

A **WebAssembly/Emscripten browser port** of Minecraft: Legacy Console Edition based on the Portable-LCE project.

This branch (`emscripten-web-port`) is specifically focused on running the game in a modern web browser. It is **not the native desktop build**.

## Play in a browser

The project is designed to be published as a static **GitHub Pages** site.

Once the WebAssembly workflow finishes successfully, GitHub Pages receives:

- `index.html` — browser shell
- `Minecraft.Client.html.js` — Emscripten JavaScript runtime
- `Minecraft.Client.html.wasm` — compiled WebAssembly game
- `Minecraft.Client.html.data` — packaged game resources
- browser runtime/service-worker files

The WebAssembly build and Pages deployment are handled automatically by `.github/workflows/web.yml`.

The branch is therefore intended to be usable as a **publishable web game**, rather than requiring users to install a compiler or native dependencies.

## Browser features

The web shell currently provides:

- Keyboard and mouse input
- Mouse-look support
- Mouse buttons and scrolling
- Browser gamepad detection
- Fullscreen mode
- Adjustable render scale (50–100%)
- FPS and browser diagnostics
- WebGL renderer diagnostics
- Persistent render-scale setting
- Progressive web app manifest
- Cross-origin isolation support for WebAssembly pthreads
- Responsive canvas sizing

**Touch controls are intentionally not included yet.**

## Controls

The game uses the existing Minecraft Console Edition input system with browser keyboard/mouse support.

| Input | Use |
| --- | --- |
| **W A S D** | Movement |
| **Mouse** | Look |
| **Left mouse** | Primary mouse action |
| **Right mouse** | Secondary mouse action |
| **Mouse wheel** | Scroll / inventory selection |
| **Space** | Jump |
| **Shift** | Sneak |
| **E** | Inventory |
| **Esc** | Pause / menu |

A compatible game controller can also be detected through the browser's Gamepad API.

## Building the Web Edition

You do **not** need a native GCC/Clang toolchain for this branch.

### Requirements

- Linux environment
- Python 3
- Meson
- Ninja
- Emscripten SDK

The GitHub Actions workflow installs Meson/Ninja and configures the Emscripten SDK automatically.

### Manual build

Install Meson and Ninja:

```bash
python3 -m pip install --upgrade meson ninja
```

Activate Emscripten, then configure:

```bash
meson setup build \
  --cross-file scripts/emscripten_native.txt \
  -Dbuildtype=release \
  -Dunity=off \
  -Drenderer=gles \
  -Dui_backend=java \
  -Denable_vsync=false \
  -Denable_mimalloc=disabled
```

Build the browser target:

```bash
meson compile -C build -j2 targets/app/Minecraft.Client.html
```

The important outputs are:

```text
build/targets/app/Minecraft.Client.html.js
build/targets/app/Minecraft.Client.html.wasm
build/targets/app/Minecraft.Client.html.data
```

The included `web/` files are used to assemble the final browser site.

## GitHub Pages deployment

Pushing to `emscripten-web-port` automatically starts the WebAssembly workflow.

The workflow:

1. Builds the Emscripten target.
2. Creates the browser `index.html`.
3. Copies the WebAssembly and asset files into a Pages site.
4. Uploads the site as a Pages artifact.
5. Deploys it to GitHub Pages.

The source branch stays separate from the native build so this branch can concentrate on browser compatibility.

## Performance and memory

Browser memory usage is a major focus of this port.

The web build already avoids packaging several unnecessary native/platform asset sets and uses the 720p UI resources for the initial media archive.

Large audio packages are also **not loaded into the initial browser package**, reducing startup memory pressure. Audio can be moved to a lazy-loaded system later rather than forcing the entire sound library into the initial WebAssembly load.

The goal is to make the game practical on lower-memory phones and other browsers, not just desktop machines.

## Project structure

```text
web/                            Browser shell and web-only runtime
scripts/emscripten_native.txt   Meson Emscripten cross file
.github/workflows/web.yml       WebAssembly + GitHub Pages CI
targets/app/                    Game application
targets/platform/               Browser-compatible platform/input/renderer code
targets/resources/              Game resources and media packaging
```

## Technical notes

This port uses:

- **Emscripten/WebAssembly**
- **SDL2**
- **OpenGL ES/WebGL**
- **Meson + Ninja**
- **Web Workers / pthread support**
- A service worker to provide the cross-origin isolation headers needed by browser threading

The renderer targets WebGL 2 through the GLES path.

## Credits

This project is derived from **Portable-LCE** and the Minecraft: Legacy Console Edition code preserved by that project.

Minecraft is a trademark of Microsoft/Mojang. This repository is an independent technical project and is not affiliated with Microsoft or Mojang.
