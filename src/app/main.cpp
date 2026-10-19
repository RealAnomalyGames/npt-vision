#include "gui/MainWindow.h"

#include <QApplication>
#include <QCoreApplication>

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    QCoreApplication::setApplicationName("NPT Vision");
    QCoreApplication::setApplicationVersion("0.1.0");
    QCoreApplication::setOrganizationName("NPT Software");

    MainWindow window;
    window.show();

    return app.exec();
}
