#ifndef SINGLEINSTANCEAPPLICATION_H
#define SINGLEINSTANCEAPPLICATION_H

#include <QApplication>
#include <QLocalServer>

class SingleInstanceApplication : public QApplication
{
    Q_OBJECT
public:
    SingleInstanceApplication(int &argc, char **argv, const QString &uniqueKey);
    ~SingleInstanceApplication();

    bool shouldExit() const { return m_shouldExit; }

private slots:
    void handleNewConnection();

private:
    QString m_uniqueKey;
    QLocalServer *m_server;
    bool m_shouldExit;
};

#endif
