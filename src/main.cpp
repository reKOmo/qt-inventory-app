#include <QApplication>
#include <QDebug>
#include <QDir>
#include <QStyleFactory>
#include <QLoggingCategory>
#include <iostream>

#include "config/AppConfig.h"
#include "config/LanguageManager.h"
#include "ui/MainWindow.h"

void msgHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    Q_UNUSED(context);

    const char *typeStr;
    switch (type)
    {
    case QtDebugMsg:
        typeStr = "DEBUG";
        break;
    case QtInfoMsg:
        typeStr = "INFO";
        break;
    case QtWarningMsg:
        typeStr = "WARNING";
        break;
    case QtCriticalMsg:
        typeStr = "CRITICAL";
        break;
    case QtFatalMsg:
        typeStr = "FATAL";
        break;
    default:
        typeStr = "UNKNOWN";
        break;
    }

    fprintf(stderr, "[%s] %s\n", typeStr, msg.toLocal8Bit().constData());
    fflush(stderr);

    if (type == QtFatalMsg)
        abort();
}

int main(int argc, char *argv[])
{
    // Enable debug logging for our application
    QLoggingCategory::setFilterRules(
        "default.debug=true\n"
        "default.info=true\n"
        "default.warning=true\n"
        "default.critical=true\n"
        "qt.*.debug=false\n"
        "qt.*.info=false\n");

    qputenv("QT_LOGGING_TO_CONSOLE", "1");
    qputenv("QT_FORCE_STDERR_LOGGING", "1");
    qputenv("QT_ASSUME_STDERR_HAS_CONSOLE", "1");

    qInstallMessageHandler(msgHandler);

    QApplication app(argc, argv);

    AppConfig &config = AppConfig::instance();
    config.load("config.json");

    QString langFile = config.languageFile();
    if (!langFile.isEmpty())
    {
        if (!Lang.loadLanguage(langFile))
        {
            qWarning() << "Failed to load language file:" << langFile;
        }
        else
        {
            qDebug() << "Loaded language file:" << langFile;
        }
    }

    qDebug() << "Working directory:" << QDir::currentPath();

    MainWindow mainWindow;
    mainWindow.show();

    return app.exec();
}
