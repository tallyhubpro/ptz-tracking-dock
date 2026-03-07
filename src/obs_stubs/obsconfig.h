#pragma once

// Standalone-build stub for OBS generated header.
//
// In a full OBS build, this file is generated from libobs/obsconfig.h.in and
// provides install paths and feature toggles.
//
// For a plugin-only build that links against an installed OBS.app, we only need
// these macros to exist as string literals/integers so libobs headers compile.

#ifndef OBS_INSTALL_PREFIX
#define OBS_INSTALL_PREFIX ""
#endif

#ifndef OBS_DATA_PATH
#define OBS_DATA_PATH ""
#endif

#ifndef OBS_PLUGIN_PATH
#define OBS_PLUGIN_PATH ""
#endif

#ifndef OBS_PLUGIN_DESTINATION
#define OBS_PLUGIN_DESTINATION ""
#endif

#ifndef OBS_RELEASE_CANDIDATE
#define OBS_RELEASE_CANDIDATE 0
#endif

#ifndef OBS_BETA
#define OBS_BETA 0
#endif
