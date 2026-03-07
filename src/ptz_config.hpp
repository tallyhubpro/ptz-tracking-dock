#pragma once

#include <QVector>

#include "ptz_types.hpp"

class PtzConfig {
public:
	static QVector<PtzCamera> load();
	static void save(const QVector<PtzCamera> &cameras);
	static const char *configFileName();

private:
};
