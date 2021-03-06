#ifndef UPDATER_H
#define UPDATER_H

#include <QObject>
#include "applicationinterface.h"
#include "downloadmanager.h"
#include "filehandler.h"
#include "types.h"

extern QTextStream* output;
extern QTextStream* foutput;

class Updater : public QObject
{
    Q_OBJECT
public:
    explicit Updater(QObject *parent = 0);
private:
    ApplicationInterface* ai;
    DownloadManager* dm;
    FileHandler* fh;
    bool finishedDownload;
    bool exchanging;
    bool updateFinished;

    void p(QString);
    QList<FileUpdate> updateList;
    QString executable;

signals:

public slots:
    void run();
public:
    bool finishedUpdate() {return updateFinished;}

private slots:
    void startUpdate(QList<FileUpdate>);
    void downloadFinished(QList<FileUpdate>);
    void startExchange(bool);
    void exchangeFinished(bool);
    void setExecutable(QString);
    void closeRequested();

};

#endif // UPDATER_H
