#include "obs-module.h"
#include "obs-frontend-api.h"

#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <cstring>

extern "C" uint32_t obs_get_version(void)
{
	// Dummy value for dev-only builds that do not link against libobs.
	return 0;
}

extern "C" char *obs_module_get_config_path(obs_module_t *, const char *file)
{
	const char *name = file ? file : "";
	const size_t len = std::strlen(name);
	char *out = static_cast<char *>(std::malloc(len + 1));
	if (!out)
		return nullptr;
	std::memcpy(out, name, len + 1);
	return out;
}

extern "C" void bfree(void *ptr)
{
	std::free(ptr);
}

extern "C" void blog(int log_level, const char *format, ...)
{
	std::fprintf(stderr, "[obs-stub:%d] ", log_level);
	va_list args;
	va_start(args, format);
	std::vfprintf(stderr, format ? format : "", args);
	va_end(args);
	std::fprintf(stderr, "\n");
}

extern "C" void *obs_frontend_get_main_window(void)
{
	return nullptr;
}

extern "C" void obs_frontend_add_custom_qdock(const char *, void *)
{
}

extern "C" obs_hotkey_id obs_hotkey_register_frontend(const char *, const char *, obs_hotkey_func, void *)
{
	return OBS_INVALID_HOTKEY_ID;
}

extern "C" void obs_hotkey_unregister(obs_hotkey_id)
{
}
