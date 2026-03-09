#include "ptz_dock.hpp"

#include <obs-frontend-api.h>

#include <QDockWidget>
#include <QEvent>
#include <QHBoxLayout>
#include <QLabel>
#include <QLayoutItem>
#include <QPushButton>
#include <QScrollArea>
#include <QToolButton>
#include <QVBoxLayout>
#include <QWidget>

#include <memory>

#include "ptz_config.hpp"
#include "ptz_settings_dialog.hpp"

namespace {
QString levelPrefix(int level)
{
	// 0=info, 1=ok, 2=err
	switch (level) {
	case 1:
		return QStringLiteral("OK: ");
	case 2:
		return QStringLiteral("ERR: ");
	default:
		return QStringLiteral("Status: ");
	}
}

void applyToggleVisual(QPushButton *toggle, bool active)
{
	const bool compact = toggle->minimumWidth() < 70;
	const QString padding = compact ? QStringLiteral("2px 8px") : QStringLiteral("3px 10px");
	const QString radius = compact ? QStringLiteral("9px") : QStringLiteral("11px");
	const QString activeStyle = QStringLiteral("QPushButton { border: 1px solid #2e7d32; border-radius: ") + radius + QStringLiteral("; padding: ") + padding + QStringLiteral("; background-color: #3a9a3f; color: white; font-weight: 600; }");
	const QString inactiveStyle = QStringLiteral("QPushButton { border: 1px solid #6b7280; border-radius: ") + radius + QStringLiteral("; padding: ") + padding + QStringLiteral("; background-color: #555b66; color: #f3f4f6; font-weight: 600; }");

	toggle->setText(active ? QStringLiteral("ON") : QStringLiteral("OFF"));
	toggle->setToolTip(active ? QStringLiteral("Disable tracking") : QStringLiteral("Enable tracking"));
	toggle->setStyleSheet(active ? activeStyle : inactiveStyle);
}

QWidget *buildCameraRow(const PtzCamera &cam, int index, PtzHttpClient *http)
{
	auto *row = new QWidget();
	auto *layout = new QHBoxLayout(row);
	layout->setContentsMargins(0, 0, 0, 0);
	layout->setSpacing(6);

	auto *name = new QLabel(cam.name);
	name->setObjectName(QStringLiteral("ptzName"));
	name->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred);

	auto *toggle = new QPushButton();
	toggle->setObjectName(QStringLiteral("ptzToggle"));
	toggle->setCheckable(true);
	toggle->setChecked(false);
	toggle->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
	applyToggleVisual(toggle, false);

	QObject::connect(toggle, &QPushButton::toggled, row, [cam, http, toggle](bool active) {
		http->sendAutotracking(cam, active);
		applyToggleVisual(toggle, active);
	});

	layout->addWidget(name, 1);
	layout->addWidget(toggle);

	row->setObjectName(QStringLiteral("ptzRow") + QString::number(index));
	return row;
}
} // namespace

PtzDockController::PtzDockController(QObject *parent) : QObject(parent)
{
	QObject::connect(&http_, &PtzHttpClient::statusMessage, this, [this](const QString &msg, int lvl) {
		setStatus(msg, lvl);
	});
}

PtzDockController::~PtzDockController()
{
	for (obs_hotkey_id id : hk_on_)
		if (id != OBS_INVALID_HOTKEY_ID)
			obs_hotkey_unregister(id);
	for (obs_hotkey_id id : hk_off_)
		if (id != OBS_INVALID_HOTKEY_ID)
			obs_hotkey_unregister(id);
	if (hk_all_off_ != OBS_INVALID_HOTKEY_ID)
		obs_hotkey_unregister(hk_all_off_);
}

void PtzDockController::initialize()
{
	cameras_ = PtzConfig::load();
	buildDock();
	rebuildCameraUi();
	registerHotkeys();
	setStatus(QStringLiteral("Ready"), 0);
}

void PtzDockController::buildDock()
{
	if (dock_)
		return;

	dock_ = new QDockWidget(QStringLiteral("PTZ Tracking"));
	dock_->setObjectName(QStringLiteral("ptz-tracking-dock"));

	root_ = new QWidget(dock_);
	rootLayout_ = new QVBoxLayout(root_);
	rootLayout_->setContentsMargins(6, 6, 6, 6);
	rootLayout_->setSpacing(6);

	// Top bar (settings)
	auto *top = new QHBoxLayout();
	top->setContentsMargins(0, 0, 0, 0);
	settings_ = new QToolButton(root_);
	settings_->setText(QStringLiteral("⚙"));
	settings_->setToolTip(QStringLiteral("Settings"));
	QObject::connect(settings_, &QToolButton::clicked, root_, [this]() { openSettings(); });

	top->addStretch(1);
	top->addWidget(settings_);
	rootLayout_->addLayout(top);

	// Camera list container
	auto *cameraContainer = new QWidget(root_);
	cameraLayout_ = new QVBoxLayout(cameraContainer);
	cameraLayout_->setContentsMargins(0, 0, 0, 0);
	cameraLayout_->setSpacing(4);

	cameraScroll_ = new QScrollArea(root_);
	cameraScroll_->setWidgetResizable(true);
	cameraScroll_->setFrameShape(QFrame::NoFrame);
	cameraScroll_->setWidget(cameraContainer);
	rootLayout_->addWidget(cameraScroll_, 1);

	// Emergency button
	allOff_ = new QPushButton(QStringLiteral("ALL OFF"), root_);
	QObject::connect(allOff_, &QPushButton::clicked, root_, [this]() {
		setStatus(QStringLiteral("ALL OFF..."), 0);
		http_.sendAllOff(cameras_);
	});
	rootLayout_->addWidget(allOff_);

	// Bottom status
	status_ = new QLabel(root_);
	status_->setWordWrap(false);
	status_->setTextInteractionFlags(Qt::TextSelectableByMouse);
	status_->setToolTip(QStringLiteral("Status"));
	rootLayout_->addWidget(status_);

	root_->setLayout(rootLayout_);
	dock_->setWidget(root_);
	root_->installEventFilter(this);
	dock_->installEventFilter(this);

	obs_frontend_add_custom_qdock("ptz-tracking-dock", dock_);
	applyResponsiveLayout();
}

void PtzDockController::rebuildCameraUi()
{
	if (!cameraLayout_)
		return;

	QLayoutItem *child = nullptr;
	while ((child = cameraLayout_->takeAt(0)) != nullptr) {
		delete child->widget();
		delete child;
	}

	int index = 0;
	for (const PtzCamera &cam : cameras_) {
		cameraLayout_->addWidget(buildCameraRow(cam, index, &http_));
		index++;
	}
	cameraLayout_->addStretch(1);
	applyResponsiveLayout();
}

void PtzDockController::openSettings()
{
	QWidget *parent = static_cast<QWidget *>(obs_frontend_get_main_window());
	PtzSettingsDialog dlg(cameras_, parent);
	if (dlg.exec() != QDialog::Accepted)
		return;

	const QVector<PtzCamera> updated = dlg.cameras();
	if (updated.isEmpty()) {
		setStatus(QStringLiteral("At least one camera is required"), 2);
		return;
	}

	cameras_ = updated;
	PtzConfig::save(cameras_);
	rebuildCameraUi();
	registerHotkeys();
	setStatus(QStringLiteral("Settings saved"), 1);
}

void PtzDockController::setStatus(const QString &message, int level)
{
	if (!status_)
		return;

	const QString full = levelPrefix(level) + message;
	status_->setText(full);
	status_->setToolTip(full);
}

void PtzDockController::registerHotkeys()
{
	for (obs_hotkey_id id : hk_on_)
		if (id != OBS_INVALID_HOTKEY_ID)
			obs_hotkey_unregister(id);
	for (obs_hotkey_id id : hk_off_)
		if (id != OBS_INVALID_HOTKEY_ID)
			obs_hotkey_unregister(id);
	if (hk_all_off_ != OBS_INVALID_HOTKEY_ID)
		obs_hotkey_unregister(hk_all_off_);

	const int count = cameras_.size();
	hk_on_.assign(count, OBS_INVALID_HOTKEY_ID);
	hk_off_.assign(count, OBS_INVALID_HOTKEY_ID);

	for (int i = 0; i < count; ++i) {
		const QByteArray idOn = QByteArray("ptz_tracking_on_") + QByteArray::number(i + 1);
		const QByteArray idOff = QByteArray("ptz_tracking_off_") + QByteArray::number(i + 1);
		const QString cameraName = cameras_[i].name.trimmed().isEmpty()
			? (QStringLiteral("Camera ") + QString::number(i + 1))
			: cameras_[i].name.trimmed();
		const QString descOn = QStringLiteral("PTZ Tracking: ") + cameraName + QStringLiteral(" ON");
		const QString descOff = QStringLiteral("PTZ Tracking: ") + cameraName + QStringLiteral(" OFF");

		hk_on_[i] = obs_hotkey_register_frontend(idOn.constData(), descOn.toUtf8().constData(), &PtzDockController::hotkeyOn, this);
		hk_off_[i] = obs_hotkey_register_frontend(idOff.constData(), descOff.toUtf8().constData(), &PtzDockController::hotkeyOff, this);
	}

	hk_all_off_ = obs_hotkey_register_frontend(
		"ptz_tracking_all_off", "PTZ Tracking: ALL OFF", &PtzDockController::hotkeyAllOff, this);
}

void PtzDockController::applyResponsiveLayout()
{
	if (!root_ || !rootLayout_ || !cameraLayout_)
		return;

	const int width = root_->width();
	const bool compact = width > 0 && width < 320;
	const bool extraCompact = width > 0 && width < 250;

	const int margin = extraCompact ? 3 : (compact ? 4 : 6);
	const int rootSpacing = extraCompact ? 3 : (compact ? 4 : 6);
	const int rowSpacing = extraCompact ? 3 : (compact ? 4 : 6);

	rootLayout_->setContentsMargins(margin, margin, margin, margin);
	rootLayout_->setSpacing(rootSpacing);
	cameraLayout_->setSpacing(rowSpacing);

	if (settings_) {
		settings_->setMinimumSize(extraCompact ? 22 : 24, extraCompact ? 22 : 24);
	}
	if (allOff_) {
		allOff_->setMinimumHeight(extraCompact ? 22 : (compact ? 24 : 28));
	}
	if (status_) {
		status_->setWordWrap(compact);
	}

	for (int i = 0; i < cameraLayout_->count(); ++i) {
		QLayoutItem *item = cameraLayout_->itemAt(i);
		if (!item || !item->widget())
			continue;

		QWidget *row = item->widget();
		if (!row->objectName().startsWith(QStringLiteral("ptzRow")))
			continue;

		if (auto *layout = qobject_cast<QHBoxLayout *>(row->layout())) {
			layout->setSpacing(rowSpacing);
		}

		if (auto *name = row->findChild<QLabel *>(QStringLiteral("ptzName"))) {
			name->setMinimumWidth(extraCompact ? 58 : (compact ? 76 : 120));
		}
		if (auto *toggle = row->findChild<QPushButton *>(QStringLiteral("ptzToggle"))) {
			toggle->setMinimumWidth(extraCompact ? 52 : (compact ? 62 : 82));
			toggle->setMinimumHeight(extraCompact ? 20 : (compact ? 22 : 24));
			applyToggleVisual(toggle, toggle->isChecked());
		}
	}
}

bool PtzDockController::eventFilter(QObject *watched, QEvent *event)
{
	if ((watched == root_ || watched == dock_) &&
	    (event->type() == QEvent::Resize || event->type() == QEvent::Show)) {
		applyResponsiveLayout();
	}

	return QObject::eventFilter(watched, event);
}

void PtzDockController::hotkeyOn(void *data, obs_hotkey_id id, obs_hotkey_t *, bool pressed)
{
	if (!pressed)
		return;
	auto *self = static_cast<PtzDockController *>(data);
	if (!self)
		return;

	for (int i = 0; i < self->hk_on_.size(); ++i) {
		if (self->hk_on_[i] == id) {
			if (i < self->cameras_.size())
				self->http_.sendAutotracking(self->cameras_[i], true);
			else
				self->setStatus(QStringLiteral("No camera mapped to hotkey ") + QString::number(i + 1), 2);
			return;
		}
	}
}

void PtzDockController::hotkeyOff(void *data, obs_hotkey_id id, obs_hotkey_t *, bool pressed)
{
	if (!pressed)
		return;
	auto *self = static_cast<PtzDockController *>(data);
	if (!self)
		return;

	for (int i = 0; i < self->hk_off_.size(); ++i) {
		if (self->hk_off_[i] == id) {
			if (i < self->cameras_.size())
				self->http_.sendAutotracking(self->cameras_[i], false);
			else
				self->setStatus(QStringLiteral("No camera mapped to hotkey ") + QString::number(i + 1), 2);
			return;
		}
	}
}

void PtzDockController::hotkeyAllOff(void *data, obs_hotkey_id, obs_hotkey_t *, bool pressed)
{
	if (!pressed)
		return;
	auto *self = static_cast<PtzDockController *>(data);
	if (!self)
		return;
	self->http_.sendAllOff(self->cameras_);
}
