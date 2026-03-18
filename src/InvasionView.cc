#include "InvasionView.hh"

InvasionView::InvasionView(){
    Qt::WindowFlags flags = Qt::WindowType::FramelessWindowHint
                          | Qt::WindowType::WindowStaysOnTopHint
                          | Qt::WindowType::Tool
                          | Qt::WindowType::WindowTransparentForInput;

    this -> setWindowFlags(flags);
}
