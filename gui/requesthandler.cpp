#include "requesthandler.h"

RequestHandler::RequestHandler(QObject *parent) :
    QObject(parent)
{
    if (downloadManager == 0) {
        qErrnoWarning("The download manager was not started. Exiting.");
        exit(EXIT_FAILURE);
    }
}

void RequestHandler::request(QUrl u) {
    qint64 uid;
    int prio;
    QString sUrl;

    sUrl = u.toString();
    if (sUrl.indexOf(QRegExp(__IMAGE_REGEXP__, Qt::CaseInsensitive)) != -1) {
        //Image requested
        prio = 1;
    }
    else {
        // HTML page requested
        prio = 0;
    }

    if (downloadManager != 0) {
        uid = downloadManager->requestDownload(this, u, prio);
//        qDebug() << "Adding request" << uid << ":" << u.toString() << "prio" <<prio;
        requests.insert(uid, u);
    }
    else {
        qDebug() << "There is no DownloadManager set - aborting";
    }
}

void RequestHandler::requestFinished(qint64 uid) {
    QByteArray ba;
    QUrl url;

    ba = downloadManager->getByteArray(uid);
    url = requests.value(uid, QUrl("NONE"));

    requests.remove(uid);
    downloadManager->freeRequest(uid);

    emit response(url, ba);
}

void RequestHandler::error(qint64 req, int err) {
    QUrl url;

    url = requests.value(req, QUrl("NONE"));
    emit responseError(url, err);

    requests.remove(req);
    downloadManager->freeRequest(req);
}

void RequestHandler::cancel(QUrl url) {
    QList<qint64> uids;

    uids = requests.keys();

    foreach(qint64 id, uids) {
        if (requests.value(id) == url) {
            downloadManager->removeRequest(id);
        }
    }
}

void RequestHandler::cancelAll() {
    QList<qint64> uids;

    uids = requests.keys();

    foreach(qint64 id, uids) {
        downloadManager->removeRequest(id);
    }
}