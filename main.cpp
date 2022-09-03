#include "window.h"
#include "crawler.h"

#include <QApplication>

int main(int argc, char **argv)
{
    QApplication app(argc, argv);

    qRegisterMetaType<RequestStatus>("RequestStatus");

    Window window;
    window.show();

    return app.exec();
}
