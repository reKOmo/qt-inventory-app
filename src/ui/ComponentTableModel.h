#ifndef COMPONENTTABLEMODEL_H
#define COMPONENTTABLEMODEL_H

#include <QAbstractTableModel>
#include <memory>
#include <vector>
#include "models/Component.h"

class ComponentTableModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    /// Column indices for the table
    enum Column
    {
        ColId = 0,
        ColName,
        ColCategory,
        ColManufacturer,
        ColQuantity,
        ColValue,
        ColPackage,
        ColDetails,
        ColumnCount
    };

    explicit ComponentTableModel(QObject *parent = nullptr);
    ~ComponentTableModel() override;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const override;

    void setComponents(std::vector<std::unique_ptr<Component>> &&components);
    void addComponent(std::unique_ptr<Component> component);
    bool updateComponent(const Component *component);
    bool removeComponent(int id);

    const Component *getComponentAt(int row) const;
    const Component *getComponentById(int id) const;
    int getRowForId(int id) const;

    void refresh();
    void clear();

    int componentCount() const { return static_cast<int>(m_components.size()); }

    /// Custom role for low stock indication
    static constexpr int LowStockRole = Qt::UserRole + 1;

private:
    std::vector<std::unique_ptr<Component>> m_components;

    QString getParam1Display(const Component *component) const;
    QString getParam2Display(const Component *component) const;
};

#endif // COMPONENTTABLEMODEL_H
