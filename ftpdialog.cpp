#include "ftpdialog.h"
#include "ui_ftpdialog.h"

#include <QMessageBox>

FtpDialog::FtpDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::FtpDialog)
{
    ui->setupUi(this);
    this -> ui -> label_status -> setText("Please enter the name of an FTP server.");

    this -> ui -> treeWidget_fileList -> setEnabled(false);
    this -> ui -> treeWidget_fileList -> setRootIsDecorated(false);
    this -> ui -> treeWidget_fileList -> setHeaderLabels(QStringList() << tr("Name") << tr("Size") << tr("Owner") << tr("Group") << tr("Time"));
    this -> ui -> treeWidget_fileList -> header() ->setStretchLastSection(false);

    this -> ui ->pushButton_cdToParent -> setIcon(QPixmap(":/images/images/cdtoparent.png"));
    this -> ui ->pushButton_cdToParent -> setEnabled(false);

    this -> ui ->pushButton_download -> setEnabled(false);

    connect(ui->treeWidget_fileList, SIGNAL(itemActivated(QTreeWidgetItem*,int)),
            this, SLOT(processItem(QTreeWidgetItem*,int)));
    connect(ui->treeWidget_fileList, SIGNAL(currentItemChanged(QTreeWidgetItem*,QTreeWidgetItem*)),
            this, SLOT(enableDownloadButton()));
    connect(ui->pushButton_connect, SIGNAL(clicked()), this, SLOT(connectOrDisconnect()));
    connect(ui->pushButton_cdToParent, SIGNAL(clicked()), this, SLOT(cdToParent()));
    connect(ui->pushButton_download, SIGNAL(clicked()), this, SLOT(downloadFile()));
    connect(ui->pushButton_quit, SIGNAL(clicked()), this, SLOT(close()));

    setWindowTitle("FTPDialog");
}

FtpDialog::~FtpDialog()
{
    delete ui;
}

void FtpDialog::connectOrDisconnect()
{
    if (ftp) {
        ftp -> abort();
        ftp -> deleteLater();
        ftp = nullptr;

        this -> ui -> treeWidget_fileList -> clear();
        this -> ui -> treeWidget_fileList -> setEnabled(false);
        this -> ui ->pushButton_cdToParent -> setEnabled(false);
        this -> ui ->pushButton_download -> setEnabled(false);
        this -> ui ->pushButton_connect -> setEnabled(true);
        this -> ui ->pushButton_connect -> setText(tr("Connect"));

        this -> ui -> label_status -> setText("Please enter the name of an FTP server.");
        return;
    }

    connectToFtp();
}


void FtpDialog::connectToFtp()
{
    ftp = new QFtp(this);
    connect(ftp, SIGNAL(commandFinished(int,bool)),
            this, SLOT(ftpCommandFinished(int,bool)));
    connect(ftp, SIGNAL(listInfo(QUrlInfo)),
            this, SLOT(addToList(QUrlInfo)));
    connect(ftp, SIGNAL(dataTransferProgress(qint64,qint64)),
            this, SLOT(updateDataTransferProgress(qint64,qint64)));

    this -> ui -> treeWidget_fileList ->clear();
    currentPath.clear();
    isDirectory.clear();

    QString ftpServer = this->ui->lineEdit_ftpServer->text();
    ftp -> connectToHost(ftpServer,21);
    ftp -> login("pi","302302302");

    this -> ui -> label_status ->setText(tr("Connecting to FTP server %1...").arg(this->ui->lineEdit_ftpServer->text()));
}

void FtpDialog::downloadFile()
{
    QString fileName = this -> ui -> treeWidget_fileList->currentItem()->text(0);
//![3]
//
    if (QFile::exists(fileName)) {
        QMessageBox::information(this, tr("FTP"),
                                 tr("There already exists a file called %1 in "
                                    "the current directory.")
                                 .arg(fileName));
        return;
    }

//![4]
    file = new QFile(fileName);
    if (!file->open(QIODevice::WriteOnly)) {
        QMessageBox::information(this, tr("FTP"),
                                 tr("Unable to save the file %1: %2.")
                                 .arg(fileName).arg(file->errorString()));
        delete file;
        return;
    }

    ftp->get(this -> ui -> treeWidget_fileList->currentItem()->text(0), file);
}

void FtpDialog::cancelDownload()
{
    ftp->abort();

    if (file->exists()) {
        file->close();
        file->remove();
    }
    delete file;
}

//![6]
void FtpDialog::ftpCommandFinished(int, bool error)
{

    if (ftp->currentCommand() == QFtp::ConnectToHost)
    {
        if (error) {
            QMessageBox::information(this, tr("FTP"),
                                     tr("Unable to connect to the FTP server "
                                        "at %1. Please check that the host "
                                        "name is correct.")
                                     .arg(this->ui->lineEdit_ftpServer->text()));
            connectOrDisconnect();
            return;
        }
        else {
            this -> ui -> label_status -> setText(tr("Logged onto %1.").arg(this->ui->lineEdit_ftpServer->text()));
            this -> ui -> treeWidget_fileList-> setFocus();
            this -> ui -> pushButton_download->setDefault(true);
            this -> ui -> pushButton_connect-> setText("Disconnect");
            return;
        }
    }
//![6]

//![7]
    if (ftp->currentCommand() == QFtp::Login)
        ftp->list();
//![7]

//![8]
    if (ftp->currentCommand() == QFtp::Get) {
        if (error) {
            this->ui->label_status->setText(tr("Canceled download of %1.")
                                 .arg(file->fileName()));
            file->close();
            file->remove();
        } else {
            this->ui->label_status->setText(tr("Downloaded %1 to current directory.")
                                 .arg(file->fileName()));
            this -> ui -> progressBar -> setValue(100);
            file->close();
        }
        delete file;
        enableDownloadButton();
//![8]
//![9]
    } else if (ftp->currentCommand() == QFtp::List) {
        if (isDirectory.isEmpty()) {
            this -> ui -> treeWidget_fileList -> addTopLevelItem(new QTreeWidgetItem(QStringList() << tr("<empty>")));
            this -> ui -> treeWidget_fileList -> setEnabled(false);
        }
    }
//![9]
}

void FtpDialog::addToList(const QUrlInfo &urlInfo)
{
    QTreeWidgetItem *item = new QTreeWidgetItem;
    item->setText(0, urlInfo.name());
    item->setText(1, QString::number(urlInfo.size()));
    item->setText(2, urlInfo.owner());
    item->setText(3, urlInfo.group());
    item->setText(4, urlInfo.lastModified().toString("MMM dd yyyy"));

    QPixmap pixmap(urlInfo.isDir() ? ":/images/images/dir.png" : ":/images/images/file.png");
    item->setIcon(0, pixmap);

    isDirectory[urlInfo.name()] = urlInfo.isDir();
    this -> ui -> treeWidget_fileList->addTopLevelItem(item);
    if (!this -> ui -> treeWidget_fileList->currentItem()) {
        this -> ui -> treeWidget_fileList->setCurrentItem(this -> ui -> treeWidget_fileList->topLevelItem(0));
        this -> ui -> treeWidget_fileList->setEnabled(true);
    }
}

void FtpDialog::processItem(QTreeWidgetItem *item, int /*column*/)
{
    QString name = item->text(0);
    if (isDirectory.value(name)) {
        this -> ui -> treeWidget_fileList->clear();
        isDirectory.clear();
        currentPath += '/';
        currentPath += name;
        ftp->cd(name);
        ftp->list();
        this-> ui -> pushButton_cdToParent->setEnabled(true);

        return;
    }
}

void FtpDialog::cdToParent()
{

    this -> ui -> treeWidget_fileList -> clear();
    isDirectory.clear();
    currentPath = currentPath.left(currentPath.lastIndexOf('/'));
    if (currentPath.isEmpty()) {
        this-> ui -> pushButton_cdToParent ->setEnabled(false);
        ftp->cd("/");
    } else {
        ftp->cd(currentPath);
    }
    ftp->list();
}


void FtpDialog::updateDataTransferProgress(qint64 readBytes,qint64 totalBytes)
{
    this -> ui -> progressBar -> setValue(static_cast<int>(readBytes/totalBytes));
}
//![13]

//![14]
void FtpDialog::enableDownloadButton()
{
    QTreeWidgetItem *current = this -> ui -> treeWidget_fileList->currentItem();
    if (current) {
        QString currentFile = current->text(0);
        this->ui->pushButton_download->setEnabled(!isDirectory.value(currentFile));
    } else {
        this->ui->pushButton_download->setEnabled(false);
    }
}
