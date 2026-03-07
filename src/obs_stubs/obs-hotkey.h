#pragma once

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef uint64_t obs_hotkey_id;
typedef struct obs_hotkey obs_hotkey_t;
typedef void (*obs_hotkey_func)(void *data, obs_hotkey_id id, obs_hotkey_t *hotkey, bool pressed);

#define OBS_INVALID_HOTKEY_ID ((obs_hotkey_id)0)

obs_hotkey_id obs_hotkey_register_frontend(const char *name, const char *description, obs_hotkey_func func, void *data);
void obs_hotkey_unregister(obs_hotkey_id id);

#ifdef __cplusplus
}
#endif
