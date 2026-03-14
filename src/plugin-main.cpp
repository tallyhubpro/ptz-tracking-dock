#include <obs-module.h>
#include <obs-frontend-api.h>

#include <QApplication>
#include <QMetaObject>
#include <QTimer>

#include "ptz_dock.hpp"

OBS_DECLARE_MODULE()
OBS_MODULE_USE_DEFAULT_LOCALE("ptz-tracking-dock", "en-US")

static PtzDockController *g_controller = nullptr;
static QTimer *g_init_timer = nullptr;

static void try_initialize_controller()
{
	if (g_controller) {
		if (g_init_timer) {
			g_init_timer->stop();
			g_init_timer->deleteLater();
			g_init_timer = nullptr;
		}
		return;
	}

	if (!obs_frontend_get_main_window())
		return;

	g_controller = new PtzDockController();
	g_controller->initialize();
	blog(LOG_INFO, "ptz-tracking-dock initialised (version %s)",
	     PTZ_TRACKING_DOCK_VERSION);

	if (g_init_timer) {
		g_init_timer->stop();
		g_init_timer->deleteLater();
		g_init_timer = nullptr;
	}
}

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
	// Defer dock/hotkey setup until OBS has finished loading its UI.
	// obs_module_load is called before the main window exists, so we
	// register an event callback and initialise on FINISHED_LOADING.
	obs_frontend_add_event_callback([](enum obs_frontend_event event, void *) {
		if (event != OBS_FRONTEND_EVENT_FINISHED_LOADING)
			return;
		try_initialize_controller();
	}, nullptr);

	// Safety net: poll briefly until the frontend main window is available.
	QMetaObject::invokeMethod(qApp, []() {
		try_initialize_controller();
		if (g_controller || g_init_timer)
			return;

		g_init_timer = new QTimer(qApp);
		g_init_timer->setInterval(250);
		QObject::connect(g_init_timer, &QTimer::timeout, qApp, []() {
			try_initialize_controller();
		});
		g_init_timer->start();
	}, Qt::QueuedConnection);

	blog(LOG_INFO, "ptz-tracking-dock loaded (version %s)", PTZ_TRACKING_DOCK_VERSION);
	return true;
}

extern "C"
void obs_module_unload(void)
{
	if (g_init_timer) {
		g_init_timer->stop();
		g_init_timer->deleteLater();
		g_init_timer = nullptr;
	}

	QMetaObject::invokeMethod(qApp, []() {
		delete g_controller;
		g_controller = nullptr;
	}, Qt::QueuedConnection);

	blog(LOG_INFO, "ptz-tracking-dock unloaded");
}
