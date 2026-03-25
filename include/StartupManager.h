#ifndef STARTUPMANAGER_H
#define STARTUPMANAGER_H

#include <QObject>
#include <QSettings>

class StartupManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool autoStartEnabled READ isAutoStartEnabled WRITE setAutoStartEnabled NOTIFY autoStartEnabledChanged)

public:
    explicit StartupManager(QObject *parent = nullptr);
    static StartupManager* instance();

    Q_INVOKABLE bool isAutoStartEnabled() const;
    Q_INVOKABLE void setAutoStartEnabled(bool enabled);
    Q_INVOKABLE void toggleAutoStart();

signals:
    void autoStartEnabledChanged(bool enabled);

private:
    void enableAutoStart();
    void disableAutoStart();

    QSettings m_settings;
    static StartupManager* m_instance;
};

#endif
