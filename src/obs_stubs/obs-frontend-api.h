#pragma once

#include "obs-hotkey.h"

#ifdef __cplusplus
extern "C" {
#endif

void *obs_frontend_get_main_window(void);
void obs_frontend_add_custom_qdock(const char *id, void *dock_widget);

#ifdef __cplusplus
}
#endif
