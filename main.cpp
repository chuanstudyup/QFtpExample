
#include <QApplication>
#include "ftpdialog.h"
#include "ftpwidget.h"
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    FtpDialog w;
    w.show();
    FtpWidget c;
    c.show();
    return a.exec();
}
