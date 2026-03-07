#pragma once

#include <QString>

struct PtzCamera {
	QString name;
	QString host; // host or ip only (no scheme)
};
