# ptz-tracking-dock (OBS native plugin)

Native OBS Studio frontend plugin that adds a dock panel for controlling PTZ autotracking via HTTP CGI, plus global OBS hotkeys.

## Features (v0 scaffold)

- Dock panel inside OBS:
  - Camera list (Name + IP)
  - Per-camera **ON** / **OFF** buttons
  - **ALL OFF**
  - Bottom status line
  - Settings dialog to add/remove/reorder cameras
- Hotkeys (true OBS hotkeys, not browser-focused):
  - Camera 1..9 ON
  - Camera 1..9 OFF
  - ALL OFF
- Sends HTTP GET requests to camera CGI endpoints (Qt Network).

## How it stores cameras

A small JSON file in the module config folder (via `obs_module_config_path`), containing an array of cameras `{ name, host }`.

## Build prerequisites

This repository is a **plugin source scaffold**. You typically build it **against the OBS Studio source tree** (recommended), because that guarantees the right headers, frontend API, Qt version, and build flags.

You will need:

- OBS Studio source (matching your OBS version)
- CMake (3.20+)
- C++ compiler (Visual Studio on Windows; Xcode/clang on macOS)
- Qt (OBS uses Qt; version depends on your OBS branch)

### OBS 32.0.2 notes

- OBS 32.x uses **Qt 6**.
- Build this plugin against the **OBS 32.0.2 source tree/tag** to avoid ABI/Qt mismatches.

## Build (recommended: inside OBS source tree)

1. Clone OBS Studio.
2. Checkout the OBS 32.0.2 tag (or the exact commit you’re using to build OBS 32.0.2).
3. Copy this folder into the OBS source tree at: `obs-studio/plugins/ptz-tracking-dock`
4. Add it to the OBS plugins build:
  - Edit `obs-studio/plugins/CMakeLists.txt` and add:
    - `add_subdirectory(ptz-tracking-dock)`
5. Configure/build OBS as usual.

## Output artifacts

- macOS: the build produces a bundle named `ptz-tracking-dock.plugin` (a folder bundle).
- Windows: the build produces `ptz-tracking-dock.dll`.

Exact output location depends on how you build OBS (and generator), but you can search the build folder for:

- `ptz-tracking-dock.plugin`
- `ptz-tracking-dock.dll`

## Install (manual)

Because OBS plugin install paths vary by build/distribution and CPU arch, the safest approach is:

- Build OBS + plugin
- Find the built plugin artifact in your build output
- Copy it into OBS’s plugin folder for your install

Typical locations:

- macOS (OBS app bundle): `OBS.app/Contents/PlugIns/` (bundle plugins)
- Windows (64-bit OBS): `obs-plugins/64bit/`

If you’re building OBS from source, OBS’s build system often already stages plugins into the right runtime folder for running OBS from the build tree.

## Notes

- Endpoint used:
  - `http://<host>/cgi-bin/param.cgi?set_overlay&autotracking&on`
  - `http://<host>/cgi-bin/param.cgi?set_overlay&autotracking&off`
- This plugin does not rely on browser CORS behavior; it uses native HTTP.

## Next steps

- Confirm your camera’s exact endpoint/params (Samtav models can vary).
- Add optional live-status polling if your cameras provide a readable status endpoint.
