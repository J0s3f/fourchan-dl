#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    uiConfig = new UIConfig(this);
    uiInfo = new UIInfo(this);
    threadAdder = new UIThreadAdder(this);
    aui = new ApplicationUpdateInterface(this);
    manager = new QNetworkAccessManager();
    blackList = new BlackList(this);
    thumbnailRemover = new ThumbnailRemoverThread(this);

    thumbnailRemover->start(QThread::LowPriority);

    overviewUpdateTimer = new QTimer(this);
    overviewUpdateTimer->setInterval(1000);
    overviewUpdateTimer->setSingleShot(true);
    _updateOverview = false;

    connect(this, SIGNAL(removeFiles(QStringList)), thumbnailRemover, SLOT(removeFiles(QStringList)));

//    downloadManager = new DownloadManager(this);
//    downloadManager->start(QThread::NormalPriority);

    ui->setupUi(this);

    // Adding actions to menu bar
    /*
    ui->menuBar->addAction(ui->actionAdd_Tab);
    ui->menuBar->addAction(ui->actionAddMultipleTabs);
    ui->menuBar->addAction(ui->actionTabOverview);

    historyMenu->setTitle("History");
    historyMenu->setIcon(QIcon(":/icons/resources/remove.png"));
    ui->menuBar->addMenu(historyMenu);

    ui->menuBar->addAction(ui->actionStart_all);
    ui->menuBar->addAction(ui->actionStop_all);

    ui->menuBar->addAction(ui->actionOpen_Configuration);
    */
    ui->menuBar->addAction(ui->actionShowInfo);

    settings = new QSettings("settings.ini", QSettings::IniFormat);
    ui->tabWidget->removeTab(0);
    oldActiveTabIndex = 0;
    pendingThumbnailsChanged(0);

    // Thread overview
    connect(ui->dockWidget, SIGNAL(visibilityChanged(bool)), ui->actionTabOverview, SLOT(setChecked(bool)));
    connect(ui->actionTabOverview, SIGNAL(triggered()), this, SLOT(scheduleOverviewUpdate()));

    loadOptions();
    restoreWindowSettings();
    updateWidgetSettings();

    connect(manager, SIGNAL(finished(QNetworkReply*)),
            this, SLOT(replyFinished(QNetworkReply*)));
    connect(ui->tabWidget, SIGNAL(tabCloseRequested(int)), this, SLOT(closeTab(int)));
    connect(uiConfig, SIGNAL(configurationChanged()), this, SLOT(loadOptions()));
    connect(uiConfig, SIGNAL(configurationChanged()), blackList, SLOT(loadSettings()));
    connect(uiConfig, SIGNAL(configurationChanged()), downloadManager, SLOT(loadSettings()));
    connect(uiConfig, SIGNAL(deleteAllThumbnails()), thumbnailRemover, SLOT(removeAll()));
    connect(ui->actionStart_all, SIGNAL(triggered()), this, SLOT(startAll()));
    connect(ui->actionStop_all, SIGNAL(triggered()), this, SLOT(stopAll()));
    connect(threadAdder, SIGNAL(addTab(QString)), this, SLOT(createTab(QString)));
    connect(downloadManager, SIGNAL(error(QString)), ui->statusBar, SLOT(showMessage(QString)));

    connect(overviewUpdateTimer, SIGNAL(timeout()), this, SLOT(overviewTimerTimeout()));
    connect(ui->menuHistory, SIGNAL(triggered(QAction*)), this, SLOT(restoreFromHistory(QAction*)));

//    if (tnt->isRunning()) {
        connect(tnt, SIGNAL(pendingThumbnails(int)), ui->pbPendingThumbnails, SLOT(setValue(int)));
        connect(tnt, SIGNAL(pendingThumbnails(int)), this, SLOT(pendingThumbnailsChanged(int)));
//    }
//    connect(downloadManager, SIGNAL(totalRequestsChanged(int)), ui->pbOpenRequests, SLOT(setMaximum(int)));
//    connect(downloadManager, SIGNAL(finishedRequestsChanged(int)), ui->pbOpenRequests, SLOT(setValue(int)));

    manager->get(QNetworkRequest(QUrl("http://sourceforge.net/projects/fourchan-dl/files/")));
    ui->mainToolBar->setVisible(false);
}

MainWindow::~MainWindow()
{
    saveSettings();

    delete ui;
}

int MainWindow::addTab() {
    int ci;
    UI4chan* w;

    w = new UI4chan(this);
    w->setBlackList(blackList);

//    w->setDownloadManager(downloadManager);

    ci = ui->tabWidget->addTab(w, "no name");
    if (settings->value("options/remember_directory", false).toBool())
        w->setDirectory(defaultDirectory);

    connect(w, SIGNAL(errorMessage(QString)), this, SLOT(displayError(QString)));
    connect(w, SIGNAL(tabTitleChanged(UI4chan*, QString)), this, SLOT(changeTabTitle(UI4chan*, QString)));
    connect(w, SIGNAL(closeRequest(UI4chan*, int)), this, SLOT(processCloseRequest(UI4chan*, int)));
    connect(w, SIGNAL(directoryChanged(QString)), this, SLOT(setDefaultDirectory(QString)));
    connect(w, SIGNAL(createTabRequest(QString)), this, SLOT(createTab(QString)));
    connect(w, SIGNAL(removeFiles(QStringList)), this, SIGNAL(removeFiles(QStringList)));
    connect(w, SIGNAL(changed()), this, SLOT(scheduleOverviewUpdate()));

    ui->tabWidget->setCurrentIndex(ci);

    changeTabTitle(w, "idle");

    return ci;
}

void MainWindow::createTab(QString s) {
    int index;
    UI4chan* w;

    index = addTab();
    w = ((UI4chan*)ui->tabWidget->widget(index));
    w->setValues(s);
    w->setAttribute(Qt::WA_DeleteOnClose, true);
}

void MainWindow::closeTab(int i) {
    UI4chan* w;

    ui->tabWidget->setCurrentIndex(i);
    w = (UI4chan*)ui->tabWidget->widget(i);

    addToHistory(w->getValues(), w->getTitle());

    if (w->close()) {
        ui->tabWidget->removeTab(i);
        w->deleteLater();
    }
    else
        qDebug() << "Close widget event not accepted";

    if (ui->tabWidget->count() == 0) {
        addTab();
    }
}

void MainWindow::displayError(QString s) {
    ui->statusBar->showMessage(s, 10000);
}

void MainWindow::showInfo(void) {
    uiInfo->show();
}

void MainWindow::showConfiguration(void) {
    uiConfig->show();
}

void MainWindow::setDefaultDirectory(QString d) {
    if (settings->value("options/remember_directory", false).toBool())
        defaultDirectory = d;
}

void MainWindow::changeTabTitle(UI4chan* w, QString s) {
    int i;

    i = ui->tabWidget->indexOf((QWidget*)w);
    ui->tabWidget->setTabText(i, s);
}

void MainWindow::restoreWindowSettings(void) {
    // Restore window position
    QPoint p;
    QSize s;
    QByteArray ba;
    int state;

    settings->beginGroup("window");
        p = settings->value("position",QPoint(0,0)).toPoint();
        state = settings->value("state",0).toInt();
        s = settings->value("size",QSize(0,0)).toSize();
        ba = settings->value("widgetstate", QByteArray()).toByteArray();
    settings->endGroup();

    if (p != QPoint(0,0))
        this->move(p);

    if (s != QSize(0,0))
        this->resize(s);

    if (state != Qt::WindowNoState)
        this->setWindowState((Qt::WindowState) state);

    if (!ba.isEmpty())
        this->restoreState(ba);
}

void MainWindow::restoreTabs() {
    int tabCount;

    tabCount = settings->value("tabs/count",0).toInt();

    ui->pbOpenRequests->setMaximum(tabCount);
    ui->pbOpenRequests->setValue(0);
    ui->pbOpenRequests->setFormat("Opening tab %v/%m");
    ui->tabWidget->setVisible(false);
    if (settings->value("options/resume_session", false).toBool() && tabCount > 0) {
        int ci;

        for (int i=0; i<tabCount; i++) {
            ci = addTab();

            ((UI4chan*)ui->tabWidget->widget(ci))->setValues(
                    settings->value(QString("tabs/tab%1").arg(i), ";;;;0;;every 30 seconds;;0").toString()
                    );
            ui->pbOpenRequests->setValue((i+1));
        }
    } else {
        addTab();
    }

    ui->tabWidget->setVisible(true);
    ui->pbOpenRequests->setVisible(false);
//    ui->pbOpenRequests->setFormat("%v/%m (%p%) requests finished");
//    ui->pbOpenRequests->setValue(0);
//    ui->pbOpenRequests->setMaximum(0);
}

void MainWindow::saveSettings(void) {
    int downloadedFiles;
    float downloadedKB;
    // Window related stuff
    settings->beginGroup("window");
        settings->setValue("position", this->pos());
        if (this->windowState() == Qt::WindowNoState)
            settings->setValue("size", this->size());
        settings->setValue("state", QString("%1").arg(this->windowState()));
        settings->setValue("widgetstate", this->saveState());
    settings->endGroup();

    // Dock widget
    settings->beginGroup("thread_overview");
//    settings->setValue("size", ui->dockWidget->size());
    settings->setValue("col_uri_width", ui->threadOverview->columnWidth(0));
    settings->setValue("col_name_width", ui->threadOverview->columnWidth(1));
    settings->setValue("col_images_width", ui->threadOverview->columnWidth(2));
    settings->setValue("col_status_width", ui->threadOverview->columnWidth(3));
    settings->endGroup();

    // Options
    settings->beginGroup("options");
    settings->endGroup();

    // Active tabs
    settings->remove("tabs");   // Clean up
    settings->beginGroup("tabs");
        settings->setValue("count", ui->tabWidget->count());

        for (int i=0; i<ui->tabWidget->count(); i++) {
            settings->setValue(QString("tab%1").arg(i), ((UI4chan*)ui->tabWidget->widget(i))->getValues());
        }
    settings->endGroup();

    downloadManager->getStatistics(&downloadedFiles, &downloadedKB);
    settings->beginGroup("statistics");
        settings->setValue("downloaded_files", downloadedFiles);
        settings->setValue("downloaded_kbytes", downloadedKB);
    settings->endGroup();

    settings->sync();
}

void MainWindow::loadOptions(void) {
    settings->beginGroup("options");
        defaultDirectory = settings->value("default_directory", "").toString();
        ui->tabWidget->setTabPosition((QTabWidget::TabPosition)settings->value("tab_position", 3).toInt());
        autoClose = settings->value("automatic_close", false).toBool();
        thumbnailSize.setWidth(settings->value("thumbnail_width", 200).toInt());
        thumbnailSize.setHeight(settings->value("thumbnail_height", 200).toInt());
        maxDownloads = settings->value("concurrent_downloads", 1).toInt();

        updateWidgetSettings();
        tnt->setIconSize(QSize(settings->value("thumbnail_width",150).toInt(),settings->value("thumbnail_height",150).toInt()));
    settings->endGroup();

    settings->beginGroup("network");
    QNetworkProxy proxy;

    if (settings->value("use_proxy", false).toBool()) {
        proxy.setType((QNetworkProxy::ProxyType)(settings->value("proxy_type", QNetworkProxy::HttpProxy).toInt()));
        proxy.setHostName(settings->value("proxy_hostname", "").toString());
        proxy.setPort(settings->value("proxy_port", 0).toUInt());
        if (settings->value("proxy_auth", false).toBool()) {
            proxy.setUser(settings->value("proxy_user", "").toString());
            proxy.setPassword(settings->value("proxy_pass", "").toString());
        }
    }
    else {
        proxy.setType(QNetworkProxy::NoProxy);
    }

    QNetworkProxy::setApplicationProxy(proxy);

    settings->endGroup();

    // Dock widget
    settings->beginGroup("thread_overview");
//    ui->dockWidget->resize();
//    settings->setValue("width", ui->dockWidget->width());
    ui->threadOverview->setColumnWidth(0, settings->value("col_uri_width", 170).toInt());
    ui->threadOverview->setColumnWidth(1, settings->value("col_name_width", 190).toInt());
    ui->threadOverview->setColumnWidth(2, settings->value("col_images_width", 60).toInt());
    ui->threadOverview->setColumnWidth(3, settings->value("col_status_width", 70).toInt());
    settings->endGroup();

}

void MainWindow::processCloseRequest(UI4chan* w, int reason) {
    int i;
    i = ui->tabWidget->indexOf((QWidget*)w);

    if (reason == 404) {
        if (settings->value("options/automatic_close", false).toBool()) {
            closeTab(i);
        }
    }
    else {
        closeTab(i);
    }
}

void MainWindow::replyFinished(QNetworkReply* r) {
    QString requestURI;
    QRegExp rx("Current version ([0-9\\.]+)", Qt::CaseInsensitive, QRegExp::RegExp2);
    QString html;
    QStringList res;
    int pos;

    if (r->isFinished()) {
        requestURI = r->request().url().toString();

        html = r->readAll();

        pos = rx.indexIn(html);
        res = rx.capturedTexts();

        if (pos != -1) {
           uiInfo->setCurrentVersion(res.at(1));
           checkVersion(res.at(1));
        }
    }

    r->deleteLater();
}


void MainWindow::updateWidgetSettings(void) {
    for (int i=0; i<ui->tabWidget->count(); i++) {
        ((UI4chan*)ui->tabWidget->widget(i))->updateSettings();
    }
}

void MainWindow::checkVersion(QString ver) {
    QStringList currVersion, thisVersion;

    currVersion = ver.split(".");
    thisVersion = QString(PROGRAM_VERSION).split(".");

    for (int i=0; i<currVersion.count(); i++) {
        if (currVersion.value(i).toInt() > thisVersion.at(i).toInt()) {
            newVersionAvailable(ver);
            break;
        }
    }
}

void MainWindow::newVersionAvailable(QString v) {
    QProcess process;
    QFileInfo fi;

    ui->statusBar->showMessage(QString("There is a new version (%1) available to download from sourceforge.").arg(v));
#ifdef USE_UPDATER
    switch (QMessageBox::question(0,"New version available",
                                  QString("There is a new version (%1) available from sourceforge<br><a href=\"http://sourceforge.net/projects/fourchan-dl/files/\">sourceforge.net/projects/fourchan-dl</a><br />Do you want to update now?").arg(v),
                                  QMessageBox::Yes | QMessageBox::No)) {
    case QMessageBox::Ok:
    case QMessageBox::Yes:

        fi.setFile(UPDATER_NAME);

        qDebug() << "Starting updater " << fi.absoluteFilePath();

        if (process.startDetached(QString("\"%1\"").arg(fi.absoluteFilePath()))) {
            ui->statusBar->showMessage("Starting updater");
        }
        else {
            ui->statusBar->showMessage("Unable to start process "+fi.absoluteFilePath()+" ("+process.errorString()+")");
        }
        aui->startUpdate(v);
        break;

    case QMessageBox::No:
    default:
        break;
    }
#else
    QMessageBox::information(0,
                             "New version available",
                             QString("There is a new version (%1) available from sourceforge<br><a href=\"http://sourceforge.net/projects/fourchan-dl/files/\">sourceforge.net/projects/fourchan-dl</a>").arg(v),
                             QMessageBox::Ok);
#endif
}

void MainWindow::startAll() {
    ui->pbOpenRequests->setFormat("Starting Thread %v/%m (%p%)");
    ui->pbOpenRequests->setMaximum(ui->tabWidget->count());

    for (int i=0; i<ui->tabWidget->count(); i++) {
        ((UI4chan*)ui->tabWidget->widget(i))->start();
        ui->pbOpenRequests->setValue((i+1));
    }
}

void MainWindow::stopAll() {
    ui->pbOpenRequests->setFormat("Stopping Thread %v/%m (%p%)");
    ui->pbOpenRequests->setMaximum(ui->tabWidget->count());

    for (int i=0; i<ui->tabWidget->count(); i++) {
        ((UI4chan*)ui->tabWidget->widget(i))->stop();
        ui->pbOpenRequests->setValue((i+1));
    }
}

void MainWindow::pendingThumbnailsChanged(int i) {
    if (i > ui->pbPendingThumbnails->maximum())
        ui->pbPendingThumbnails->setMaximum(i);

    if (i == 0) {
        ui->pbPendingThumbnails->setVisible(false);
        ui->pbPendingThumbnails->setMaximum(0);
    }
    else if (ui->pbPendingThumbnails->maximum() > 4)
        ui->pbPendingThumbnails->setVisible(true);

}

void MainWindow::addMultipleTabs() {
    threadAdder->show();
}

void MainWindow::showTab(QTreeWidgetItem* item, int column) {
    int index;

    index = ui->threadOverview->indexOfTopLevelItem(item);

    if (index != -1) {
        ui->tabWidget->setCurrentIndex(index);
    }
}

void MainWindow::scheduleOverviewUpdate() {
    if (!overviewUpdateTimer->isActive()) {
        updateThreadOverview();
        overviewUpdateTimer->start();
    }
    else {
         _updateOverview = true;
    }
}

void MainWindow::overviewTimerTimeout() {
    updateThreadOverview();
    if (_updateOverview) {
        _updateOverview = false;
        overviewUpdateTimer->start();
    }
}

void MainWindow::updateThreadOverview() {
//    QList<QTreeWidgetItem *> items;
    QStringList sl;

    if (ui->threadOverview->isVisible()) {
//        qDebug() << "updating thread overview";
//        ui->threadOverview->clear();

        for (int i=0; i<ui->tabWidget->count(); i++) {
            UI4chan* tab;
            QTreeWidgetItem* item;
            sl.clear();

            tab = (UI4chan*)(ui->tabWidget->widget(i));
            sl << tab->getURI();
            sl << tab->getTitle();
            sl << QString("%1/%2").arg(tab->getDownloadedCount()).arg(tab->getTotalCount());
            sl << tab->getStatus();

            if (ui->threadOverview->topLevelItemCount() > i) {                   // If there is an entry for the i-th tab
                item = ui->threadOverview->topLevelItem(i);     //  change its content
                for (int k=0; k<4; k++)
                    item->setText(k, sl.at(k));
            }
            else {                                              // Otherwise create a new one and append it
                ui->threadOverview->addTopLevelItem(new QTreeWidgetItem(ui->threadOverview, sl));
            }
        }

        // Remove obsolete rows of overview (if any)
        if (ui->threadOverview->topLevelItemCount() > ui->tabWidget->count()) {
            for (int i=ui->threadOverview->topLevelItemCount(); i>=ui->tabWidget->count(); --i) {
                ui->threadOverview->takeTopLevelItem(i);
            }
        }
//        ui->threadOverview->insertTopLevelItems(0, items);
    }
}

void MainWindow::debugButton() {
    updateThreadOverview();
}

bool MainWindow::threadExists(QString url) {
    bool ret;
    int count;

    ret = false;
    count = 0;

    for (int i=0; i<ui->tabWidget->count(); i++) {
        if (((UI4chan*)ui->tabWidget->widget(i))->getURI() == url)
            count++;
    }

    if (count > 1)      // Check if url was found more than 1 time, because requesting thread is also counted
        ret = true;

    return ret;
}

void MainWindow::addToHistory(QString s, QString title="") {
    QStringList sl;
    QString key;
    QString actionTitle;
    QAction* a;

    sl = s.split(";;");
    if (sl.count() > 0) {
        key = sl.at(0);
        if (!key.isEmpty()) {
            historyList.insert(key, s);

            if (title.isEmpty())
                actionTitle = QString("%1 -> %2").arg(key).arg(sl.at(1));
            else
                actionTitle = QString("%1 (%2) -> %3").arg(key).arg(title).arg(sl.at(1));

            a = ui->menuHistory->addAction(actionTitle);
            a->setIcon(QIcon(":/icons/resources/reload.png"));
            a->setStatusTip(QString("Reopen thread %1").arg(key));
            a->setToolTip(key); // Abusing the tooltip for saving the historyList key
            //        qDebug() << QString("Adding (%1, %2) to history").arg(key).arg(s);
        }
    }
}

void MainWindow::removeFromHistory(QString key) {
    historyList.remove(key);
//    qDebug() << QString("Removing (%1) from history").arg(key);
}

void MainWindow::restoreFromHistory(QAction* a) {
    QString key;

    key = a->toolTip();

    if (historyList.count(key) > 0)
        createTab(historyList.value(key, ""));

    removeFromHistory(key);
    ui->menuHistory->removeAction(a);
}

void MainWindow::keyPressEvent(QKeyEvent *event) {
    QMainWindow::keyPressEvent(event);

    if (event->key() == Qt::Key_W && (event->modifiers() & Qt::ControlModifier) == Qt::ControlModifier) {
        closeTab(ui->tabWidget->currentIndex());
    }
}
