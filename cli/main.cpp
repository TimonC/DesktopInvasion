#include <QLocalSocket>
#include <QCoreApplication>
#include <QProcess>
#include <QDebug>
#include <QThread>

int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);

    const int waitTime = 500;

    QString signal;
    bool showHelp = false;

    for (int i = 1; i < argc; ++i) {
        QString arg = QString::fromLocal8Bit(argv[i]);
        if (     arg == "--start" || arg == "-s") signal = "START";
        else if (arg == "--quit" || arg == "-q") signal = "QUIT";
        else if (arg == "--toggle" || arg == "-t") signal = "TOGGLE";
        else if (arg == "--on") signal = "ON";
        else if (arg == "--off") signal = "OFF";
        else if (arg == "--menu") signal = "MENU";
        else if (arg == "--settings") signal = "SETTINGS";
        else if (arg == "--help" || arg == "-h") {
            showHelp = true;
        }
    }

    if (showHelp || signal.isEmpty()) {
        qDebug() << "Usage: dinv [OPTION]";
        qDebug() << "CLI tool to control DesktopInvasion";
        qDebug() << "";
        qDebug() << "  --start, -s       Launch DesktopInvasion";
        qDebug() << "  --quit, -q        Quit DesktopInvasion";
        qDebug() << "  --toggle, -t      Toggle the invasion";
        qDebug() << "  --on              Enable the invasion";
        qDebug() << "  --off             Disable the invasion";
        qDebug() << "  --menu            Open menu";
        qDebug() << "  --settings        Open settings";
        qDebug() << "  --help, -h        Display this help";
        return 0;
    }

    if (signal == "QUIT") {
        QLocalSocket socket;
        socket.connectToServer("DesktopInvasion");
        if (socket.waitForConnected(waitTime)) {
            socket.write(signal.toUtf8());
            socket.waitForBytesWritten(waitTime);
            qDebug() << "Quit signal sent.";
            return 0;
        } else {
            qDebug() << "Game not running.";
            return 1;
        }
    }

    if (signal == "START") {
        QLocalSocket socket;
        socket.connectToServer("DesktopInvasion");
        if (socket.waitForConnected(waitTime)) {
            qDebug() << "Game is already running.";
            return 0;
        }

        qDebug() << "Starting game...";
        QString appPath;
#ifdef Q_OS_WIN
        appPath = QCoreApplication::applicationDirPath() + "/DesktopInvasion.exe";
#else
        appPath = QCoreApplication::applicationDirPath() + "/DesktopInvasion";
#endif

        if (!QProcess::startDetached(appPath, QStringList())) {
            qDebug() << "Failed to start game.";
            return 1;
        }

        qDebug() << "Game started.";
        return 0;
    }

    QLocalSocket socket;
    socket.connectToServer("DesktopInvasion");

    if (!socket.waitForConnected(waitTime)) {
        qDebug() << "Game is not running. Use 'dinv --start' first.";
        return 1;
    }

    socket.write(signal.toUtf8());
    socket.waitForBytesWritten(waitTime);

    if (socket.waitForReadyRead(waitTime)) {
        qDebug() << socket.readAll();
    } else {
        qDebug() << "Signal sent.";
    }

    socket.disconnectFromServer();

    return 0;
}
