#include "ui/ComponentTableModel.h"
#include "models/PassiveComponent.h"
#include "models/ActiveComponent.h"
#include "database/DatabaseManager.h"
#include "config/LanguageManager.h"
#include <QColor>
#include <QFont>

ComponentTableModel::ComponentTableModel(QObject *parent)
    : QAbstractTableModel(parent)
{
}

ComponentTableModel::~ComponentTableModel() = default;

int ComponentTableModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
    {
        return 0; // Flat table, no children
    }
    return static_cast<int>(m_components.size());
}

int ComponentTableModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
    {
        return 0;
    }
    return ColumnCount;
}

QVariant ComponentTableModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= static_cast<int>(m_components.size()))
    {
        return QVariant();
    }

    const auto &component = m_components[index.row()];

    // Custom role for low stock indication
    if (role == LowStockRole)
    {
        return component->isLowStock();
    }

    if (role == Qt::DisplayRole)
    {
        switch (index.column())
        {
        case ColId:
            return component->getId();
        case ColName:
            return component->getName();
        case ColCategory:
            return component->getCategory();
        case ColManufacturer:
            return component->getManufacturer();
        case ColQuantity:
            return component->getQuantity();
        case ColValue:
            return getParam1Display(component.get());
        case ColPackage:
            return getParam2Display(component.get());
        case ColDetails:
            return component->getDetails();
        }
    }

    if (role == Qt::TextAlignmentRole)
    {
        switch (index.column())
        {
        case ColId:
        case ColQuantity:
            return static_cast<int>(Qt::AlignCenter);
        case ColValue:
            return static_cast<int>(Qt::AlignRight | Qt::AlignVCenter);
        default:
            return static_cast<int>(Qt::AlignLeft | Qt::AlignVCenter);
        }
    }

    if (role == Qt::BackgroundRole)
    {
        if (component->isLowStock())
        {
            return QColor(255, 230, 230); // Light red/pink background
        }
    }

    if (role == Qt::ForegroundRole)
    {
        if (component->isLowStock())
        {
            return QColor(180, 0, 0); // Dark red text
        }
    }

    if (role == Qt::FontRole)
    {
        if (component->isLowStock())
        {
            QFont font;
            font.setBold(true);
            return font;
        }
    }

    if (role == Qt::ToolTipRole)
    {
        if (component->isLowStock())
        {
            return QString("Low Stock Warning: Only %1 units remaining!")
                .arg(component->getQuantity());
        }
        return component->getDetails();
    }

    return QVariant();
}

QVariant ComponentTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole)
    {
        return QVariant();
    }

    if (orientation == Qt::Horizontal)
    {
        switch (section)
        {
        case ColId:
            return Lang.translate("table.columns.id");
        case ColName:
            return Lang.translate("table.columns.name");
        case ColCategory:
            return Lang.translate("table.columns.category");
        case ColManufacturer:
            return Lang.translate("table.columns.manufacturer");
        case ColQuantity:
            return Lang.translate("table.columns.quantity");
        case ColValue:
            return Lang.translate("table.columns.value");
        case ColPackage:
            return Lang.translate("table.columns.package");
        case ColDetails:
            return "Details";
        }
    }
    else
    {
        return section + 1; // Row numbers
    }

    return QVariant();
}

void ComponentTableModel::setComponents(std::vector<std::unique_ptr<Component>> &&components)
{
    beginResetModel();
    m_components = std::move(components);
    endResetModel();
}

void ComponentTableModel::addComponent(std::unique_ptr<Component> component)
{
    int row = static_cast<int>(m_components.size());
    beginInsertRows(QModelIndex(), row, row);
    m_components.push_back(std::move(component));
    endInsertRows();
}

bool ComponentTableModel::updateComponent(const Component *component)
{
    if (!component)
        return false;

    for (size_t i = 0; i < m_components.size(); ++i)
    {
        if (m_components[i]->getId() == component->getId())
        {
            // Clone the new data
            m_components[i] = component->clone();

            // Emit data changed for entire row
            QModelIndex topLeft = createIndex(static_cast<int>(i), 0);
            QModelIndex bottomRight = createIndex(static_cast<int>(i), ColumnCount - 1);
            emit dataChanged(topLeft, bottomRight);
            return true;
        }
    }
    return false;
}

bool ComponentTableModel::removeComponent(int id)
{
    for (size_t i = 0; i < m_components.size(); ++i)
    {
        if (m_components[i]->getId() == id)
        {
            beginRemoveRows(QModelIndex(), static_cast<int>(i), static_cast<int>(i));
            m_components.erase(m_components.begin() + i);
            endRemoveRows();
            return true;
        }
    }
    return false;
}

const Component *ComponentTableModel::getComponentAt(int row) const
{
    if (row >= 0 && row < static_cast<int>(m_components.size()))
    {
        return m_components[row].get();
    }
    return nullptr;
}

const Component *ComponentTableModel::getComponentById(int id) const
{
    for (const auto &component : m_components)
    {
        if (component->getId() == id)
        {
            return component.get();
        }
    }
    return nullptr;
}

void ComponentTableModel::refresh()
{
    beginResetModel();
    m_components = DatabaseManager::instance().fetchAllComponents();
    endResetModel();
}

int ComponentTableModel::getRowForId(int id) const
{
    for (size_t i = 0; i < m_components.size(); ++i)
    {
        if (m_components[i]->getId() == id)
        {
            return static_cast<int>(i);
        }
    }
    return -1;
}

void ComponentTableModel::clear()
{
    beginResetModel();
    m_components.clear();
    endResetModel();
}

QString ComponentTableModel::getParam1Display(const Component *component) const
{
    if (auto *passive = dynamic_cast<const PassiveComponent *>(component))
    {
        return passive->getFormattedValue() + passive->getUnit();
    }
    else if (auto *active = dynamic_cast<const ActiveComponent *>(component))
    {
        return QString("%1V").arg(active->getOperatingVoltage(), 0, 'f', 1);
    }
    return QString::number(component->getParam1());
}

QString ComponentTableModel::getParam2Display(const Component *component) const
{
    if (auto *passive = dynamic_cast<const PassiveComponent *>(component))
    {
        return passive->getPackage();
    }
    else if (auto *active = dynamic_cast<const ActiveComponent *>(component))
    {
        return QString("%1 pins").arg(active->getPinCount());
    }
    return component->getParam2();
}
