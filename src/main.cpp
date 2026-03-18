#include <QGuiApplication>
#include <QDebug>
#include "InvasionOverlay.h"

int main(int argc, char *argv[])
{
    qDebug() << "Starting application...";
    QGuiApplication app(argc, argv);

    qDebug() << "Creating overlay...";
    InvasionOverlay overlay;

    qDebug() << "Showing overlay...";
    overlay.show();

    qDebug() << "Starting event loop...";
    return app.exec();
}
