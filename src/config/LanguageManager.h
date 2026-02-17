#ifndef LANGUAGEMANAGER_H
#define LANGUAGEMANAGER_H

#include <QObject>
#include <QString>
#include <QJsonObject>
#include <QMap>

class LanguageManager : public QObject
{
    Q_OBJECT

public:
    static LanguageManager &instance();

    ~LanguageManager() override;

    LanguageManager(const LanguageManager &) = delete;
    LanguageManager &operator=(const LanguageManager &) = delete;

    bool loadLanguage(const QString &filePath);

    QString translate(const QString &key, const QString &defaultValue = QString()) const;
    QString translate(const QString &key, const QString &arg1, const QString &defaultValue) const;
    QString translate(const QString &key, const QStringList &args, const QString &defaultValue = QString()) const;

    QString currentLanguage() const { return m_currentLanguage; }

signals:
    // Emitted when language changes
    void languageChanged();

private:
    LanguageManager();

    QString getNestedValue(const QString &key) const;

    QJsonObject m_translations;
    QString m_currentLanguage;
};

#define Lang LanguageManager::instance()

#endif // LANGUAGEMANAGER_H
