#include <QApplication>
#include "InvasionView.h"
#include "Sprite.h"

int main(int argc, char* argv[]){
    QApplication app(argc, argv);

    InvasionView view;

    Sprite *sprite = new Sprite();
    view.addItem(sprite);
    view.show();

    return app.exec();
}
