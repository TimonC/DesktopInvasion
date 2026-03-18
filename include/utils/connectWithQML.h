#ifndef CONNECTWITHQML_H
#define CONNECTWITHQML_H

#include <QObject>
template<typename Signal, typename Slot>
void connectWithQML(QObject* signaler, Signal signal, Slot slot) {
    QObject* helper = new QObject(signaler);
    QObject::connect(signaler, signal, helper, SLOT(deleteLater()));
    QObject::connect(helper, &QObject::destroyed, slot);
}

#endif
