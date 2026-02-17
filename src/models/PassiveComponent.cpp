#include "models/PassiveComponent.h"
#include <cmath>

PassiveComponent::PassiveComponent(int id, const QString &name, const QString &manufacturer,
                                   int quantity, const QString &category,
                                   double value, const QString &unit, const QString &package)
    : Component(id, name, manufacturer, quantity, category), m_value(value), m_unit(unit), m_package(package)
{
}

PassiveComponent::~PassiveComponent() = default;

QString PassiveComponent::getDetails() const
{
    return QString("%1 %2, Package: %3, Qty: %4")
        .arg(getFormattedValue())
        .arg(m_unit)
        .arg(m_package)
        .arg(m_quantity);
}

QString PassiveComponent::getType() const
{
    return m_category;
}

std::unique_ptr<Component> PassiveComponent::clone() const
{
    return std::make_unique<PassiveComponent>(
        m_id, m_name, m_manufacturer, m_quantity, m_category,
        m_value, m_unit, m_package);
}

QString PassiveComponent::getFormattedValue() const
{
    // Format value with SI prefixes
    if (m_value >= 1e9)
    {
        return QString::number(m_value / 1e9, 'g', 3) + "G";
    }
    else if (m_value >= 1e6)
    {
        return QString::number(m_value / 1e6, 'g', 3) + "M";
    }
    else if (m_value >= 1e3)
    {
        return QString::number(m_value / 1e3, 'g', 3) + "k";
    }
    else if (m_value >= 1)
    {
        return QString::number(m_value, 'g', 3);
    }
    else if (m_value >= 1e-3)
    {
        return QString::number(m_value * 1e3, 'g', 3) + "m";
    }
    else if (m_value >= 1e-6)
    {
        return QString::number(m_value * 1e6, 'g', 3) + "μ";
    }
    else if (m_value >= 1e-9)
    {
        return QString::number(m_value * 1e9, 'g', 3) + "n";
    }
    else if (m_value >= 1e-12)
    {
        return QString::number(m_value * 1e12, 'g', 3) + "p";
    }
    return QString::number(m_value, 'g', 3);
}
