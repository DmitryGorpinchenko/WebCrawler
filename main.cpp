#include "window.h"

#include <QApplication>
#include <QMutex>

QWaitCondition cond;
QMutex mutex;
bool paused = false;
bool stoped = false;

int main(int argc, char **argv)
{
    QApplication app(argc, argv);

    Window window;
    window.show();

    return app.exec();
}
