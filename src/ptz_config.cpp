#include "ptz_config.hpp"

#include <obs-module.h>

#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

namespace {
QVector<PtzCamera> defaultCameras()
{
	return {
		{QStringLiteral("Stage Camera"), QStringLiteral("192.168.1.108")},
		{QStringLiteral("Audience Camera"), QStringLiteral("192.168.1.109")},
	};
}

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
} // namespace

const char *PtzConfig::configFileName()
{
	return "ptz-tracking-dock.json";
}

QVector<PtzCamera> PtzConfig::load()
{
	const QString path = configPath();
	if (path.isEmpty())
		return defaultCameras();

	QFile file(path);
	if (!file.exists())
		return defaultCameras();
	if (!file.open(QIODevice::ReadOnly))
		return defaultCameras();

	const QByteArray raw = file.readAll();
	const QJsonDocument doc = QJsonDocument::fromJson(raw);
	if (!doc.isObject())
		return defaultCameras();

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

	if (out.isEmpty())
		return defaultCameras();
	return out;
}

void PtzConfig::save(const QVector<PtzCamera> &cameras)
{
	const QString path = configPath();
	if (path.isEmpty())
		return;

	QJsonArray cams;
	for (const PtzCamera &cam : cameras) {
		if (cam.host.trimmed().isEmpty())
			continue;
		QJsonObject o;
		o.insert("name", cam.name.trimmed().isEmpty() ? QStringLiteral("Camera") : cam.name.trimmed());
		o.insert("host", cam.host.trimmed());
		cams.append(o);
	}

	QJsonObject root;
	root.insert("cameras", cams);

	QFile file(path);
	if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate))
		return;
	file.write(QJsonDocument(root).toJson(QJsonDocument::Indented));
}
