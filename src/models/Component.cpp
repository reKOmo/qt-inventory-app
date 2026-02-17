#include "models/Component.h"
#include "config/AppConfig.h"

Component::Component(int id, const QString &name, const QString &manufacturer,
                     int quantity, const QString &category)
    : m_id(id), m_name(name), m_manufacturer(manufacturer), m_quantity(quantity), m_category(category)
{
}

Component::~Component() = default;

bool Component::isLowStock() const
{
    return m_quantity < AppConfig::instance().lowStockThreshold();
}
