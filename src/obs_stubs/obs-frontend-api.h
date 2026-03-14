#pragma once

#include "obs-hotkey.h"

#ifdef __cplusplus
extern "C" {
#endif

enum obs_frontend_event {
	OBS_FRONTEND_EVENT_SCENE_CHANGED = 0,
	OBS_FRONTEND_EVENT_FINISHED_LOADING = 31,
};

typedef void (*obs_frontend_cb)(enum obs_frontend_event event, void *private_data);

void *obs_frontend_get_main_window(void);
void obs_frontend_add_custom_qdock(const char *id, void *dock_widget);
bool obs_frontend_add_dock_by_id(const char *id, const char *title, void *widget);
void obs_frontend_remove_dock(const char *id);
void obs_frontend_add_event_callback(obs_frontend_cb callback, void *private_data);
void obs_frontend_remove_event_callback(obs_frontend_cb callback, void *private_data);

#ifdef __cplusplus
}
#endif
