#include "ptz_http.hpp"

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QTimer>
#include <QUrl>

namespace {
QString buildUrl(const QString &host, bool enabled)
{
	// Matches the browser dock behavior: set_overlay&autotracking&on|off
	const QString mode = enabled ? QStringLiteral("on") : QStringLiteral("off");
	return QStringLiteral("http://") + host + QStringLiteral("/cgi-bin/param.cgi?set_overlay&autotracking&") + mode;
}

QString compactPreview(const QString &text)
{
	QString out = text;
	out.replace('\r', ' ');
	out.replace('\n', ' ');
	out = out.simplified();
	if (out.size() > 96)
		out = out.left(96) + QStringLiteral("...");
	return out;
}

bool responseConfirmsTracking(const QString &bodyLower, bool enabled)
{
	if (bodyLower.isEmpty())
		return false;

	const bool hasKeyword = bodyLower.contains(QStringLiteral("autotracking")) ||
					bodyLower.contains(QStringLiteral("tracking"));
	if (!hasKeyword)
		return false;

	if (enabled) {
		return bodyLower.contains(QStringLiteral("autotracking=on")) ||
		       bodyLower.contains(QStringLiteral("autotracking on")) ||
		       bodyLower.contains(QStringLiteral("tracking=on")) ||
		       bodyLower.contains(QStringLiteral("tracking on")) ||
		       bodyLower.contains(QStringLiteral("tracking=1")) ||
		       bodyLower.contains(QStringLiteral("autotracking=1"));
	}

	return bodyLower.contains(QStringLiteral("autotracking=off")) ||
	       bodyLower.contains(QStringLiteral("autotracking off")) ||
	       bodyLower.contains(QStringLiteral("tracking=off")) ||
	       bodyLower.contains(QStringLiteral("tracking off")) ||
	       bodyLower.contains(QStringLiteral("tracking=0")) ||
	       bodyLower.contains(QStringLiteral("autotracking=0"));
}
} // namespace
class PtzHttpClientPrivate {
public:
	QNetworkAccessManager net;
};

PtzHttpClient::PtzHttpClient(QObject *parent) : QObject(parent)
{
	// nothing
}

void PtzHttpClient::sendAutotracking(const PtzCamera &camera, bool enabled)
{
	const QString url = buildUrl(camera.host, enabled);
	const QString label = camera.name + QStringLiteral(": tracking ") + (enabled ? QStringLiteral("ON") : QStringLiteral("OFF"));
	sendRequest(url, label, enabled);
}

void PtzHttpClient::sendAllOff(const QVector<PtzCamera> &cameras)
{
	for (const PtzCamera &cam : cameras)
		sendAutotracking(cam, false);
}

void PtzHttpClient::sendRequest(const QString &url, const QString &label, bool enabled)
{
	auto *net = new QNetworkAccessManager(this);
	QNetworkRequest req{QUrl(url)};
	req.setHeader(QNetworkRequest::UserAgentHeader, QStringLiteral("ptz-tracking-dock"));

	emit statusMessage(QStringLiteral("Sending: ") + label, 0);

	QNetworkReply *reply = net->get(req);
	QTimer *timer = new QTimer(reply);
	timer->setSingleShot(true);
	timer->setInterval(3500);

	QObject::connect(timer, &QTimer::timeout, reply, [this, reply, label]() {
		reply->abort();
		emit statusMessage(QStringLiteral("No response (timeout): ") + label, 2);
	});

	QObject::connect(reply, &QNetworkReply::finished, reply, [this, reply, label, enabled]() {
		const QVariant statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
		const int code = statusCode.isValid() ? statusCode.toInt() : 0;
		const QString body = QString::fromUtf8(reply->readAll()).trimmed();
		const QString bodyPreview = compactPreview(body);
		const QString bodyLower = body.toLower();
		if (reply->error() != QNetworkReply::NoError) {
			emit statusMessage(QStringLiteral("Failed: ") + label + QStringLiteral(" (") + reply->errorString() + QStringLiteral(")"), 2);
			reply->deleteLater();
			return;
		}
		if (code >= 400) {
			emit statusMessage(QStringLiteral("HTTP ") + QString::number(code) + QStringLiteral(": ") + label, 2);
			reply->deleteLater();
			return;
		}

		if (responseConfirmsTracking(bodyLower, enabled)) {
			emit statusMessage(QStringLiteral("Confirmed: ") + label, 1);
			reply->deleteLater();
			return;
		}

		if (!bodyPreview.isEmpty()) {
			emit statusMessage(QStringLiteral("HTTP ") + QString::number(code) + QStringLiteral(" (unconfirmed): ") + label + QStringLiteral(" | ") + bodyPreview, 0);
		} else {
			emit statusMessage(QStringLiteral("HTTP ") + QString::number(code) + QStringLiteral(" (no body): ") + label, 0);
		}
		reply->deleteLater();
	});

	timer->start();
}
