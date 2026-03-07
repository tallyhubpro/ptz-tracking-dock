#pragma once

#include <QObject>
#include <QVector>

#include "ptz_types.hpp"

class PtzHttpClient : public QObject {
	Q_OBJECT
public:
	explicit PtzHttpClient(QObject *parent = nullptr);

	void sendAutotracking(const PtzCamera &camera, bool enabled);
	void sendAllOff(const QVector<PtzCamera> &cameras);

signals:
	void statusMessage(const QString &message, int level);

private:
	void sendRequest(const QString &url, const QString &label, bool enabled);
};
