#pragma once

#include <QObject>
#include <QVector>

#include <obs-module.h>
#include <obs-hotkey.h>

#include "ptz_http.hpp"
#include "ptz_types.hpp"

class QDockWidget;
class QLabel;
class QScrollArea;
class QToolButton;
class QPushButton;
class QVBoxLayout;

class PtzDockController : public QObject {
	Q_OBJECT
public:
	explicit PtzDockController(QObject *parent = nullptr);
	~PtzDockController() override;

	void initialize();

private:
	void buildDock();
	void rebuildCameraUi();
	void openSettings();
	void setStatus(const QString &message, int level);
	void registerHotkeys();
	void applyResponsiveLayout();
	bool eventFilter(QObject *watched, QEvent *event) override;

	static void hotkeyOn(void *data, obs_hotkey_id, obs_hotkey_t *, bool pressed);
	static void hotkeyOff(void *data, obs_hotkey_id, obs_hotkey_t *, bool pressed);
	static void hotkeyAllOff(void *data, obs_hotkey_id, obs_hotkey_t *, bool pressed);

	QDockWidget *dock_ = nullptr;
	QWidget *root_ = nullptr;
	QVBoxLayout *rootLayout_ = nullptr;
	QVBoxLayout *cameraLayout_ = nullptr;
	QScrollArea *cameraScroll_ = nullptr;
	QLabel *status_ = nullptr;
	QToolButton *settings_ = nullptr;
	QPushButton *allOff_ = nullptr;

	QVector<PtzCamera> cameras_;
	PtzHttpClient http_;

	obs_hotkey_id hk_all_off_ = OBS_INVALID_HOTKEY_ID;
	QVector<obs_hotkey_id> hk_on_;
	QVector<obs_hotkey_id> hk_off_;
};
