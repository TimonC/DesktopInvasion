#include "InvasionView.hh"
#include <QApplication>
#include <QRect>
#include <QRectF>
#include <QScreen>
#include <QGraphicsScene>
#include <qnamespace.h>
InvasionView::InvasionView(){

    Qt::WindowFlags flags = Qt::WindowType::FramelessWindowHint
                          | Qt::WindowType::WindowStaysOnTopHint
                          | Qt::WindowType::Tool
                          | Qt::WindowType::WindowTransparentForInput;
    this -> setWindowFlags(flags);
    this -> setAttribute(Qt::WidgetAttribute::WA_TranslucentBackground);
    this -> setAttribute(Qt::WidgetAttribute::WA_ShowWithoutActivating);

    QScreen *screen = QApplication::primaryScreen();
    QRect geometry = screen->geometry();
    this -> setGeometry(geometry);

    scene = new QGraphicsScene(this);
    scene->setSceneRect(QRectF(geometry));
    scene->setBackgroundBrush(Qt::GlobalColor::transparent);
    this->setScene(scene);
    this->setStyleSheet("background: transparent;");

    this->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    this->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
}
