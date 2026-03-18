#include <QApplication>
#include "InvasionView.h"

int main(int argc, char* argv[]){
    QApplication app(argc, argv);

    InvasionView view;
    view.show();

    return app.exec();
}
