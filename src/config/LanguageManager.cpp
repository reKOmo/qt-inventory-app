#include "config/LanguageManager.h"

#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QDebug>

LanguageManager &LanguageManager::instance()
{
    static LanguageManager instance;
    return instance;
}

LanguageManager::LanguageManager()
    : QObject(nullptr), m_currentLanguage("en")
{
}

LanguageManager::~LanguageManager() = default;

bool LanguageManager::loadLanguage(const QString &filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly))
    {
        qWarning() << "Could not open language file:" << filePath;
        return false;
    }

    QByteArray data = file.readAll();
    file.close();

    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(data, &error);

    if (error.error != QJsonParseError::NoError)
    {
        qWarning() << "JSON parse error in language file:" << error.errorString();
        return false;
    }

    if (!doc.isObject())
    {
        qWarning() << "Language file must contain a JSON object";
        return false;
    }

    m_translations = doc.object();

    // Get language code from file if present
    if (m_translations.contains("_meta"))
    {
        QJsonObject meta = m_translations["_meta"].toObject();
        m_currentLanguage = meta.value("language").toString("en");
    }

    emit languageChanged();
    return true;
}

QString LanguageManager::getNestedValue(const QString &key) const
{
    QStringList parts = key.split('.');
    QJsonObject current = m_translations;

    for (int i = 0; i < parts.size() - 1; ++i)
    {
        if (!current.contains(parts[i]))
        {
            return QString();
        }
        QJsonValue val = current[parts[i]];
        if (!val.isObject())
        {
            return QString();
        }
        current = val.toObject();
    }

    if (parts.isEmpty())
    {
        return QString();
    }

    QJsonValue finalValue = current[parts.last()];

    if (finalValue.isString())
    {
        return finalValue.toString();
    }
    else if (finalValue.isArray())
    {
        // Return array as comma-separated for simple cases
        QJsonArray arr = finalValue.toArray();
        QStringList items;
        for (const QJsonValue &v : arr)
        {
            items << v.toString();
        }
        return items.join(",");
    }

    return QString();
}

QString LanguageManager::translate(const QString &key, const QString &defaultValue) const
{
    QString result = getNestedValue(key);
    if (result.isEmpty() && !defaultValue.isEmpty())
    {
        return defaultValue;
    }
    return result.isEmpty() ? key : result;
}

QString LanguageManager::translate(const QString &key, const QString &arg1, const QString &defaultValue) const
{
    QString result = translate(key, defaultValue);
    return result.arg(arg1);
}

QString LanguageManager::translate(const QString &key, const QStringList &args, const QString &defaultValue) const
{
    QString result = translate(key, defaultValue);
    for (int i = 0; i < args.size(); ++i)
    {
        result = result.arg(args[i]);
    }
    return result;
}
