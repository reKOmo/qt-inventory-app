#include "models/ActiveComponent.h"

ActiveComponent::ActiveComponent(int id, const QString &name, const QString &manufacturer,
                                 int quantity, const QString &category,
                                 double operatingVoltage, int pinCount, const QString &datasheetLink)
    : Component(id, name, manufacturer, quantity, category), m_operatingVoltage(operatingVoltage), m_pinCount(pinCount), m_datasheetLink(datasheetLink)
{
}

ActiveComponent::~ActiveComponent() = default;

QString ActiveComponent::getDetails() const
{
    QString details = QString("%1V, %2 pins")
                          .arg(m_operatingVoltage, 0, 'f', 1)
                          .arg(m_pinCount);

    if (!m_datasheetLink.isEmpty())
    {
        details += ", Datasheet available";
    }

    return details;
}

QString ActiveComponent::getType() const
{
    return m_category;
}

std::unique_ptr<Component> ActiveComponent::clone() const
{
    return std::make_unique<ActiveComponent>(
        m_id, m_name, m_manufacturer, m_quantity, m_category,
        m_operatingVoltage, m_pinCount, m_datasheetLink);
}
