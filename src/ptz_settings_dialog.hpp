#pragma once

#include <QDialog>
#include <QVector>

#include "ptz_types.hpp"

class QTableWidget;
class QPushButton;

class PtzSettingsDialog : public QDialog {
	Q_OBJECT
public:
	explicit PtzSettingsDialog(const QVector<PtzCamera> &initial, QWidget *parent = nullptr);

	QVector<PtzCamera> cameras() const;

private:
	void addRow(const PtzCamera &cam);
	void moveRow(int delta);

	QTableWidget *table_ = nullptr;
	QPushButton *add_ = nullptr;
	QPushButton *remove_ = nullptr;
	QPushButton *up_ = nullptr;
	QPushButton *down_ = nullptr;
	QPushButton *ok_ = nullptr;
	QPushButton *cancel_ = nullptr;
};
