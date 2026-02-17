#ifndef CATEGORYINFO_H
#define CATEGORYINFO_H

#include <QString>

class CategoryInfo
{
public:
    CategoryInfo()
        : m_id(-1), m_isPassive(false), m_isActive(false) {}

    CategoryInfo(int id, const QString &name, bool isPassive, bool isActive, const QString &defaultUnit = QString())
        : m_id(id), m_name(name), m_isPassive(isPassive), m_isActive(isActive), m_defaultUnit(defaultUnit) {}

    int id() const { return m_id; }
    QString name() const { return m_name; }
    bool isPassive() const { return m_isPassive; }
    bool isActive() const { return m_isActive; }
    QString defaultUnit() const { return m_defaultUnit; }

    void setId(int id) { m_id = id; }
    void setName(const QString &name) { m_name = name; }
    void setIsPassive(bool passive) { m_isPassive = passive; }
    void setIsActive(bool active) { m_isActive = active; }
    void setDefaultUnit(const QString &unit) { m_defaultUnit = unit; }

    bool isValid() const { return m_id >= 0 && !m_name.isEmpty(); }

private:
    int m_id;
    QString m_name;
    bool m_isPassive;
    bool m_isActive;
    QString m_defaultUnit;
};

#endif // CATEGORYINFO_H
