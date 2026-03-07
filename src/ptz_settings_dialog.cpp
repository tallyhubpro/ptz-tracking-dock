#include "ptz_settings_dialog.hpp"

#include <QHeaderView>
#include <QHBoxLayout>
#include <QPushButton>
#include <QTableWidget>
#include <QVBoxLayout>

namespace {
QString sanitizeHost(QString host)
{
	host = host.trimmed();
	if (host.startsWith("http://", Qt::CaseInsensitive))
		host = host.mid(7);
	if (host.startsWith("https://", Qt::CaseInsensitive))
		host = host.mid(8);
	int slash = host.indexOf('/');
	if (slash >= 0)
		host = host.left(slash);
	return host.trimmed();
}
} // namespace

PtzSettingsDialog::PtzSettingsDialog(const QVector<PtzCamera> &initial, QWidget *parent)
	: QDialog(parent)
{
	setWindowTitle(QStringLiteral("PTZ Tracking Cameras"));
	setModal(true);

	table_ = new QTableWidget(this);
	table_->setColumnCount(2);
	table_->setHorizontalHeaderLabels({QStringLiteral("Name"), QStringLiteral("Host/IP")});
	table_->horizontalHeader()->setStretchLastSection(true);
	table_->verticalHeader()->setVisible(false);
	table_->setSelectionBehavior(QAbstractItemView::SelectRows);
	table_->setSelectionMode(QAbstractItemView::SingleSelection);
	table_->setEditTriggers(QAbstractItemView::DoubleClicked | QAbstractItemView::EditKeyPressed | QAbstractItemView::SelectedClicked);

	for (const PtzCamera &cam : initial)
		addRow(cam);
	if (table_->rowCount() == 0)
		addRow({QStringLiteral("Stage Camera"), QStringLiteral("192.168.1.108")});

	add_ = new QPushButton(QStringLiteral("+"), this);
	remove_ = new QPushButton(QStringLiteral("–"), this);
	up_ = new QPushButton(QStringLiteral("↑"), this);
	down_ = new QPushButton(QStringLiteral("↓"), this);
	ok_ = new QPushButton(QStringLiteral("Save"), this);
	cancel_ = new QPushButton(QStringLiteral("Cancel"), this);

	QObject::connect(add_, &QPushButton::clicked, this, [this]() {
		addRow({QStringLiteral("New Camera"), QString()});
	});
	QObject::connect(remove_, &QPushButton::clicked, this, [this]() {
		const int row = table_->currentRow();
		if (row >= 0)
			table_->removeRow(row);
	});
	QObject::connect(up_, &QPushButton::clicked, this, [this]() { moveRow(-1); });
	QObject::connect(down_, &QPushButton::clicked, this, [this]() { moveRow(+1); });
	QObject::connect(ok_, &QPushButton::clicked, this, &QDialog::accept);
	QObject::connect(cancel_, &QPushButton::clicked, this, &QDialog::reject);

	auto *tools = new QVBoxLayout();
	tools->addWidget(add_);
	tools->addWidget(remove_);
	tools->addSpacing(8);
	tools->addWidget(up_);
	tools->addWidget(down_);
	tools->addStretch(1);

	auto *top = new QHBoxLayout();
	top->addWidget(table_, 1);
	top->addLayout(tools);

	auto *buttons = new QHBoxLayout();
	buttons->addStretch(1);
	buttons->addWidget(cancel_);
	buttons->addWidget(ok_);

	auto *root = new QVBoxLayout(this);
	root->addLayout(top, 1);
	root->addLayout(buttons);
	setLayout(root);

	resize(520, 260);
}
void PtzSettingsDialog::addRow(const PtzCamera &cam)
{
	const int row = table_->rowCount();
	table_->insertRow(row);
	table_->setItem(row, 0, new QTableWidgetItem(cam.name));
	table_->setItem(row, 1, new QTableWidgetItem(cam.host));
	if (row == 0)
		table_->selectRow(0);
}

void PtzSettingsDialog::moveRow(int delta)
{
	const int row = table_->currentRow();
	if (row < 0)
		return;
	const int target = row + delta;
	if (target < 0 || target >= table_->rowCount())
		return;

	for (int col = 0; col < table_->columnCount(); ++col) {
		QTableWidgetItem *a = table_->takeItem(row, col);
		QTableWidgetItem *b = table_->takeItem(target, col);
		table_->setItem(row, col, b);
		table_->setItem(target, col, a);
	}
	table_->selectRow(target);
}

QVector<PtzCamera> PtzSettingsDialog::cameras() const
{
	QVector<PtzCamera> out;
	out.reserve(table_->rowCount());

	for (int r = 0; r < table_->rowCount(); ++r) {
		const QString name = table_->item(r, 0) ? table_->item(r, 0)->text().trimmed() : QString();
		const QString host = table_->item(r, 1) ? sanitizeHost(table_->item(r, 1)->text()) : QString();
		if (host.isEmpty())
			continue;
		out.push_back({name.isEmpty() ? QStringLiteral("Camera") : name, host});
	}

	return out;
}
