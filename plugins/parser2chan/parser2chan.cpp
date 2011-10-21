#include "parser2chan.h"

Parser2chan::Parser2chan()
{
    _html = "";
    _statusCode.hasErrors = false;
    _statusCode.hasImages = false;
    _statusCode.hasTitle = false;
    _statusCode.isFrontpage = false;
    _errorCode = 0;
    _redirect = QUrl();
    _images.clear();
}

QString Parser2chan::getAuthor() {
    return "Florian Born";
}

QString Parser2chan::getPluginName() {
    return "2chan.net Parser";
}

QString Parser2chan::getVersion() {
    return _LIB_VERSION;
}

QString Parser2chan::getDomain() {
    return "2chan.net";
}

ParsingStatus Parser2chan::parseHTML(QString html) {
    QStringList res;
    QRegExp rx("</small>([\\n|\\r|\\t]*)<br>([\\n|\\r|\\t]*)<a href=\"http://([^\\.])+\\.2chan\\.net/([^\"]+)\"(?:[^<]+)<img src=\"([^\\s]+)\"(?:[^<]+)</a>", Qt::CaseInsensitive, QRegExp::RegExp2);
    QRegExp boardPage("<a href=\"res/(\\d+)\">Reply</a>", Qt::CaseSensitive, QRegExp::RegExp2);
    QRegExp rxTitle("<span class=\"filetitle\">([^<]+)</span>");
    bool imagesAdded;
    bool pageIsFrontpage;
    int pos;
    _IMAGE i;
    QString tempFilename;
    QUrl u;
    QString sUrl;
    QString boardName;

    _html = html;
    _images.clear();
    _redirect.clear();
    _urlList.clear();
    _statusCode.hasErrors = false;
    _statusCode.hasImages = false;
    _statusCode.hasTitle = false;
    _statusCode.isFrontpage = false;

    imagesAdded = false;
    pos = 0;
    i.downloaded = false;
    i.requested = false;
    pageIsFrontpage = false;

    while (pos > -1) {
        pos = boardPage.indexIn(html, pos+1);
        res = boardPage.capturedTexts();

        if (res.at(1) != "") {
            pageIsFrontpage=true;

            u.setUrl(QString("res/%1").arg(res.at(1)));

            // build complete url
            boardName = _url.path().section("/", 1, 1);

            if (u.isRelative()) {
                sUrl = "";
                if (u.path().startsWith("/")) {
                    // We need to complete only the host
                    sUrl = QString("%1/%2").arg(_url.host()).arg(u.path());
                }
                else if (u.path().startsWith("res")) {
                    sUrl = QString("http://boards.2chan.net/%1/%2").arg(boardName).arg(u.path());
                }
                else {
                    qDebug() << "Parsing front page and don't know what to do. Found url" << u.toString();
                }
            }

            _urlList << QUrl("res/"+res.at(1));
            _statusCode.isFrontpage = true;
        }
    }

    pos = 0;
    while (pos > -1) {
        pos = rx.indexIn(html, pos+1);
        res = rx.capturedTexts();

        tempFilename = res.at(4);

        i.originalFilename = tempFilename.right(tempFilename.count() - tempFilename.lastIndexOf("/") - 1);
        i.largeURI = "http://sep.2chan.net/"+res.at(4);
        i.thumbURI = res.at(5);

        if (pos != -1) {
            _images.append(i);
            _statusCode.hasImages = true;
        }
    }

    pos = 0;
    while (pos > -1) {
        pos = rxTitle.indexIn(html,pos+1);
        res = rxTitle.capturedTexts();

        if (res.at(1) != "") {
            _threadTitle = res.at(1);
            _statusCode.hasTitle = true;
        }

    }

    return _statusCode;
}

QString Parser2chan::getThreadTitle() {
    return _threadTitle;
}

QList<_IMAGE> Parser2chan::getImageList() {
    return _images;
}

QUrl Parser2chan::getRedirectURL() {
    return _redirect;
}

int Parser2chan::getErrorCode() {
    return _errorCode;
}

ParsingStatus Parser2chan::getStatusCode() {
    return _statusCode;
}

QList<QUrl> Parser2chan::getUrlList() {
    return _urlList;
}

void Parser2chan::setURL(QUrl url) {
    _url = url;
}

Q_EXPORT_PLUGIN2(pParser2chan, Parser2chan)