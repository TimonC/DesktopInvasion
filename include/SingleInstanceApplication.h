#ifndef SINGLEINSTANCEAPPLICATION_HH
#define SINGLEINSTANCEAPPLICATION_HH

#include <QApplication>
#include <QSharedMemory>
#include <QLocalServer>

class SingleInstanceApplication : public QApplication {
    Q_OBJECT

public:
    SingleInstanceApplication(int &argc, char **argv, const QString &uniqueKey);
    ~SingleInstanceApplication();

    bool isPrimary() const { return m_isPrimary; }
    bool shouldExit() const { return m_shouldExit; }

signals:
    void signalReceived(const QString &signal);

private slots:
    void handleNewConnection();

private:
    void setupServer();
    void checkForExistingInstance();

    QString m_uniqueKey;
    QSharedMemory m_sharedMemory;
    QLocalServer m_server;
    bool m_isPrimary = false;
    bool m_shouldExit = false;
};

#endif
