#include "obs-module.h"
#include "obs-frontend-api.h"

#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#ifdef _WIN32
#include <windows.h>
#endif

namespace {
#ifdef _WIN32
HMODULE loadModule(const char *name)
{
	HMODULE mod = GetModuleHandleA(name);
	if (!mod)
		mod = LoadLibraryA(name);
	return mod;
}

template<typename Fn>
Fn resolveObs(const char *symbol)
{
	HMODULE mod = loadModule("obs.dll");
	if (!mod)
		return nullptr;
	return reinterpret_cast<Fn>(GetProcAddress(mod, symbol));
}

template<typename Fn>
Fn resolveFrontend(const char *symbol)
{
	HMODULE mod = loadModule("obs-frontend-api.dll");
	if (!mod)
		return nullptr;
	return reinterpret_cast<Fn>(GetProcAddress(mod, symbol));
}
#endif
} // namespace

extern "C" uint32_t obs_get_version(void)
{
#ifdef _WIN32
	using Fn = uint32_t (*)();
	if (auto fn = resolveObs<Fn>("obs_get_version"))
		return fn();
#endif
	return 0;
}

extern "C" char *obs_module_get_config_path(obs_module_t *module, const char *file)
{
#ifdef _WIN32
	using Fn = char *(*)(obs_module_t *, const char *);
	if (auto fn = resolveObs<Fn>("obs_module_get_config_path"))
		return fn(module ? module : obs_current_module(), file);
#endif

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
#ifdef _WIN32
	using Fn = void (*)(void *);
	if (auto fn = resolveObs<Fn>("bfree")) {
		fn(ptr);
		return;
	}
#endif

	std::free(ptr);
}

extern "C" void blog(int log_level, const char *format, ...)
{
	va_list args;
	va_start(args, format);

#ifdef _WIN32
	using Fn = void (*)(int, const char *, ...);
	if (auto fn = resolveObs<Fn>("blog")) {
		char buffer[2048] = {};
		std::vsnprintf(buffer, sizeof(buffer), format ? format : "", args);
		fn(log_level, "%s", buffer);
		va_end(args);
		return;
	}
#endif

	std::fprintf(stderr, "[obs-stub:%d] ", log_level);
	std::vfprintf(stderr, format ? format : "", args);
	va_end(args);
	std::fprintf(stderr, "\n");
}

extern "C" void *obs_frontend_get_main_window(void)
{
#ifdef _WIN32
	using Fn = void *(*)();
	if (auto fn = resolveFrontend<Fn>("obs_frontend_get_main_window"))
		return fn();
#endif

	return nullptr;
}

extern "C" void obs_frontend_add_custom_qdock(const char *id, void *dock_widget)
{
#ifdef _WIN32
	using Fn = void (*)(const char *, void *);
	if (auto fn = resolveFrontend<Fn>("obs_frontend_add_custom_qdock"))
		fn(id, dock_widget);
#endif
}

extern "C" bool obs_frontend_add_dock_by_id(const char *id, const char *title, void *widget)
{
#ifdef _WIN32
	using Fn = bool (*)(const char *, const char *, void *);
	if (auto fn = resolveFrontend<Fn>("obs_frontend_add_dock_by_id"))
		return fn(id, title, widget);
#endif
	return false;
}

extern "C" void obs_frontend_remove_dock(const char *id)
{
#ifdef _WIN32
	using Fn = void (*)(const char *);
	if (auto fn = resolveFrontend<Fn>("obs_frontend_remove_dock"))
		fn(id);
#endif
}

extern "C" obs_hotkey_id obs_hotkey_register_frontend(const char *name, const char *description,
		obs_hotkey_func function, void *data)
{
#ifdef _WIN32
	using Fn = obs_hotkey_id (*)(const char *, const char *, obs_hotkey_func, void *);
	if (auto fn = resolveObs<Fn>("obs_hotkey_register_frontend"))
		return fn(name, description, function, data);
#endif

	return OBS_INVALID_HOTKEY_ID;
}

extern "C" void obs_hotkey_unregister(obs_hotkey_id id)
{
#ifdef _WIN32
	using Fn = void (*)(obs_hotkey_id);
	if (auto fn = resolveObs<Fn>("obs_hotkey_unregister"))
		fn(id);
#endif
}

extern "C" void obs_frontend_add_event_callback(obs_frontend_cb callback, void *private_data)
{
#ifdef _WIN32
	using Fn = void (*)(obs_frontend_cb, void *);
	if (auto fn = resolveFrontend<Fn>("obs_frontend_add_event_callback"))
		fn(callback, private_data);
#endif
}

extern "C" void obs_frontend_remove_event_callback(obs_frontend_cb callback, void *private_data)
{
#ifdef _WIN32
	using Fn = void (*)(obs_frontend_cb, void *);
	if (auto fn = resolveFrontend<Fn>("obs_frontend_remove_event_callback"))
		fn(callback, private_data);
#endif
}
