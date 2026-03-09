cmake_minimum_required(VERSION 3.20)

project(ptz-tracking-dock VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

option(PTZ_TRACKING_DOCK_DYNAMIC_LOOKUP "Build without OBS stubs and resolve OBS symbols at runtime (macOS/local dev fallback)" OFF)

# NOTE:
# The most reliable way to build OBS plugins is inside the OBS Studio source tree.
# This CMakeLists is a scaffold; OBS's build system may provide helper macros/targets.

add_library(ptz-tracking-dock MODULE
  src/plugin-main.cpp
  src/ptz_config.cpp
  src/ptz_http.cpp
  src/ptz_settings_dialog.cpp
  src/ptz_dock.cpp
)

target_include_directories(ptz-tracking-dock PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# OBS headers (32.x) require simde headers for SSE intrinsics.
find_path(SIMDE_INCLUDE_DIR simde/x86/sse2.h
  PATHS
    /opt/homebrew/opt/simde/include
    /opt/homebrew/include
)
if(SIMDE_INCLUDE_DIR)
  target_include_directories(ptz-tracking-dock PRIVATE ${SIMDE_INCLUDE_DIR})
endif()

target_compile_definitions(ptz-tracking-dock PRIVATE
  PTZ_TRACKING_DOCK_VERSION=\"${PROJECT_VERSION}\"
  QT_NO_VERSION_TAGGING
)

# Try Qt6 first, then Qt5.
find_package(Qt6 COMPONENTS Widgets Network QUIET)
if(Qt6_FOUND)
  target_link_libraries(ptz-tracking-dock PRIVATE Qt6::Widgets Qt6::Network)
  target_compile_definitions(ptz-tracking-dock PRIVATE PTZ_TRACKING_DOCK_QT6=1)
else()
  find_package(Qt5 COMPONENTS Widgets Network REQUIRED)
  target_link_libraries(ptz-tracking-dock PRIVATE Qt5::Widgets Qt5::Network)
endif()

# OBS headers/libs are normally provided by OBS build.
# If we are being built from inside OBS Studio, these targets usually exist.
if(EXISTS "${CMAKE_SOURCE_DIR}/frontend/api")
  target_include_directories(ptz-tracking-dock PRIVATE "${CMAKE_SOURCE_DIR}/frontend/api")
elseif(EXISTS "${CMAKE_SOURCE_DIR}/UI/obs-frontend-api")
  # Fallback for older/alternate OBS source layouts.
  target_include_directories(ptz-tracking-dock PRIVATE "${CMAKE_SOURCE_DIR}/UI/obs-frontend-api")
endif()

if(TARGET libobs)
  target_link_libraries(ptz-tracking-dock PRIVATE libobs)
endif()
if(TARGET obs-frontend-api)
  target_link_libraries(ptz-tracking-dock PRIVATE obs-frontend-api)
elseif(TARGET OBS::frontend-api)
  target_link_libraries(ptz-tracking-dock PRIVATE OBS::frontend-api)
elseif(EXISTS "${CMAKE_SOURCE_DIR}/frontend/api/CMakeLists.txt")
  # In OBS superbuilds, this plugin may be configured before frontend/api is added.
  # Link by library name so the final link still resolves against obs-frontend-api.
  target_link_libraries(ptz-tracking-dock PRIVATE obs-frontend-api)
endif()

if(NOT TARGET libobs AND NOT TARGET obs-frontend-api)
  if(PTZ_TRACKING_DOCK_DYNAMIC_LOOKUP)
    message(WARNING "Building with runtime OBS symbol lookup (no stubs linked).")
    target_include_directories(ptz-tracking-dock PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src/obs_stubs)
    if(APPLE)
      target_link_options(ptz-tracking-dock PRIVATE "-Wl,-undefined,dynamic_lookup")
    else()
      # Non-Apple fallback: provide stub symbols so CI test builds can link.
      target_sources(ptz-tracking-dock PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src/obs_stubs/obs_stubs.cpp)
    endif()
  else()
    message(FATAL_ERROR "OBS targets not found. Build inside OBS source tree or enable PTZ_TRACKING_DOCK_DYNAMIC_LOOKUP.")
  endif()
endif()

# Output name
set_target_properties(ptz-tracking-dock PROPERTIES
  OUTPUT_NAME "ptz-tracking-dock"
  PREFIX ""
)

if(APPLE)
  # OBS on macOS uses bundle-style plugins with a .plugin extension.
  set(PTZ_DOCK_BUNDLE_ID "com.ptz.ptz-tracking-dock" CACHE STRING "CFBundleIdentifier for the macOS .plugin")
  configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/Info.plist.in
    ${CMAKE_CURRENT_BINARY_DIR}/Info.plist
    @ONLY
  )

  set_target_properties(ptz-tracking-dock PROPERTIES
    BUNDLE TRUE
    BUNDLE_EXTENSION "plugin"
    MACOSX_BUNDLE TRUE
    MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_BINARY_DIR}/Info.plist"
    MACOSX_BUNDLE_GUI_IDENTIFIER "${PTZ_DOCK_BUNDLE_ID}"
    MACOSX_BUNDLE_BUNDLE_NAME "ptz-tracking-dock"
    MACOSX_BUNDLE_SHORT_VERSION_STRING "${PROJECT_VERSION}"
    MACOSX_BUNDLE_BUNDLE_VERSION "${PROJECT_VERSION}"
  )

  # Ensure we resolve Qt from the running OBS.app instead of Homebrew Qt.
  # OBS's Qt frameworks live at @executable_path/../Frameworks.
  add_custom_command(TARGET ptz-tracking-dock POST_BUILD
    COMMAND /bin/sh -c "\
      bin=\"$<TARGET_FILE:ptz-tracking-dock>\"; \
      /usr/bin/install_name_tool -delete_rpath /opt/homebrew/opt/qt/lib \"$bin\" 2>/dev/null || true; \
      /usr/bin/install_name_tool -delete_rpath /usr/local/opt/qt/lib \"$bin\" 2>/dev/null || true; \
      /usr/bin/install_name_tool -add_rpath @executable_path/../Frameworks \"$bin\" 2>/dev/null || true; \
      for prefix in /opt/homebrew/opt/qtbase/lib /usr/local/opt/qtbase/lib; do \
        /usr/bin/install_name_tool -change $prefix/QtCore.framework/Versions/A/QtCore @rpath/QtCore.framework/Versions/A/QtCore \"$bin\" 2>/dev/null || true; \
        /usr/bin/install_name_tool -change $prefix/QtGui.framework/Versions/A/QtGui @rpath/QtGui.framework/Versions/A/QtGui \"$bin\" 2>/dev/null || true; \
        /usr/bin/install_name_tool -change $prefix/QtWidgets.framework/Versions/A/QtWidgets @rpath/QtWidgets.framework/Versions/A/QtWidgets \"$bin\" 2>/dev/null || true; \
        /usr/bin/install_name_tool -change $prefix/QtNetwork.framework/Versions/A/QtNetwork @rpath/QtNetwork.framework/Versions/A/QtNetwork \"$bin\" 2>/dev/null || true; \
      done"
    VERBATIM
  )
endif()
