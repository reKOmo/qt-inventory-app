#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTableView>
#include <QLineEdit>
#include <QComboBox>
#include <QPushButton>
#include <QLabel>
#include <QSortFilterProxyModel>
#include <QSplitter>
#include <QListWidget>
#include <memory>
#include <vector>
#include "models/CategoryInfo.h"

class ComponentTableModel;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private slots:
    void refreshData();

    void onSearchTextChanged(const QString &text);
    void onCategoryFilterChanged(int index);

    void onAddComponent();
    void onEditComponent();
    void onDeleteComponent();

    void onSelectionChanged();
    void onTableDoubleClicked(const QModelIndex &index);

    void onShowLowStock();
    void onShowAll();

    void onSidebarCategorySelected(QListWidgetItem *item);
    void updateStatusBar();

    void onDatabaseError(const QString &message);

    void onManageCategories();
    void onAddCategory();
    void onCategoriesChanged();

private:
    void setupUi();
    void setupMenuBar();
    void setupToolBar();

    QWidget *createSidebar();
    QWidget *createMainContent();

    void setupConnections();
    void initializeDatabase();

    int getSelectedComponentId() const;

    void loadCategories();
    void updateSidebarCategories();
    void updateCategoryFilter();

    // UI Components
    QSplitter *m_splitter;
    QListWidget *m_sidebarList;
    QTableView *m_tableView;
    QLineEdit *m_searchEdit;
    QComboBox *m_categoryFilter;
    QPushButton *m_addButton;
    QPushButton *m_editButton;
    QPushButton *m_deleteButton;
    QPushButton *m_refreshButton;
    QLabel *m_statusLabel;

    // Model
    ComponentTableModel *m_model;
    QSortFilterProxyModel *m_proxyModel;

    // Categories from database
    std::vector<CategoryInfo> m_categories;

    // Current filter state
    QString m_currentCategoryFilter;
    bool m_showingLowStockOnly;
};

#endif // MAINWINDOW_H
