#ifndef FTPDIALOG_H
#define FTPDIALOG_H

#include <QDialog>
#include <QUrl>
#include <QFtp>
#include <QTreeWidgetItem>
#include <QFile>

namespace Ui {
class FtpDialog;
}

class FtpDialog : public QDialog
{
    Q_OBJECT

public:
    explicit FtpDialog(QWidget *parent = nullptr);
    ~FtpDialog();

private:
    Ui::FtpDialog *ui;

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

#endif // FTPDIALOG_H
