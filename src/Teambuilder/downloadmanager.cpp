#include <ctime>
#include <QNetworkReply>
#include <QDomDocument>

#include "downloadmanager.h"
#include "../Utilities/functions.h"
#include "../Shared/config.h"

inline QString getXmlFileName()
{
    int major = CLIENT_VERSION_NUMBER/1000;
    int minor = (CLIENT_VERSION_NUMBER%1000)/100;

    return QString("%1.%2.xml").arg(major).arg(minor);
}

DownloadManager::DownloadManager(QObject *parent) :
    QObject(parent), currentUpdateId(-1)
{
}

void DownloadManager::loadUpdatesAvailable()
{
    QSettings settings;

    int time = settings.value("Updates/LastUpdateCheck").toInt();

    /* If the update information is older than 3 days... */
    if (::time(NULL) - time > 24*3600*3) {
        download(QString("https://raw.github.com/coyotte508/pokemon-online/master/updates/%1").arg(getXmlFileName()),this,SLOT(onUpdateFileDownloaded()));
    } else {
        readAvailableUpdatesFromFile();
    }
}

void DownloadManager::download(const QString &url, QObject *target, const char *slot)
{
    QNetworkRequest request;
    request.setUrl(QUrl(url));
    request.setRawHeader("User-Agent", "Pokemon-Online Updater");

    QNetworkReply* reply = manager.get(request);
    connect(reply, SIGNAL(finished()), target, slot);
    connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), SLOT(onUpdateFileDownloaded()));
}

void DownloadManager::onUpdateFileDownloaded()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply*> (sender());

    if (!reply) {
        return;
    }

    reply->deleteLater();

    if (reply->error()) {
        emit updatesAvailable(reply->errorString(), true);
        return;
    }

    QFile out(appDataPath("Updates/", true) + getXmlFileName());
    out.open(QIODevice::WriteOnly);

    if (!out.isOpen()) {
        /* Error while downloading */
        emit updatesAvailable(tr("Impossible to see available updates: impossible to write to %1.").arg(appDataPath("Updates/")+getXmlFileName()), true);
        return;
    }
    QByteArray response = reply->readAll();

    out.write(response);
    out.close();

    QSettings settings;
    settings.setValue("Updates/LastUpdateCheck", QString::number(::time(NULL)));

    readAvailableUpdatesFromFile();
}

void DownloadManager::loadCurrentUpdateId()
{
    if (currentUpdateId != -1) {
        return;
    }

    QSettings in("version.ini", QSettings::IniFormat);
    currentUpdateId = std::max(in.value("updateId").toInt(), UPDATE_ID);
}

void DownloadManager::readAvailableUpdatesFromFile()
{
    QFile in(appDataPath("Updates/") + getXmlFileName());

    if (!in.exists()) {
        return;
    }

    in.open(QIODevice::ReadOnly);

    QString data = QString::fromUtf8(in.readAll());

    QDomDocument doc;

    if (!doc.setContent(data)) {
        return;
    }

    qDebug() << "doc.name " <<  doc.nodeName();
    qDebug() << "doc.firstChild" << doc.firstChild().nodeName();

    QDomElement el = doc.firstChildElement("updates");

    qDebug() << "element.name: " << el.nodeName();
    qDebug() << "first update: " << el.firstChildElement("update").nodeName();

    el = el.firstChildElement("update");

    while (!el.isNull() && !isValidUpdateElement(el)) {
        el = el.nextSiblingElement("update");
    }

    if (!el.isNull()) {
        targetDownload = el.attribute("download");
        changeLogUrl = el.attribute("changelog");

        QString versionTo = el.attribute("versionTo");

        if (versionTo.length() > 0 &&  versionTo != VERSION) {
            emit updatesAvailable(tr("An update to version %1 is available!").arg(versionTo), false);
        } else {
            emit updatesAvailable(tr("An update is available!"), false);
        }
    }
}

bool DownloadManager::isValidUpdateElement(const QDomElement &el)
{
    qDebug() << "calling valid element";

    if (!el.hasAttribute("updateIdFrom") || !el.hasAttribute("os")) {
        return false;
    }

    loadCurrentUpdateId();

    int minId, maxId;

    minId = el.attribute("updateIdFrom").section(",", 0, 0).trimmed().toInt();
    maxId = el.attribute("updateIdFrom").section(",", -1, -1).trimmed().toInt();

    if (currentUpdateId < minId || currentUpdateId > maxId) {
        return false;
    }

    QString os = el.attribute("os");

    if (os != "all" && os != OS) {
        return false;
    }

    qDebug() << "valid element";

    return true;
}
