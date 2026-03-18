#include <QApplication>
#include "InvasionView.hh"
#include <iostream>

int main(int argc, char* argv[]){
    QApplication app(argc, argv);

    InvasionView view;
    view.show();

    std::cout << "hi" <<std::endl;;
    return app.exec();
}
