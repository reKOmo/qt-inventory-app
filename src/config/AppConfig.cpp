#include "config/AppConfig.h"

#include <QDebug>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>

AppConfig &AppConfig::instance()
{
    static AppConfig instance;
    return instance;
}

AppConfig::AppConfig()
    : QObject(nullptr)
{
    setDefaults();
}

AppConfig::~AppConfig() = default;

void AppConfig::setDefaults()
{
    m_settings["app/name"] = DEFAULT_APP_NAME;
    m_settings["app/version"] = DEFAULT_APP_VERSION;
    m_settings["app/organization"] = DEFAULT_ORG_NAME;

    m_settings["database/path"] = DEFAULT_DB_PATH;

    m_settings["ui/lowStockThreshold"] = DEFAULT_LOW_STOCK_THRESHOLD;
    m_settings["ui/showLowStockWarnings"] = true;
    m_settings["ui/windowWidth"] = DEFAULT_WINDOW_WIDTH;
    m_settings["ui/windowHeight"] = DEFAULT_WINDOW_HEIGHT;
    m_settings["ui/sidebarWidth"] = DEFAULT_SIDEBAR_WIDTH;

    m_settings["features/enableSampleData"] = true;

    m_settings["language/file"] = DEFAULT_LANGUAGE_FILE;
}

bool AppConfig::configExists(const QString &filePath) const
{
    return QFileInfo::exists(filePath);
}

bool AppConfig::load(const QString &filePath)
{
    QString path = filePath.isEmpty() ? "config.json" : filePath;

    QFile file(path);
    if (!file.exists())
    {
        qWarning() << "Config file not found:" << path << "- using defaults";
        return false;
    }

    if (!file.open(QIODevice::ReadOnly))
    {
        qWarning() << "Failed to open config file:" << path;
        return false;
    }

    QByteArray data = file.readAll();
    file.close();

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);

    if (parseError.error != QJsonParseError::NoError)
    {
        qWarning() << "JSON parse error:" << parseError.errorString();
        return false;
    }

    if (!doc.isObject())
    {
        qWarning() << "Config file is not a JSON object";
        return false;
    }

    fromJson(doc.object());
    m_configFilePath = path;

    qDebug() << "Configuration loaded from:" << path;
    emit configLoaded();
    return true;
}

bool AppConfig::save(const QString &filePath)
{
    QString path = filePath.isEmpty() ? m_configFilePath : filePath;
    if (path.isEmpty())
    {
        path = "config.json";
    }

    QFile file(path);
    if (!file.open(QIODevice::WriteOnly))
    {
        qWarning() << "Failed to open config file for writing:" << path;
        return false;
    }

    QJsonDocument doc(toJson());
    file.write(doc.toJson(QJsonDocument::Indented));
    file.close();

    m_configFilePath = path;
    qDebug() << "Configuration saved to:" << path;
    emit configSaved();
    return true;
}

QJsonObject AppConfig::toJson() const
{
    QJsonObject root;

    // App
    QJsonObject app;
    app["name"] = m_settings["app/name"].toString();
    app["version"] = m_settings["app/version"].toString();
    app["organization"] = m_settings["app/organization"].toString();
    root["app"] = app;

    // Database
    QJsonObject database;
    database["path"] = m_settings["database/path"].toString();
    root["database"] = database;

    // UI
    QJsonObject ui;
    ui["lowStockThreshold"] = m_settings["ui/lowStockThreshold"].toInt();
    ui["showLowStockWarnings"] = m_settings["ui/showLowStockWarnings"].toBool();
    ui["windowWidth"] = m_settings["ui/windowWidth"].toInt();
    ui["windowHeight"] = m_settings["ui/windowHeight"].toInt();
    ui["sidebarWidth"] = m_settings["ui/sidebarWidth"].toInt();
    root["ui"] = ui;

    // Features
    QJsonObject features;
    features["enableSampleData"] = m_settings["features/enableSampleData"].toBool();
    root["features"] = features;

    // Language
    QJsonObject language;
    language["file"] = m_settings["language/file"].toString();
    root["language"] = language;

    return root;
}

void AppConfig::fromJson(const QJsonObject &json)
{
    // App
    if (json.contains("app") && json["app"].isObject())
    {
        QJsonObject app = json["app"].toObject();
        if (app.contains("name"))
            m_settings["app/name"] = app["name"].toString();
        if (app.contains("version"))
            m_settings["app/version"] = app["version"].toString();
        if (app.contains("organization"))
            m_settings["app/organization"] = app["organization"].toString();
    }

    // Database
    if (json.contains("database") && json["database"].isObject())
    {
        QJsonObject database = json["database"].toObject();
        if (database.contains("path"))
            m_settings["database/path"] = database["path"].toString();
    }

    // UI
    if (json.contains("ui") && json["ui"].isObject())
    {
        QJsonObject ui = json["ui"].toObject();
        if (ui.contains("lowStockThreshold"))
            m_settings["ui/lowStockThreshold"] = ui["lowStockThreshold"].toInt();
        if (ui.contains("showLowStockWarnings"))
            m_settings["ui/showLowStockWarnings"] = ui["showLowStockWarnings"].toBool();
        if (ui.contains("windowWidth"))
            m_settings["ui/windowWidth"] = ui["windowWidth"].toInt();
        if (ui.contains("windowHeight"))
            m_settings["ui/windowHeight"] = ui["windowHeight"].toInt();
        if (ui.contains("sidebarWidth"))
            m_settings["ui/sidebarWidth"] = ui["sidebarWidth"].toInt();
    }

    // Features
    if (json.contains("features") && json["features"].isObject())
    {
        QJsonObject features = json["features"].toObject();
        if (features.contains("enableSampleData"))
            m_settings["features/enableSampleData"] = features["enableSampleData"].toBool();
    }

    // Language
    if (json.contains("language") && json["language"].isObject())
    {
        QJsonObject language = json["language"].toObject();
        if (language.contains("file"))
            m_settings["language/file"] = language["file"].toString();
    }

    emit configChanged();
}

QString AppConfig::appName() const
{
    return m_settings["app/name"].toString();
}

QString AppConfig::appVersion() const
{
    return m_settings["app/version"].toString();
}

QString AppConfig::organizationName() const
{
    return m_settings["app/organization"].toString();
}

QString AppConfig::databasePath() const
{
    return m_settings["database/path"].toString();
}

void AppConfig::setDatabasePath(const QString &path)
{
    m_settings["database/path"] = path;
    emit configChanged();
}

int AppConfig::lowStockThreshold() const
{
    return m_settings["ui/lowStockThreshold"].toInt();
}

void AppConfig::setLowStockThreshold(int threshold)
{
    m_settings["ui/lowStockThreshold"] = threshold;
    emit configChanged();
}

bool AppConfig::showLowStockWarnings() const
{
    return m_settings["ui/showLowStockWarnings"].toBool();
}

void AppConfig::setShowLowStockWarnings(bool show)
{
    m_settings["ui/showLowStockWarnings"] = show;
    emit configChanged();
}

int AppConfig::defaultWindowWidth() const
{
    return m_settings["ui/windowWidth"].toInt();
}

void AppConfig::setDefaultWindowWidth(int width)
{
    m_settings["ui/windowWidth"] = width;
    emit configChanged();
}

int AppConfig::defaultWindowHeight() const
{
    return m_settings["ui/windowHeight"].toInt();
}

void AppConfig::setDefaultWindowHeight(int height)
{
    m_settings["ui/windowHeight"] = height;
    emit configChanged();
}

int AppConfig::sidebarWidth() const
{
    return m_settings["ui/sidebarWidth"].toInt();
}

void AppConfig::setSidebarWidth(int width)
{
    m_settings["ui/sidebarWidth"] = width;
    emit configChanged();
}

bool AppConfig::enableSampleData() const
{
    return m_settings["features/enableSampleData"].toBool();
}

void AppConfig::setEnableSampleData(bool enable)
{
    m_settings["features/enableSampleData"] = enable;
    emit configChanged();
}

QString AppConfig::languageFile() const
{
    return m_settings["language/file"].toString();
}

void AppConfig::setLanguageFile(const QString &filePath)
{
    m_settings["language/file"] = filePath;
    emit configChanged();
}
