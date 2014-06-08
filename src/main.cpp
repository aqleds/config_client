#include <QCoreApplication>
#include <QApplication>
#include "aqleds.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    AqLeds w;
    w.show();

    QCoreApplication::setOrganizationName("AqLeds");
    QCoreApplication::setOrganizationDomain("aqleds.com");
    QCoreApplication::setApplicationName("AqLed Manager");


    //settings.setValue("editor/wrapMargin", 68);
    //  int margin = settings.value("editor/wrapMargin").toInt();


    return a.exec();
}
