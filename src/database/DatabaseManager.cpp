#include "database/DatabaseManager.h"
#include <QSqlQuery>
#include <QSqlRecord>
#include <QVariant>
#include <QDebug>
#include <QFileInfo>

DatabaseManager &DatabaseManager::instance()
{
    static DatabaseManager instance;
    return instance;
}

DatabaseManager::DatabaseManager()
    : QObject(nullptr), m_connectionName("ElectraBaseConnection")
{
}

DatabaseManager::~DatabaseManager()
{
    if (m_database.isOpen())
    {
        m_database.close();
    }
    QSqlDatabase::removeDatabase(m_connectionName);
}

bool DatabaseManager::initialize(const QString &dbPath)
{
    if (QSqlDatabase::contains(m_connectionName))
    {
        m_database = QSqlDatabase::database(m_connectionName);
        if (m_database.isOpen())
        {
            return true;
        }
    }

    // Create new connection
    m_database = QSqlDatabase::addDatabase("QSQLITE", m_connectionName);
    m_database.setDatabaseName(dbPath);

    if (!m_database.open())
    {
        m_lastError = m_database.lastError();
        emit errorOccurred(QString("Failed to open database: %1").arg(m_lastError.text()));
        return false;
    }

    qDebug() << "Database opened successfully:" << dbPath;

    // Create tables if they don't exist
    if (!createTables())
    {
        return false;
    }

    // Create categories table
    if (!createCategoriesTable())
    {
        return false;
    }

    return true;
}

bool DatabaseManager::isConnected() const
{
    return m_database.isOpen();
}

QSqlError DatabaseManager::lastError() const
{
    return m_lastError;
}

bool DatabaseManager::createTables()
{
    QSqlQuery query(m_database);

    // Create inventory table
    const QString createTableSQL = R"(
        CREATE TABLE IF NOT EXISTS inventory (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            name TEXT NOT NULL,
            manufacturer TEXT,
            type TEXT NOT NULL,
            quantity INTEGER DEFAULT 0,
            param_1 REAL,
            param_2 TEXT,
            extra_data TEXT
        )
    )";

    if (!query.exec(createTableSQL))
    {
        m_lastError = query.lastError();
        emit errorOccurred(QString("Failed to create table: %1").arg(m_lastError.text()));
        return false;
    }

    // Create index for faster searches
    query.exec("CREATE INDEX IF NOT EXISTS idx_name ON inventory(name)");
    query.exec("CREATE INDEX IF NOT EXISTS idx_type ON inventory(type)");
    query.exec("CREATE INDEX IF NOT EXISTS idx_quantity ON inventory(quantity)");

    qDebug() << "Database tables created/verified successfully";
    return true;
}

bool DatabaseManager::createCategoriesTable()
{
    QSqlQuery query(m_database);

    const QString createCategoriesSQL = R"(
        CREATE TABLE IF NOT EXISTS categories (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            name TEXT NOT NULL UNIQUE,
            is_passive INTEGER DEFAULT 0,
            is_active INTEGER DEFAULT 0,
            default_unit TEXT,
            is_system INTEGER DEFAULT 0
        )
    )";

    if (!query.exec(createCategoriesSQL))
    {
        m_lastError = query.lastError();
        emit errorOccurred(QString("Failed to create categories table: %1").arg(m_lastError.text()));
        return false;
    }

    query.exec("SELECT COUNT(*) FROM categories");
    if (query.next() && query.value(0).toInt() == 0)
    {
        return populateDefaultCategories();
    }

    return true;
}

bool DatabaseManager::populateDefaultCategories()
{
    qDebug() << "Populating default categories...";

    struct DefaultCategory
    {
        QString name;
        bool isPassive;
        bool isActive;
        QString defaultUnit;
        bool isSystem;
    };

    std::vector<DefaultCategory> defaults = {
        {"Resistor", true, false, "Ω", true},
        {"Capacitor", true, false, "F", true},
        {"Inductor", true, false, "H", true},
        {"IC", false, true, "", true},
        {"Transistor", false, true, "", true},
        {"Diode", false, true, "", true},
        {"Connector", false, false, "", true},
        {"Other", false, false, "", true}};

    QSqlQuery query(m_database);
    query.prepare(R"(
        INSERT INTO categories (name, is_passive, is_active, default_unit, is_system)
        VALUES (:name, :is_passive, :is_active, :default_unit, :is_system)
    )");

    for (const auto &cat : defaults)
    {
        query.bindValue(":name", cat.name);
        query.bindValue(":is_passive", cat.isPassive ? 1 : 0);
        query.bindValue(":is_active", cat.isActive ? 1 : 0);
        query.bindValue(":default_unit", cat.defaultUnit);
        query.bindValue(":is_system", cat.isSystem ? 1 : 0);

        if (!query.exec())
        {
            m_lastError = query.lastError();
            qWarning() << "Failed to insert category:" << cat.name << m_lastError.text();
        }
    }

    qDebug() << "Default categories populated";
    return true;
}

// ==================== Categories ====================

std::vector<CategoryInfo> DatabaseManager::fetchAllCategories()
{
    std::vector<CategoryInfo> categories;

    QSqlQuery query(m_database);
    if (!query.exec("SELECT * FROM categories ORDER BY id"))
    {
        m_lastError = query.lastError();
        return categories;
    }

    while (query.next())
    {
        CategoryInfo cat(
            query.value("id").toInt(),
            query.value("name").toString(),
            query.value("is_passive").toBool(),
            query.value("is_active").toBool(),
            query.value("default_unit").toString());
        categories.push_back(cat);
    }

    return categories;
}

CategoryInfo DatabaseManager::fetchCategory(int id)
{
    QSqlQuery query(m_database);
    query.prepare("SELECT * FROM categories WHERE id = :id");
    query.bindValue(":id", id);

    if (!query.exec() || !query.next())
    {
        return CategoryInfo();
    }

    return CategoryInfo(
        query.value("id").toInt(),
        query.value("name").toString(),
        query.value("is_passive").toBool(),
        query.value("is_active").toBool(),
        query.value("default_unit").toString());
}

CategoryInfo DatabaseManager::fetchCategoryByName(const QString &name)
{
    QSqlQuery query(m_database);
    query.prepare("SELECT * FROM categories WHERE name = :name");
    query.bindValue(":name", name);

    if (!query.exec() || !query.next())
    {
        return CategoryInfo();
    }

    return CategoryInfo(
        query.value("id").toInt(),
        query.value("name").toString(),
        query.value("is_passive").toBool(),
        query.value("is_active").toBool(),
        query.value("default_unit").toString());
}

int DatabaseManager::addCategory(const CategoryInfo &category)
{
    QSqlQuery query(m_database);
    query.prepare(R"(
        INSERT INTO categories (name, is_passive, is_active, default_unit, is_system)
        VALUES (:name, :is_passive, :is_active, :default_unit, 0)
    )");

    query.bindValue(":name", category.name());
    query.bindValue(":is_passive", category.isPassive() ? 1 : 0);
    query.bindValue(":is_active", category.isActive() ? 1 : 0);
    query.bindValue(":default_unit", category.defaultUnit());

    if (!query.exec())
    {
        m_lastError = query.lastError();
        emit errorOccurred(QString("Failed to add category: %1").arg(m_lastError.text()));
        return -1;
    }

    int newId = query.lastInsertId().toInt();
    emit categoriesChanged();
    return newId;
}

bool DatabaseManager::updateCategory(const CategoryInfo &category)
{
    if (category.id() < 0)
        return false;

    QSqlQuery query(m_database);
    query.prepare(R"(
        UPDATE categories
        SET name = :name, is_passive = :is_passive,
            is_active = :is_active, default_unit = :default_unit
        WHERE id = :id
    )");

    query.bindValue(":id", category.id());
    query.bindValue(":name", category.name());
    query.bindValue(":is_passive", category.isPassive() ? 1 : 0);
    query.bindValue(":is_active", category.isActive() ? 1 : 0);
    query.bindValue(":default_unit", category.defaultUnit());

    if (!query.exec())
    {
        m_lastError = query.lastError();
        emit errorOccurred(QString("Failed to update category: %1").arg(m_lastError.text()));
        return false;
    }

    emit categoriesChanged();
    return true;
}

bool DatabaseManager::deleteCategory(int id)
{
    if (!canDeleteCategory(id))
    {
        emit errorOccurred("Cannot delete system categories");
        return false;
    }

    // Get the category name before deleting
    CategoryInfo cat = fetchCategory(id);
    if (!cat.isValid())
        return false;

    // Move components with this category to "Other"
    QSqlQuery updateQuery(m_database);
    updateQuery.prepare("UPDATE inventory SET type = 'Other' WHERE type = :type");
    updateQuery.bindValue(":type", cat.name());

    if (!updateQuery.exec())
    {
        m_lastError = updateQuery.lastError();
        emit errorOccurred(QString("Failed to reassign components: %1").arg(m_lastError.text()));
        return false;
    }

    int movedCount = updateQuery.numRowsAffected();
    if (movedCount > 0)
    {
        qDebug() << "Moved" << movedCount << "components from" << cat.name() << "to Other";
    }

    // Delete the category
    QSqlQuery deleteQuery(m_database);
    deleteQuery.prepare("DELETE FROM categories WHERE id = :id");
    deleteQuery.bindValue(":id", id);

    if (!deleteQuery.exec())
    {
        m_lastError = deleteQuery.lastError();
        emit errorOccurred(QString("Failed to delete category: %1").arg(m_lastError.text()));
        return false;
    }

    emit categoriesChanged();
    if (movedCount > 0)
    {
        emit dataChanged();
    }
    return true;
}

bool DatabaseManager::canDeleteCategory(int id)
{
    QSqlQuery query(m_database);
    query.prepare("SELECT is_system FROM categories WHERE id = :id");
    query.bindValue(":id", id);

    if (!query.exec() || !query.next())
    {
        return false;
    }

    return query.value(0).toInt() == 0;
}

int DatabaseManager::getComponentCountForCategory(const QString &categoryName)
{
    QSqlQuery query(m_database);
    query.prepare("SELECT COUNT(*) FROM inventory WHERE type = :type");
    query.bindValue(":type", categoryName);

    if (!query.exec() || !query.next())
    {
        return 0;
    }

    return query.value(0).toInt();
}

int DatabaseManager::getComponentCountForCategory(int categoryId)
{
    // First get the category name
    CategoryInfo cat = fetchCategory(categoryId);
    if (!cat.isValid())
    {
        return 0;
    }
    return getComponentCountForCategory(cat.name());
}

// ==================== Components ====================

int DatabaseManager::addComponent(const Component *component)
{
    if (!component)
        return -1;

    QSqlQuery query(m_database);
    query.prepare(R"(
        INSERT INTO inventory (name, manufacturer, type, quantity, param_1, param_2, extra_data)
        VALUES (:name, :manufacturer, :type, :quantity, :param_1, :param_2, :extra_data)
    )");

    query.bindValue(":name", component->getName());
    query.bindValue(":manufacturer", component->getManufacturer());
    query.bindValue(":type", component->getType());
    query.bindValue(":quantity", component->getQuantity());
    query.bindValue(":param_1", component->getParam1());
    query.bindValue(":param_2", component->getParam2());

    // Store additional data for passive/active components
    QString extraData;
    if (auto *passive = dynamic_cast<const PassiveComponent *>(component))
    {
        extraData = passive->getUnit();
    }
    else if (auto *active = dynamic_cast<const ActiveComponent *>(component))
    {
        extraData = active->getDatasheetLink();
    }
    query.bindValue(":extra_data", extraData);

    if (!query.exec())
    {
        m_lastError = query.lastError();
        emit errorOccurred(QString("Failed to add component: %1").arg(m_lastError.text()));
        return -1;
    }

    int newId = query.lastInsertId().toInt();
    emit dataChanged();
    return newId;
}

bool DatabaseManager::updateComponent(const Component *component)
{
    if (!component || component->getId() < 0)
        return false;

    QSqlQuery query(m_database);
    query.prepare(R"(
        UPDATE inventory 
        SET name = :name, manufacturer = :manufacturer, type = :type,
            quantity = :quantity, param_1 = :param_1, param_2 = :param_2,
            extra_data = :extra_data
        WHERE id = :id
    )");

    query.bindValue(":id", component->getId());
    query.bindValue(":name", component->getName());
    query.bindValue(":manufacturer", component->getManufacturer());
    query.bindValue(":type", component->getType());
    query.bindValue(":quantity", component->getQuantity());
    query.bindValue(":param_1", component->getParam1());
    query.bindValue(":param_2", component->getParam2());

    QString extraData;
    if (auto *passive = dynamic_cast<const PassiveComponent *>(component))
    {
        extraData = passive->getUnit();
    }
    else if (auto *active = dynamic_cast<const ActiveComponent *>(component))
    {
        extraData = active->getDatasheetLink();
    }
    query.bindValue(":extra_data", extraData);

    if (!query.exec())
    {
        m_lastError = query.lastError();
        emit errorOccurred(QString("Failed to update component: %1").arg(m_lastError.text()));
        return false;
    }

    emit dataChanged();
    return true;
}

bool DatabaseManager::deleteComponent(int id)
{
    QSqlQuery query(m_database);
    query.prepare("DELETE FROM inventory WHERE id = :id");
    query.bindValue(":id", id);

    if (!query.exec())
    {
        m_lastError = query.lastError();
        emit errorOccurred(QString("Failed to delete component: %1").arg(m_lastError.text()));
        return false;
    }

    emit dataChanged();
    return true;
}

std::unique_ptr<Component> DatabaseManager::hydrateComponent(const QSqlQuery &query)
{
    int id = query.value("id").toInt();
    QString name = query.value("name").toString();
    QString manufacturer = query.value("manufacturer").toString();
    QString type = query.value("type").toString();
    int quantity = query.value("quantity").toInt();
    double param1 = query.value("param_1").toDouble();
    QString param2 = query.value("param_2").toString();
    QString extraData = query.value("extra_data").toString();

    // Look up category info from database
    CategoryInfo catInfo = fetchCategoryByName(type);

    // Instantiate correct derived class based on category type
    if (catInfo.isPassive())
    {
        return std::make_unique<PassiveComponent>(
            id, name, manufacturer, quantity, type,
            param1,    // value
            extraData, // unit (stored in extra_data)
            param2     // package
        );
    }
    else if (catInfo.isActive())
    {
        return std::make_unique<ActiveComponent>(
            id, name, manufacturer, quantity, type,
            param1,         // operating voltage
            param2.toInt(), // pin count
            extraData       // datasheet link
        );
    }

    // For unknown types, create a passive component as fallback
    return std::make_unique<PassiveComponent>(
        id, name, manufacturer, quantity, type,
        param1, extraData, param2);
}

std::unique_ptr<Component> DatabaseManager::fetchComponent(int id)
{
    QSqlQuery query(m_database);
    query.prepare("SELECT * FROM inventory WHERE id = :id");
    query.bindValue(":id", id);

    if (!query.exec() || !query.next())
    {
        return nullptr;
    }

    return hydrateComponent(query);
}

std::vector<std::unique_ptr<Component>> DatabaseManager::fetchAllComponents()
{
    std::vector<std::unique_ptr<Component>> components;

    QSqlQuery query(m_database);
    if (!query.exec("SELECT * FROM inventory ORDER BY name"))
    {
        m_lastError = query.lastError();
        emit errorOccurred(QString("Failed to fetch components: %1").arg(m_lastError.text()));
        return components;
    }

    while (query.next())
    {
        if (auto component = hydrateComponent(query))
        {
            components.push_back(std::move(component));
        }
    }

    return components;
}

std::vector<std::unique_ptr<Component>> DatabaseManager::fetchByCategory(const QString &categoryName)
{
    std::vector<std::unique_ptr<Component>> components;

    QSqlQuery query(m_database);
    query.prepare("SELECT * FROM inventory WHERE type = :type ORDER BY name");
    query.bindValue(":type", categoryName);

    if (!query.exec())
    {
        m_lastError = query.lastError();
        return components;
    }

    while (query.next())
    {
        if (auto component = hydrateComponent(query))
        {
            components.push_back(std::move(component));
        }
    }

    return components;
}

std::vector<std::unique_ptr<Component>> DatabaseManager::fetchLowStock(int threshold)
{
    std::vector<std::unique_ptr<Component>> components;

    QSqlQuery query(m_database);
    query.prepare("SELECT * FROM inventory WHERE quantity < :threshold ORDER BY quantity ASC");
    query.bindValue(":threshold", threshold);

    if (!query.exec())
    {
        m_lastError = query.lastError();
        return components;
    }

    while (query.next())
    {
        if (auto component = hydrateComponent(query))
        {
            components.push_back(std::move(component));
        }
    }

    return components;
}

std::vector<std::unique_ptr<Component>> DatabaseManager::searchByName(const QString &searchTerm)
{
    std::vector<std::unique_ptr<Component>> components;

    QSqlQuery query(m_database);
    query.prepare("SELECT * FROM inventory WHERE name LIKE :term ORDER BY name");
    query.bindValue(":term", "%" + searchTerm + "%");

    if (!query.exec())
    {
        m_lastError = query.lastError();
        return components;
    }

    while (query.next())
    {
        if (auto component = hydrateComponent(query))
        {
            components.push_back(std::move(component));
        }
    }

    return components;
}

bool DatabaseManager::populateSampleData()
{
    // Check if data already exists
    QSqlQuery countQuery(m_database);
    if (countQuery.exec("SELECT COUNT(*) FROM inventory") && countQuery.next())
    {
        if (countQuery.value(0).toInt() > 0)
        {
            qDebug() << "Sample data already exists, skipping population";
            return true;
        }
    }

    qDebug() << "Populating sample data...";

    // Sample Resistors
    std::vector<std::unique_ptr<Component>> samples;

    samples.push_back(std::make_unique<PassiveComponent>(
        -1, "RES-10R-0805", "Yageo", 100, "Resistor", 10.0, "Ω", "0805"));
    samples.push_back(std::make_unique<PassiveComponent>(
        -1, "RES-100R-0805", "Yageo", 150, "Resistor", 100.0, "Ω", "0805"));
    samples.push_back(std::make_unique<PassiveComponent>(
        -1, "RES-1K-0603", "Vishay", 200, "Resistor", 1000.0, "Ω", "0603"));
    samples.push_back(std::make_unique<PassiveComponent>(
        -1, "RES-4K7-0805", "Panasonic", 75, "Resistor", 4700.0, "Ω", "0805"));
    samples.push_back(std::make_unique<PassiveComponent>(
        -1, "RES-10K-0805", "Yageo", 8, "Resistor", 10000.0, "Ω", "0805"));
    samples.push_back(std::make_unique<PassiveComponent>(
        -1, "RES-100K-1206", "Vishay", 50, "Resistor", 100000.0, "Ω", "1206"));

    // Sample Capacitors
    samples.push_back(std::make_unique<PassiveComponent>(
        -1, "CAP-100nF-0805", "Murata", 300, "Capacitor", 100e-9, "F", "0805"));
    samples.push_back(std::make_unique<PassiveComponent>(
        -1, "CAP-1uF-0805", "Samsung", 5, "Capacitor", 1e-6, "F", "0805"));
    samples.push_back(std::make_unique<PassiveComponent>(
        -1, "CAP-10uF-1206", "Murata", 120, "Capacitor", 10e-6, "F", "1206"));
    samples.push_back(std::make_unique<PassiveComponent>(
        -1, "CAP-100pF-0603", "TDK", 180, "Capacitor", 100e-12, "F", "0603"));

    // Sample Inductors
    samples.push_back(std::make_unique<PassiveComponent>(
        -1, "IND-10uH-1210", "Wurth", 45, "Inductor", 10e-6, "H", "1210"));
    samples.push_back(std::make_unique<PassiveComponent>(
        -1, "IND-100uH-THT", "Bourns", 3, "Inductor", 100e-6, "H", "Radial"));

    // Sample ICs
    samples.push_back(std::make_unique<ActiveComponent>(
        -1, "ATmega328P", "Microchip", 25, "IC", 5.0, 28, "https://ww1.microchip.com/downloads/en/DeviceDoc/ATmega328P.pdf"));
    samples.push_back(std::make_unique<ActiveComponent>(
        -1, "STM32F103C8T6", "STMicroelectronics", 15, "IC", 3.3, 48, "https://www.st.com/resource/en/datasheet/stm32f103c8.pdf"));
    samples.push_back(std::make_unique<ActiveComponent>(
        -1, "NE555", "Texas Instruments", 50, "IC", 15.0, 8, "https://www.ti.com/lit/ds/symlink/ne555.pdf"));
    samples.push_back(std::make_unique<ActiveComponent>(
        -1, "LM7805", "ON Semiconductor", 7, "IC", 35.0, 3, ""));
    samples.push_back(std::make_unique<ActiveComponent>(
        -1, "ESP32-WROOM-32", "Espressif", 12, "IC", 3.3, 38, "https://www.espressif.com/sites/default/files/documentation/esp32-wroom-32_datasheet_en.pdf"));

    // Sample Transistors
    samples.push_back(std::make_unique<ActiveComponent>(
        -1, "2N2222A", "ON Semiconductor", 200, "Transistor", 40.0, 3, ""));
    samples.push_back(std::make_unique<ActiveComponent>(
        -1, "BC547B", "Fairchild", 150, "Transistor", 45.0, 3, ""));
    samples.push_back(std::make_unique<ActiveComponent>(
        -1, "IRF540N", "Infineon", 6, "Transistor", 100.0, 3, ""));

    // Sample Diodes
    samples.push_back(std::make_unique<ActiveComponent>(
        -1, "1N4148", "Vishay", 500, "Diode", 100.0, 2, ""));
    samples.push_back(std::make_unique<ActiveComponent>(
        -1, "1N4007", "ON Semiconductor", 300, "Diode", 1000.0, 2, ""));
    samples.push_back(std::make_unique<ActiveComponent>(
        -1, "LED-RED-5mm", "Kingbright", 9, "Diode", 2.0, 2, ""));

    // Add all samples to database
    for (const auto &component : samples)
    {
        if (addComponent(component.get()) < 0)
        {
            return false;
        }
    }

    qDebug() << "Sample data populated successfully";
    return true;
}
