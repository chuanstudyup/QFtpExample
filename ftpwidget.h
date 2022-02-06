#ifndef FTPWIDGET_H
#define FTPWIDGET_H

#include <QWidget>
#include <QUrl>
#include <QFtp>
#include <QTreeWidgetItem>
#include <QFile>

namespace Ui {
class FtpWidget;
}

class FtpWidget : public QWidget
{
    Q_OBJECT

public:
    explicit FtpWidget(QWidget *parent = nullptr);
    ~FtpWidget();

private:
    Ui::FtpWidget *ui;

private slots:

    void downloadFile();
    void cancelDownload();
    void connectToFtp();

    void ftpCommandFinished(int commandId, bool error);
    void addToList(const QUrlInfo &urlInfo);
    void processItem(QTreeWidgetItem *item, int column);
    void cdToParent();
    void updateDataTransferProgress(qint64 readBytes, qint64 totalBytes);
    void enableDownloadButton();

    void connectOrDisconnect();

private:
    QHash<QString, bool> isDirectory;
    QString currentPath;
    QFtp *ftp = nullptr;
    QFile *file = nullptr;
};

#endif // FTPWIDGET_H
