#include "SingleInstanceApplication.h"
#include <QSharedMemory>
#include <QLocalServer>
#include <QDebug>

SingleInstanceApplication::SingleInstanceApplication(int &argc, char **argv, const QString &uniqueKey)
    : QApplication(argc, argv), m_uniqueKey(uniqueKey) {

    m_sharedMemory.setKey(m_uniqueKey + "_memory");

    if (m_sharedMemory.attach()) {
        m_isPrimary = false;
        m_shouldExit = true;
    } else {
        if (m_sharedMemory.create(1)) {
            m_isPrimary = true;
            m_shouldExit = false;
        }
    }
}

SingleInstanceApplication::~SingleInstanceApplication() {
    if (m_isPrimary) {
        m_sharedMemory.detach();
    }
}

