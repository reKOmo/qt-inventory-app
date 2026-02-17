#ifndef ACTIVECOMPONENT_H
#define ACTIVECOMPONENT_H

#include "models/Component.h"

class ActiveComponent : public Component
{
public:
    /**
     * @param id Database ID
     * @param name Component name/part number
     * @param manufacturer Manufacturer name
     * @param quantity Stock quantity
     * @param category Component category name
     * @param operatingVoltage Operating voltage in Volts
     * @param pinCount Number of pins
     * @param datasheetLink URL to datasheet
     */
    ActiveComponent(int id, const QString &name, const QString &manufacturer,
                    int quantity, const QString &category,
                    double operatingVoltage, int pinCount, const QString &datasheetLink);

    ~ActiveComponent() override;

    QString getDetails() const override;
    QString getType() const override;
    double getParam1() const override { return m_operatingVoltage; }
    QString getParam2() const override { return QString::number(m_pinCount); }
    std::unique_ptr<Component> clone() const override;

    double getOperatingVoltage() const { return m_operatingVoltage; }
    int getPinCount() const { return m_pinCount; }
    QString getDatasheetLink() const { return m_datasheetLink; }

    void setOperatingVoltage(double voltage) { m_operatingVoltage = voltage; }
    void setPinCount(int count) { m_pinCount = count; }
    void setDatasheetLink(const QString &link) { m_datasheetLink = link; }

private:
    double m_operatingVoltage;
    int m_pinCount;
    QString m_datasheetLink;
};

#endif // ACTIVECOMPONENT_H
