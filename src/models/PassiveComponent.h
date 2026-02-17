#ifndef PASSIVECOMPONENT_H
#define PASSIVECOMPONENT_H

#include "models/Component.h"

class PassiveComponent : public Component
{
public:
    /**
     * @param id Database ID
     * @param name Component name
     * @param manufacturer Manufacturer name
     * @param quantity Stock quantity
     * @param category Component category name
     * @param value Component value (e.g., 10000 for 10kΩ)
     * @param unit Unit string (Ω, F, H)
     * @param package Package type (e.g., 0805, 0603, SMD, THT)
     */
    PassiveComponent(int id, const QString &name, const QString &manufacturer,
                     int quantity, const QString &category,
                     double value, const QString &unit, const QString &package);

    ~PassiveComponent() override;

    QString getDetails() const override;
    QString getType() const override;
    double getParam1() const override { return m_value; }
    QString getParam2() const override { return m_package; }
    std::unique_ptr<Component> clone() const override;

    double getValue() const { return m_value; }
    QString getUnit() const { return m_unit; }
    QString getPackage() const { return m_package; }

    void setValue(double value) { m_value = value; }
    void setUnit(const QString &unit) { m_unit = unit; }
    void setPackage(const QString &package) { m_package = package; }

    QString getFormattedValue() const;

private:
    double m_value;
    QString m_unit;
    QString m_package;
};

#endif // PASSIVECOMPONENT_H
