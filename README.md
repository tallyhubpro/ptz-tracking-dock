# ptz-tracking-dock

`ptz-tracking-dock` is a native OBS Studio plugin that adds a compact PTZ tracking control dock and true OBS frontend hotkeys for per-camera `ON`/`OFF` and global `ALL OFF` actions.

It sends tracking toggle commands directly from OBS using Qt Network HTTP requests to camera CGI endpoints, avoiding browser source CORS limitations and keeping live control fast and reliable.

This repository also includes an HTML-based alternative UI at `PTZ Camera Tracking OBS.html` for users who want a browser-driven control surface.

Keywords: OBS PTZ plugin, OBS camera tracking toggle, PTZ autotracking OBS, OBS hotkeys for PTZ, Samtav PTZ CGI.

## Why This Plugin

- Control PTZ tracking directly in OBS, not in a browser tab.
- Use true OBS frontend hotkeys for camera tracking ON/OFF.
- Keep a lightweight workflow focused on one job: tracking toggle reliability.

## Features

- OBS dock panel for PTZ tracking control.
- Multiple camera entries (name + host/IP).
- Per-camera tracking `ON` / `OFF` toggle buttons.
- Global `ALL OFF` button.
- Global OBS hotkeys:
  - Camera 1..9 tracking ON
  - Camera 1..9 tracking OFF
  - ALL OFF
- Responsive dock layout for small OBS dock sizes.
- Camera settings dialog (add, remove, reorder cameras).
- Native HTTP status reporting in dock status bar.
- Confirmation-aware response parsing (when camera response includes track state text).

## Supported Control Method

This plugin currently controls tracking using vendor HTTP CGI endpoints.

Commands used:

- `http://<host>/cgi-bin/param.cgi?set_overlay&autotracking&on`
- `http://<host>/cgi-bin/param.cgi?set_overlay&autotracking&off`

Note:

- ONVIF/VISCA are not used in this plugin right now.
- AI tracking is typically vendor-specific and not standardized across ONVIF/VISCA.

## Installation

## Option 1: Download Release Artifacts (recommended)

Open Releases:

- `https://github.com/tallyhubpro/ptz-tracking-dock/releases`

Assets:

- `ptz-tracking-dock-macos.zip`
- `ptz-tracking-dock-windows.zip`

### macOS install

1. Download `ptz-tracking-dock-macos.zip`.
2. Extract `ptz-tracking-dock.plugin`.
3. Copy to one of these locations:
   - User plugins: `~/Library/Application Support/obs-studio/plugins/`
   - App bundle plugins: `/Applications/OBS.app/Contents/PlugIns/`
4. Restart OBS.

### Windows install

1. Download `ptz-tracking-dock-windows.zip`.
2. Extract contents.
3. Copy `obs-plugins/64bit/ptz-tracking-dock.dll` into your OBS plugin path.
4. Restart OBS.

## Option 2: Build From Source

### Prerequisites

- CMake `3.20+`
- C++17 toolchain
- Qt (OBS-compatible version)
- OBS Studio source tree (recommended for production builds)

### Recommended build flow

1. Clone OBS Studio (matching your OBS version).
2. Place this project in `obs-studio/plugins/ptz-tracking-dock`.
3. Add to `obs-studio/plugins/CMakeLists.txt`:
   - `add_subdirectory(ptz-tracking-dock)`
4. Build OBS normally.

## HTML Alternative (Browser UI)

If you do not want to install a native plugin, you can use:

- `PTZ Camera Tracking OBS.html`

What it provides:

- Compact PTZ control UI with per-camera `ON/OFF`, `ALL OFF`, and camera settings.
- Local camera list persistence via browser `localStorage`.

Important differences from the native plugin:

- No true OBS frontend hotkeys (browser key handling only when the page is focused).
- Subject to browser/network restrictions depending on your OBS Browser Source and camera CORS behavior.

## Usage

1. Open OBS.
2. Open the `PTZ Tracking` dock.
3. Click the settings icon and add your camera host/IP entries.
4. Toggle `ON` or `OFF` per camera.
5. Use `ALL OFF` when needed.
6. Optionally bind OBS hotkeys for hands-free operation.

## Status Bar Messages

Dock status messages indicate request progress and result:

- `Status: Sending: ...`
- `OK: Confirmed: ...` when response text confirms tracking state.
- `Status: HTTP 200 (unconfirmed): ...` when request succeeded but response did not explicitly confirm state.
- `ERR: ...` for timeout, network, or HTTP errors.

## Configuration Storage

Camera entries are stored as JSON using OBS module config storage via `obs_module_config_path`.

Stored schema:

- `[{ "name": "Camera A", "host": "192.168.0.100" }]`

## CI and Release Automation

GitHub Actions automatically builds artifacts on push/tag:

- Windows zip artifact
- macOS zip artifact
- Both are built against OBS Studio `31.1.2` source (real `libobs`/`obs-frontend-api`, not dev stubs)
- On tags, both assets are attached to the GitHub Release automatically.

Workflow file:

- `.github/workflows/windows-artifact.yml`

## Troubleshooting

- Plugin not loading in OBS:
  - Confirm plugin was built for your OBS/Qt compatibility.
  - Check OBS log file in `~/Library/Application Support/obs-studio/logs/` (macOS) or `%APPDATA%/obs-studio/logs/` (Windows).
- Request says success but camera does not track:
  - Verify your camera supports this CGI endpoint.
  - Confirm auth/session requirements for your model.
  - Test endpoint in browser/curl using the same host.

## Roadmap

- Optional preset action chaining (for example: tracking ON + preset recall).
- Optional readback endpoint support per camera model.
- Additional vendor endpoint templates.
