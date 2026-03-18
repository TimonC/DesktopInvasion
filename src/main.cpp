#include <QGuiApplication>
#include "InvasionOverlay.h"
#include <QQuickItem>
#include <QDebug>

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    InvasionOverlay overlay;
    overlay.show();

    return app.exec();
}

