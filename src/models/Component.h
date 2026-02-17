#ifndef COMPONENT_H
#define COMPONENT_H

#include <QString>
#include <memory>

/**
 * @brief Abstract Base Class for all electronic components
 *
 * This class defines the common interface for all component types
 * following the OOP hierarchy required for ECAD systems.
 */
class Component
{
public:
    /**
     * @param id Database ID (-1 for new components)
     * @param name Component name/part number
     * @param manufacturer Manufacturer name
     * @param quantity Stock quantity
     * @param category Component category name
     */
    Component(int id, const QString &name, const QString &manufacturer,
              int quantity, const QString &category);

    virtual ~Component();

    virtual QString getDetails() const = 0;
    virtual QString getType() const = 0;
    virtual double getParam1() const = 0;
    virtual QString getParam2() const = 0;

    virtual std::unique_ptr<Component> clone() const = 0;

    int getId() const { return m_id; }
    QString getName() const { return m_name; }
    QString getManufacturer() const { return m_manufacturer; }
    int getQuantity() const { return m_quantity; }
    QString getCategory() const { return m_category; }

    void setId(int id) { m_id = id; }
    void setName(const QString &name) { m_name = name; }
    void setManufacturer(const QString &manufacturer) { m_manufacturer = manufacturer; }
    void setQuantity(int quantity) { m_quantity = quantity; }
    void setCategory(const QString &category) { m_category = category; }

    bool isLowStock() const;

protected:
    int m_id;
    QString m_name;
    QString m_manufacturer;
    int m_quantity;
    QString m_category;
};

#endif // COMPONENT_H
