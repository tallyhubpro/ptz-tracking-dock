#include <obs-module.h>
#include <obs-frontend-api.h>

#include <QApplication>
#include <QMetaObject>

#include "ptz_dock.hpp"

OBS_DECLARE_MODULE()
OBS_MODULE_USE_DEFAULT_LOCALE("ptz-tracking-dock", "en-US")

static PtzDockController *g_controller = nullptr;

extern "C"
const char *obs_module_name(void)
{
	return "PTZ Tracking Dock";
}

extern "C"
const char *obs_module_description(void)
{
	return "Dock panel + hotkeys for PTZ autotracking control.";
}

extern "C"
bool obs_module_load(void)
{
	// OBS frontend is required for docks/hotkeys.
	if (!obs_frontend_get_main_window()) {
		blog(LOG_WARNING, "ptz-tracking-dock: frontend not available yet");
		return true;
	}

	QMetaObject::invokeMethod(qApp, []() {
		if (g_controller) {
			return;
		}
		g_controller = new PtzDockController();
		g_controller->initialize();
	}, Qt::QueuedConnection);

	blog(LOG_INFO, "ptz-tracking-dock loaded (version %s)", PTZ_TRACKING_DOCK_VERSION);
	return true;
}

extern "C"
void obs_module_unload(void)
{
	QMetaObject::invokeMethod(qApp, []() {
		delete g_controller;
		g_controller = nullptr;
	}, Qt::QueuedConnection);

	blog(LOG_INFO, "ptz-tracking-dock unloaded");
}
