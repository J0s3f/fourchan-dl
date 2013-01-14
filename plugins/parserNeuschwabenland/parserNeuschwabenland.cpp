#include "parserNeuschwabenland.h"

ParserNeuschwabenland::ParserNeuschwabenland()
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

QString ParserNeuschwabenland::getAuthor() {
  return "Josef Schneider";
}

QString ParserNeuschwabenland::getPluginName() {
  return "neuschwabenland.org Parser";
}

QString ParserNeuschwabenland::getVersion() {
  return _LIB_VERSION;
}

QString ParserNeuschwabenland::getDomain() {
  return "neuschwabenland.org";
}

ParsingStatus ParserNeuschwabenland::parseHTML(QString html) {
  QStringList res;
  QRegExp rxImages("<span class=\"filesize\">[^<]+<a [^>]+>[^<]+</a>[^<]+<a href=\"([^\"]+)\" [^>]+>([^<]+)</a>[^<]*</span>[^<]*<br>[^<]*<a href=\"([^\"]+)\" [^>]*>[^<]*<span [^>]*>[^<]*<img class=\"thumb\"[^>]*src=\"([^\"]+)\">", Qt::CaseInsensitive, QRegExp::RegExp2);
  QRegExp rxThreads("<a href=\"([^\"]+)\"[^>]*>Antworten</a>", Qt::CaseInsensitive, QRegExp::RegExp2);
  QRegExp rxTitle("<span class=\"postsubject\">([^<]+)</span>");
  QRegExp rxThreadPage("<h2>Antwortmodus\\s+\\(Thread \\d+\\)</h2>");

  bool imagesAdded;
  bool pageIsFrontpage;
  int pos;
  _IMAGE i;
  QUrl u;
  QString sUrl;

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
  pageIsFrontpage = !html.contains(rxThreadPage);

  if (pageIsFrontpage) {
      pos = 0;
      _statusCode.isFrontpage = true;

      while (pos > -1) {
          pos = rxThreads.indexIn(html, pos + 1);
          res = rxThreads.capturedTexts();

          if (!res.at(1).isEmpty()) {
              sUrl = "http://krautchan.net"+res.at(1);
              if (sUrl.endsWith("/")) {
                  sUrl.remove(sUrl.length()-1,1);
                }

              _urlList.append(QUrl(sUrl));
            }
        }
    }
  else {
      // Checking for Images

      //extract name, url and thumbnail url for all images
      pos = rxImages.indexIn(html);
      while (pos > -1) {
          i.originalFilename = i.thumbURI = i.largeURI = "";
          res = rxImageDiv.capturedTexts();
          i.largeURI = "https://neuschwabenland.org/"+ res.at(1);
          i.originalFilename = res.at(2);
//        i.thumbURI = "https://neuschwabenland.org/"+res.at(3);
          _images.append(i);
          _statusCode.hasImages = true;

          pos = rxImages.indexIn(html, pos+1);
        }

      pos = rxTitle.indexIn(html);
      while (pos > -1) {
          pos = rxTitle.indexIn(html,pos+1);
          res = rxTitle.capturedTexts();

          if (pos > -1 && res.at(1) != "") {
              _threadTitle = res.at(1);
              _statusCode.hasTitle = true;
              pos = -1;
            }
        }
    }

  return _statusCode;
}

QString ParserNeuschwabenland::getThreadTitle() {
  return _threadTitle;
}

QList<_IMAGE> ParserNeuschwabenland::getImageList() {
  return _images;
}

QUrl ParserNeuschwabenland::getRedirectURL() {
  return _redirect;
}

int ParserNeuschwabenland::getErrorCode() {
  return _errorCode;
}

ParsingStatus ParserNeuschwabenland::getStatusCode() {
  return _statusCode;
}

QList<QUrl> ParserNeuschwabenland::getUrlList() {
  return _urlList;
}

void ParserNeuschwabenland::setURL(QUrl url) {
  _url = url;

  boardName = _url.path().section("/",1,1);
  threadNumber = "";
  if (_url.path().contains("thread")) {
      QRegExp rxTid("/thread-(\\d+)\\.html");
      int pos = rxTid.indexIn(_url.path());
      QStringList res = rxTid.capturedTexts();

      if (pos > -1 && res.at(1) != "") {
          threadNumber = res.at(1);
        }
    }
}

QString ParserNeuschwabenland::parseSavepath(QString s) {
  s.replace("%n", threadNumber);
  s.replace("%b", boardName);
  s.replace("%h", _url.host());

  return s;
}

QMap<QString, QString> ParserNeuschwabenland::getSupportedReplaceCharacters() {
  QMap<QString, QString> ret;

  ret.insert("%n", "Threadnumber");
  ret.insert("%b", "Board");
  ret.insert("%h", "Host");

  return ret;
}

#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2(pParserNeuschwabenland, ParserNeuschwabenland)
#endif
