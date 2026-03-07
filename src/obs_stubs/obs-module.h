#pragma once

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct obs_module obs_module_t;

enum {
	LOG_DEBUG = 100,
	LOG_INFO = 200,
	LOG_WARNING = 300,
	LOG_ERROR = 400,
};

/*
 * Dev-only fallback implementation of the minimal OBS module ABI.
 * These exports let OBS discover and initialize the module even when
 * building locally without the real OBS SDK headers.
 */
#define OBS_DECLARE_MODULE()                                                            \
	static obs_module_t *ptz_obs_module_ptr = NULL;                                   \
	extern "C" void obs_module_set_pointer(obs_module_t *module)                      \
	{                                                                                  \
		ptz_obs_module_ptr = module;                                                \
	}                                                                                  \
	extern "C" uint32_t obs_module_ver(void)                                          \
	{                                                                                  \
		return obs_get_version();                                                    \
	}                                                                                  \
	extern "C" obs_module_t *obs_current_module(void)                                 \
	{                                                                                  \
		return ptz_obs_module_ptr;                                                   \
	}

#define OBS_MODULE_USE_DEFAULT_LOCALE(module_name, default_locale)                      \
	extern "C" const char *obs_module_text(const char *lookup)                        \
	{                                                                                  \
		return lookup ? lookup : "";                                                \
	}                                                                                  \
	extern "C" const char *obs_module_get_string(const char *lookup, ...)            \
	{                                                                                  \
		return lookup ? lookup : "";                                                \
	}                                                                                  \
	extern "C" void obs_module_set_locale(const char *active_locale, ...)             \
	{                                                                                  \
		(void)active_locale;                                                          \
	}                                                                                  \
	extern "C" void obs_module_free_locale(void)                                      \
	{                                                                                  \
	}

uint32_t obs_get_version(void);
char *obs_module_get_config_path(obs_module_t *module, const char *file);
void bfree(void *ptr);
void blog(int log_level, const char *format, ...);
const char *obs_module_name(void);
obs_module_t *obs_current_module(void);

#define obs_module_config_path(file) obs_module_get_config_path(obs_current_module(), file)

#ifdef __cplusplus
}
#endif
