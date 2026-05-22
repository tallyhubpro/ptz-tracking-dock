#include "ptz_config.hpp"

#include <obs-module.h>

#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

#include <cerrno>
#include <string>
#include <sys/stat.h>
#include <sys/types.h>

namespace {
QString configPath()
{
	char *path = obs_module_config_path(PtzConfig::configFileName());
	if (!path) {
		return QString();
	}
	QString result = QString::fromUtf8(path);
	bfree(path);
	return result;
}

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
	int q = host.indexOf('?');
	if (q >= 0)
		host = host.left(q);
	int hash = host.indexOf('#');
	if (hash >= 0)
		host = host.left(hash);
	return host.trimmed();
}

bool ensureDirectoryPath(const QString &directory)
{
	const QByteArray utf8 = directory.toUtf8();
	if (utf8.isEmpty())
		return true;

	std::string path(utf8.constData(), static_cast<size_t>(utf8.size()));
	if (path.empty())
		return true;

	if (mkdir(path.c_str(), 0755) == 0 || errno == EEXIST)
		return true;

	size_t start = (path[0] == '/') ? 1 : 0;
	for (size_t i = start; i < path.size(); ++i) {
		if (path[i] != '/')
			continue;

		std::string part = path.substr(0, i);
		if (part.empty())
			continue;
		if (mkdir(part.c_str(), 0755) != 0 && errno != EEXIST)
			return false;
	}

	if (mkdir(path.c_str(), 0755) != 0 && errno != EEXIST)
		return false;

	return true;
}
} // namespace

const char *PtzConfig::configFileName()
{
	return "ptz-tracking-dock.json";
}

QVector<PtzCamera> PtzConfig::load()
{
	const QString path = configPath();
	if (path.isEmpty())
		return {};

	QFile file(path);
	if (!file.exists())
		return {};
	if (!file.open(QIODevice::ReadOnly)) {
		blog(LOG_WARNING, "ptz-tracking-dock: failed to open config for read: %s", path.toUtf8().constData());
		return {};
	}

	const QByteArray raw = file.readAll();
	const QJsonDocument doc = QJsonDocument::fromJson(raw);
	if (!doc.isObject())
		return {};

	const QJsonObject obj = doc.object();
	const QJsonArray cams = obj.value("cameras").toArray();
	QVector<PtzCamera> out;
	out.reserve(cams.size());

	for (const QJsonValue v : cams) {
		if (!v.isObject())
			continue;
		const QJsonObject c = v.toObject();
		PtzCamera cam;
		cam.name = c.value("name").toString().trimmed();
		cam.host = sanitizeHost(c.value("host").toString());
		if (cam.name.isEmpty())
			cam.name = QStringLiteral("Camera");
		if (cam.host.isEmpty())
			continue;
		out.push_back(cam);
	}

	return out;
}

void PtzConfig::save(const QVector<PtzCamera> &cameras)
{
	const QString path = configPath();
	if (path.isEmpty())
		return;

	const QFileInfo info(path);
	const QString parentPath = info.absolutePath();
	if (!parentPath.isEmpty()) {
		if (!ensureDirectoryPath(parentPath)) {
			blog(LOG_WARNING, "ptz-tracking-dock: failed to create config dir: %s", parentPath.toUtf8().constData());
			return;
		}
	}

	QJsonArray cams;
	for (const PtzCamera &cam : cameras) {
		const QString host = sanitizeHost(cam.host);
		if (host.isEmpty())
			continue;
		QJsonObject o;
		o.insert("name", cam.name.trimmed().isEmpty() ? QStringLiteral("Camera") : cam.name.trimmed());
		o.insert("host", host);
		cams.append(o);
	}

	QJsonObject root;
	root.insert("cameras", cams);

	QFile file(path);
	if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
		blog(LOG_WARNING, "ptz-tracking-dock: failed to open config for write: %s", path.toUtf8().constData());
		return;
	}
	file.write(QJsonDocument(root).toJson(QJsonDocument::Indented));
}
