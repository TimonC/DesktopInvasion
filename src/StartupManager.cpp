#include "StartupManager.h"
#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QStandardPaths>
#include <QDebug>

#ifdef Q_OS_WIN
#include <windows.h>
#endif

#ifdef Q_OS_LINUX
#include <QProcess>
#include <QFileInfo>
#endif

#ifdef Q_OS_MAC
#include <QProcess>
#include <QFileInfo>
#endif

StartupManager* StartupManager::m_instance = nullptr;

StartupManager::StartupManager(QObject *parent)
    : QObject(parent)
    , m_settings("DesktopInvasion", "DesktopInvasion")
{
    m_instance = this;
}

StartupManager* StartupManager::instance()
{
    return m_instance;
}

bool StartupManager::isAutoStartEnabled() const
{
    return m_settings.value("autoStart", false).toBool();
}

void StartupManager::setAutoStartEnabled(bool enabled)
{
    if (enabled == isAutoStartEnabled()) {
        return;
    }

    if (enabled) {
        enableAutoStart();
    } else {
        disableAutoStart();
    }

    m_settings.setValue("autoStart", enabled);
    emit autoStartEnabledChanged(enabled);
    qDebug() << "Auto-start" << (enabled ? "enabled" : "disabled");
}

void StartupManager::toggleAutoStart()
{
    setAutoStartEnabled(!isAutoStartEnabled());
}

#ifdef Q_OS_WIN
void StartupManager::enableAutoStart()
{
    QString appPath = QCoreApplication::applicationFilePath();
    appPath = QDir::toNativeSeparators(appPath);

    HKEY hKey;
    LONG result = RegOpenKeyExW(HKEY_CURRENT_USER,
                                 L"Software\\Microsoft\\Windows\\CurrentVersion\\Run",
                                 0, KEY_SET_VALUE, &hKey);

    if (result == ERROR_SUCCESS) {
        std::wstring wAppPath = appPath.toStdWString();
        RegSetValueExW(hKey, L"DesktopInvasion", 0, REG_SZ,
                       (BYTE*)wAppPath.c_str(),
                       (wAppPath.size() + 1) * sizeof(wchar_t));
        RegCloseKey(hKey);
        qDebug() << "Added to Windows Registry Run key";
    } else {
        qWarning() << "Failed to open Registry Run key";
    }
}

void StartupManager::disableAutoStart()
{
    HKEY hKey;
    LONG result = RegOpenKeyExW(HKEY_CURRENT_USER,
                                 L"Software\\Microsoft\\Windows\\CurrentVersion\\Run",
                                 0, KEY_SET_VALUE, &hKey);

    if (result == ERROR_SUCCESS) {
        RegDeleteValueW(hKey, L"DesktopInvasion");
        RegCloseKey(hKey);
        qDebug() << "Removed from Windows Registry Run key";
    }
}
#endif

#ifdef Q_OS_MAC
void StartupManager::enableAutoStart()
{
    QString bundlePath = QCoreApplication::applicationDirPath();

    for (int i = 0; i < 3; ++i) {
        bundlePath = QFileInfo(bundlePath).path();
        if (bundlePath.endsWith(".app")) {
            break;
        }
    }

    if (!bundlePath.endsWith(".app")) {
        qWarning() << "Could not find .app bundle";
        return;
    }

    QString plistDir = QStandardPaths::writableLocation(QStandardPaths::HomeLocation)
                       + "/Library/LaunchAgents";
    QDir().mkpath(plistDir);

    QString plistPath = plistDir + "/com.desktopinvasion.plist";

    QFile plistFile(plistPath);
    if (plistFile.open(QIODevice::WriteOnly)) {
        QString plistContent =
            "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
            "<!DOCTYPE plist PUBLIC \"-//Apple//DTD PLIST 1.0//EN\" \"http://www.apple.com/DTDs/PropertyList-1.0.dtd\">\n"
            "<plist version=\"1.0\">\n"
            "<dict>\n"
            "    <key>Label</key>\n"
            "    <string>com.desktopinvasion</string>\n"
            "    <key>ProgramArguments</key>\n"
            "    <array>\n"
            "        <string>" + bundlePath + "/Contents/MacOS/DesktopInvasion</string>\n"
            "    </array>\n"
            "    <key>RunAtLoad</key>\n"
            "    <true/>\n"
            "    <key>KeepAlive</key>\n"
            "    <false/>\n"
            "    <key>ProcessType</key>\n"
            "    <string>Background</string>\n"
            "</dict>\n"
            "</plist>\n";

        plistFile.write(plistContent.toUtf8());
        plistFile.close();

        QProcess::execute("launchctl", {"load", plistPath});
        qDebug() << "Added to macOS LaunchAgents";
    } else {
        qWarning() << "Failed to create plist file:" << plistPath;
    }
}

void StartupManager::disableAutoStart()
{
    QString plistPath = QStandardPaths::writableLocation(QStandardPaths::HomeLocation)
                        + "/Library/LaunchAgents/com.desktopinvasion.plist";

    if (QFile::exists(plistPath)) {
        QProcess::execute("launchctl", {"unload", plistPath});
        QFile::remove(plistPath);
        qDebug() << "Removed from macOS LaunchAgents";
    }
}
#endif

#ifdef Q_OS_LINUX
void StartupManager::enableAutoStart()
{
    QString autostartDir = QStandardPaths::writableLocation(QStandardPaths::HomeLocation)
                           + "/.config/autostart";
    QDir().mkpath(autostartDir);

    QString desktopFilePath = autostartDir + "/desktopinvasion.desktop";

    QString appPath;
    QByteArray appImagePath = qgetenv("APPIMAGE");
    if (!appImagePath.isEmpty()) {
        appPath = QString::fromLocal8Bit(appImagePath);
    } else {
        appPath = QCoreApplication::applicationFilePath();
    }

    QString iconPath;
    if (!appImagePath.isEmpty()) {
        QString appImageDir = QFileInfo(appPath).path();
        QStringList iconLocations = {
            appImageDir + "/.DirIcon",
            appImageDir + "/usr/share/icons/hicolor/256x256/apps/desktop-invasion.png",
            appImageDir + "/assets/icon/icon.png"
        };

        for (const QString& path : iconLocations) {
            if (QFile::exists(path)) {
                iconPath = path;
                break;
            }
        }
    }

    if (iconPath.isEmpty()) {
        QStringList iconLocations = {
            QCoreApplication::applicationDirPath() + "/../share/icons/hicolor/256x256/apps/desktop-invasion.png",
            QCoreApplication::applicationDirPath() + "/assets/icon/icon.png",
            ":/assets/icon/icon.png"
        };

        for (const QString& path : iconLocations) {
            if (QFile::exists(path)) {
                iconPath = path;
                break;
            }
        }
    }

    QFile desktopFile(desktopFilePath);
    if (desktopFile.open(QIODevice::WriteOnly)) {
        QString desktopContent =
            "[Desktop Entry]\n"
            "Type=Application\n"
            "Name=Desktop Invasion\n"
            "Exec=" + appPath + "\n"
            "Icon=" + iconPath + "\n"
            "Comment=You can't escape the invasion!\n"
            "X-GNOME-Autostart-enabled=true\n"
            "Terminal=false\n"
            "Categories=Game;\n"
            "StartupNotify=false\n"
            "X-GNOME-Autostart-Delay=3\n";

        desktopFile.write(desktopContent.toUtf8());
        desktopFile.close();

        QFile::setPermissions(desktopFilePath,
                              QFile::ReadOwner | QFile::WriteOwner | QFile::ExeOwner |
                              QFile::ReadGroup | QFile::ExeGroup |
                              QFile::ReadOther | QFile::ExeOther);

        qDebug() << "Added to Linux autostart:" << desktopFilePath;
        qDebug() << "Exec path:" << appPath;
    } else {
        qWarning() << "Failed to create desktop file:" << desktopFilePath;
    }
}

void StartupManager::disableAutoStart()
{
    QString desktopFilePath = QStandardPaths::writableLocation(QStandardPaths::HomeLocation)
                              + "/.config/autostart/desktopinvasion.desktop";

    if (QFile::exists(desktopFilePath)) {
        QFile::remove(desktopFilePath);
        qDebug() << "Removed from Linux autostart";
    }
}
#endif

#ifndef Q_OS_WIN
#ifndef Q_OS_MAC
#ifndef Q_OS_LINUX
void StartupManager::enableAutoStart()
{
    qWarning() << "Auto-start not supported on this platform";
}

void StartupManager::disableAutoStart()
{
    qWarning() << "Auto-start not supported on this platform";
}
#endif
#endif
#endif
