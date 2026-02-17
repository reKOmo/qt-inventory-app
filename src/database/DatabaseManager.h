#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include <QObject>
#include <QSqlDatabase>
#include <QSqlError>
#include <memory>
#include <vector>
#include "models/Component.h"
#include "models/PassiveComponent.h"
#include "models/ActiveComponent.h"
#include "models/CategoryInfo.h"

class DatabaseManager : public QObject
{
    Q_OBJECT

public:
    static DatabaseManager &instance();

    ~DatabaseManager();
    DatabaseManager(const DatabaseManager &) = delete;
    DatabaseManager &operator=(const DatabaseManager &) = delete;

    bool initialize(const QString &dbPath = "inventory.db");
    bool isConnected() const;
    QSqlError lastError() const;

    // ==================== Category Operations ====================
    std::vector<CategoryInfo> fetchAllCategories();
    CategoryInfo fetchCategory(int id);
    CategoryInfo fetchCategoryByName(const QString &name);
    int addCategory(const CategoryInfo &category);
    bool updateCategory(const CategoryInfo &category);
    bool deleteCategory(int id);

    /**
     * @brief Check if a category can be deleted (not a system category)
     */
    bool canDeleteCategory(int id);

    int getComponentCountForCategory(const QString &categoryName);
    int getComponentCountForCategory(int categoryId);

    // ==================== Component Operations ====================
    int addComponent(const Component *component);
    bool updateComponent(const Component *component);
    bool deleteComponent(int id);
    std::unique_ptr<Component> fetchComponent(int id);
    std::vector<std::unique_ptr<Component>> fetchAllComponents();

    // Filter Operations
    std::vector<std::unique_ptr<Component>> fetchByCategory(const QString &categoryName);
    std::vector<std::unique_ptr<Component>> fetchLowStock(int threshold = 10);
    std::vector<std::unique_ptr<Component>> searchByName(const QString &searchTerm);

    bool populateSampleData();

signals:
    void dataChanged();
    void categoriesChanged();

    void errorOccurred(const QString &message);

private:
    DatabaseManager();

    /// Create database tables if they don't exist
    bool createTables();

    bool createCategoriesTable();
    bool populateDefaultCategories();

    std::unique_ptr<Component> hydrateComponent(const class QSqlQuery &query);

    QSqlDatabase m_database;
    QString m_connectionName;
    QSqlError m_lastError;
};

#endif // DATABASEMANAGER_H
