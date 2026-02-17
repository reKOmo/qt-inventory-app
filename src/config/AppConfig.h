#ifndef APPCONFIG_H
#define APPCONFIG_H

#include <QObject>
#include <QString>
#include <QVariant>
#include <QJsonObject>
#include <QMap>

class AppConfig : public QObject
{
    Q_OBJECT

public:
    static AppConfig &instance();

    AppConfig(const AppConfig &) = delete;
    AppConfig &operator=(const AppConfig &) = delete;

    bool load(const QString &filePath = "config.json");
    bool save(const QString &filePath = QString());

    // Application settings
    QString appName() const;
    QString appVersion() const;
    QString organizationName() const;

    // Database settings
    QString databasePath() const;
    void setDatabasePath(const QString &path);

    // UI settings
    int lowStockThreshold() const;
    void setLowStockThreshold(int threshold);

    bool showLowStockWarnings() const;
    void setShowLowStockWarnings(bool show);

    int defaultWindowWidth() const;
    void setDefaultWindowWidth(int width);

    int defaultWindowHeight() const;
    void setDefaultWindowHeight(int height);

    int sidebarWidth() const;
    void setSidebarWidth(int width);

    // Feature flags
    bool enableSampleData() const;
    void setEnableSampleData(bool enable);

    // Language settings
    QString languageFile() const;
    void setLanguageFile(const QString &filePath);

    bool configExists(const QString &filePath = "config.json") const;

    QString configFilePath() const { return m_configFilePath; }

signals:
    void configChanged();
    void configLoaded();
    void configSaved();

private:
    AppConfig();
    ~AppConfig() override;

    void setDefaults();
    QJsonObject toJson() const;
    void fromJson(const QJsonObject &json);

    QString m_configFilePath;
    QMap<QString, QVariant> m_settings;

    // Default values
    static constexpr const char *DEFAULT_APP_NAME = "ElectraBase Pro";
    static constexpr const char *DEFAULT_APP_VERSION = "1.0.0";
    static constexpr const char *DEFAULT_ORG_NAME = "ElectraBase";
    static constexpr const char *DEFAULT_DB_PATH = "inventory.db";
    static constexpr const char *DEFAULT_LANGUAGE_FILE = "lang/en.json";
    static constexpr int DEFAULT_LOW_STOCK_THRESHOLD = 10;
    static constexpr int DEFAULT_WINDOW_WIDTH = 1200;
    static constexpr int DEFAULT_WINDOW_HEIGHT = 700;
    static constexpr int DEFAULT_SIDEBAR_WIDTH = 200;
};

#define Config AppConfig::instance()

#endif // APPCONFIG_H
