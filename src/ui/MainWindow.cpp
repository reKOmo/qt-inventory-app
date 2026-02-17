#include "ui/MainWindow.h"
#include "config/AppConfig.h"
#include "config/LanguageManager.h"
#include "ui/CategoryDialog.h"
#include "ui/ComponentTableModel.h"
#include "ui/ComponentDialog.h"

#include "database/DatabaseManager.h"
#include "models/PassiveComponent.h"
#include "models/ActiveComponent.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMenuBar>
#include <QMenu>
#include <QToolBar>
#include <QStatusBar>
#include <QHeaderView>
#include <QMessageBox>
#include <QApplication>
#include <QGroupBox>
#include <QIcon>
#include <QInputDialog>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), m_splitter(nullptr), m_sidebarList(nullptr), m_tableView(nullptr), m_searchEdit(nullptr), m_categoryFilter(nullptr), m_addButton(nullptr), m_editButton(nullptr), m_deleteButton(nullptr), m_refreshButton(nullptr), m_statusLabel(nullptr), m_model(nullptr), m_proxyModel(nullptr), m_showingLowStockOnly(false)
{
    setWindowTitle(Config.appName());
    setMinimumSize(Config.defaultWindowWidth(), Config.defaultWindowHeight());

    initializeDatabase();
    loadCategories();
    setupUi();
    setupMenuBar();
    setupToolBar();
    setupConnections();

    refreshData();
    updateStatusBar();
}

MainWindow::~MainWindow() = default;

void MainWindow::loadCategories()
{
    m_categories = DatabaseManager::instance().fetchAllCategories();
}

void MainWindow::setupUi()
{
    // Create main splitter
    m_splitter = new QSplitter(Qt::Horizontal, this);
    m_splitter->addWidget(createSidebar());
    m_splitter->addWidget(createMainContent());
    m_splitter->setSizes({Config.sidebarWidth(), 1000});
    m_splitter->setStretchFactor(0, 0);
    m_splitter->setStretchFactor(1, 1);

    setCentralWidget(m_splitter);

    // Status bar
    m_statusLabel = new QLabel(this);
    statusBar()->addWidget(m_statusLabel, 1);
}

void MainWindow::setupMenuBar()
{
    // File menu
    qDebug() << Lang.translate("menu.file.title");
    QMenu *fileMenu = menuBar()->addMenu(Lang.translate("menu.file.title"));

    QAction *refreshAction = fileMenu->addAction(Lang.translate("menu.file.refresh"));
    refreshAction->setShortcut(QKeySequence::Refresh);
    connect(refreshAction, &QAction::triggered, this, &MainWindow::refreshData);

    fileMenu->addSeparator();

    QAction *exitAction = fileMenu->addAction(Lang.translate("menu.file.exit"));
    exitAction->setShortcut(QKeySequence::Quit);
    connect(exitAction, &QAction::triggered, this, &QWidget::close);

    // Component menu
    QMenu *componentMenu = menuBar()->addMenu(Lang.translate("menu.component.title"));

    QAction *addAction = componentMenu->addAction(Lang.translate("menu.component.addNew"));
    addAction->setShortcut(QKeySequence::New);
    connect(addAction, &QAction::triggered, this, &MainWindow::onAddComponent);

    QAction *editAction = componentMenu->addAction(Lang.translate("menu.component.edit"));
    editAction->setShortcut(Qt::Key_F2);
    connect(editAction, &QAction::triggered, this, &MainWindow::onEditComponent);

    QAction *deleteAction = componentMenu->addAction(Lang.translate("menu.component.delete"));
    deleteAction->setShortcut(QKeySequence::Delete);
    connect(deleteAction, &QAction::triggered, this, &MainWindow::onDeleteComponent);

    // Category menu
    QMenu *categoryMenu = menuBar()->addMenu(Lang.translate("menu.category.title"));

    QAction *addCategoryAction = categoryMenu->addAction(Lang.translate("menu.category.addNew"));
    connect(addCategoryAction, &QAction::triggered, this, &MainWindow::onAddCategory);

    QAction *manageCategoriesAction = categoryMenu->addAction(Lang.translate("menu.category.manage"));
    connect(manageCategoriesAction, &QAction::triggered, this, &MainWindow::onManageCategories);

    // View menu
    QMenu *viewMenu = menuBar()->addMenu(Lang.translate("menu.view.title"));

    QAction *showAllAction = viewMenu->addAction(Lang.translate("menu.view.showAll"));
    connect(showAllAction, &QAction::triggered, this, &MainWindow::onShowAll);

    QAction *showLowStockAction = viewMenu->addAction(Lang.translate("menu.view.showLowStock"));
    connect(showLowStockAction, &QAction::triggered, this, &MainWindow::onShowLowStock);

    // Help menu
    QMenu *helpMenu = menuBar()->addMenu(Lang.translate("menu.help.title"));

    QAction *aboutQtAction = helpMenu->addAction(Lang.translate("menu.help.aboutQt"));
    connect(aboutQtAction, &QAction::triggered, qApp, &QApplication::aboutQt);
}

void MainWindow::setupToolBar()
{
    QToolBar *toolBar = addToolBar(Lang.translate("toolbar.title"));
    toolBar->setMovable(false);

    m_addButton = new QPushButton(Lang.translate("toolbar.add"), this);
    m_addButton->setToolTip(Lang.translate("toolbar.addTooltip"));
    toolBar->addWidget(m_addButton);

    m_editButton = new QPushButton(Lang.translate("toolbar.edit"), this);
    m_editButton->setToolTip(Lang.translate("toolbar.editTooltip"));
    m_editButton->setEnabled(false);
    toolBar->addWidget(m_editButton);

    m_deleteButton = new QPushButton(Lang.translate("toolbar.delete"), this);
    m_deleteButton->setToolTip(Lang.translate("toolbar.deleteTooltip"));
    m_deleteButton->setEnabled(false);
    toolBar->addWidget(m_deleteButton);

    toolBar->addSeparator();

    m_refreshButton = new QPushButton(Lang.translate("toolbar.refresh"), this);
    m_refreshButton->setToolTip(Lang.translate("toolbar.refreshTooltip"));
    toolBar->addWidget(m_refreshButton);

    toolBar->addSeparator();

    // Search box
    QLabel *searchLabel = new QLabel(" " + Lang.translate("toolbar.search") + " ", this);
    toolBar->addWidget(searchLabel);

    m_searchEdit = new QLineEdit(this);
    m_searchEdit->setPlaceholderText(Lang.translate("toolbar.searchPlaceholder"));
    m_searchEdit->setMinimumWidth(200);
    m_searchEdit->setClearButtonEnabled(true);
    toolBar->addWidget(m_searchEdit);

    toolBar->addSeparator();

    // Category filter
    QLabel *filterLabel = new QLabel(" " + Lang.translate("toolbar.category") + " ", this);
    toolBar->addWidget(filterLabel);

    m_categoryFilter = new QComboBox(this);
    updateCategoryFilter(); // Populate from database
    m_categoryFilter->setMinimumWidth(120);
    toolBar->addWidget(m_categoryFilter);
}

QWidget *MainWindow::createSidebar()
{
    QWidget *sidebar = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(sidebar);
    layout->setContentsMargins(5, 5, 5, 5);

    // Title
    QLabel *title = new QLabel(QString("<b>%1</b>").arg(Lang.translate("sidebar.title")), sidebar);
    layout->addWidget(title);

    // Category list
    m_sidebarList = new QListWidget(sidebar);
    m_sidebarList->addItem(Lang.translate("sidebar.allComponents"));
    m_sidebarList->addItem(Lang.translate("sidebar.lowStockItems"));
    m_sidebarList->addItem(QString::fromUtf8("───────────────"));
    m_sidebarList->item(2)->setFlags(Qt::NoItemFlags); // Separator not selectable

    // Add categories from database
    updateSidebarCategories();

    m_sidebarList->setCurrentRow(0);
    layout->addWidget(m_sidebarList);

    // Stats group
    QGroupBox *statsGroup = new QGroupBox(Lang.translate("sidebar.quickStats"), sidebar);
    QVBoxLayout *statsLayout = new QVBoxLayout(statsGroup);

    QLabel *statsPlaceholder = new QLabel(Lang.translate("sidebar.loading"), statsGroup);
    statsPlaceholder->setObjectName("statsLabel");
    statsLayout->addWidget(statsPlaceholder);

    layout->addWidget(statsGroup);

    return sidebar;
}

QWidget *MainWindow::createMainContent()
{
    QWidget *content = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(content);
    layout->setContentsMargins(5, 5, 5, 5);

    // Table view
    m_tableView = new QTableView(content);

    // Create model and proxy
    m_model = new ComponentTableModel(this);
    m_proxyModel = new QSortFilterProxyModel(this);
    m_proxyModel->setSourceModel(m_model);
    m_proxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
    m_proxyModel->setFilterKeyColumn(-1); // Search all columns

    m_tableView->setModel(m_proxyModel);
    m_tableView->setSortingEnabled(true);
    m_tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_tableView->setSelectionMode(QAbstractItemView::SingleSelection);
    m_tableView->setAlternatingRowColors(true);
    m_tableView->horizontalHeader()->setStretchLastSection(true);
    m_tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
    m_tableView->verticalHeader()->setVisible(false);
    m_tableView->setShowGrid(true);

    // Set initial column widths
    m_tableView->setColumnWidth(ComponentTableModel::ColId, 50);
    m_tableView->setColumnWidth(ComponentTableModel::ColName, 180);
    m_tableView->setColumnWidth(ComponentTableModel::ColCategory, 100);
    m_tableView->setColumnWidth(ComponentTableModel::ColManufacturer, 130);
    m_tableView->setColumnWidth(ComponentTableModel::ColQuantity, 60);
    m_tableView->setColumnWidth(ComponentTableModel::ColValue, 100);
    m_tableView->setColumnWidth(ComponentTableModel::ColPackage, 100);

    layout->addWidget(m_tableView);

    return content;
}

void MainWindow::setupConnections()
{
    // Button connections
    connect(m_addButton, &QPushButton::clicked, this, &MainWindow::onAddComponent);
    connect(m_editButton, &QPushButton::clicked, this, &MainWindow::onEditComponent);
    connect(m_deleteButton, &QPushButton::clicked, this, &MainWindow::onDeleteComponent);
    connect(m_refreshButton, &QPushButton::clicked, this, &MainWindow::refreshData);

    // Search and filter
    connect(m_searchEdit, &QLineEdit::textChanged, this, &MainWindow::onSearchTextChanged);
    connect(m_categoryFilter, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::onCategoryFilterChanged);

    // Table selection
    connect(m_tableView->selectionModel(), &QItemSelectionModel::selectionChanged,
            this, &MainWindow::onSelectionChanged);
    connect(m_tableView, &QTableView::doubleClicked, this, &MainWindow::onTableDoubleClicked);

    // Sidebar
    connect(m_sidebarList, &QListWidget::itemClicked, this, &MainWindow::onSidebarCategorySelected);

    // Database manager
    connect(&DatabaseManager::instance(), &DatabaseManager::dataChanged,
            this, &MainWindow::updateStatusBar);
    connect(&DatabaseManager::instance(), &DatabaseManager::errorOccurred,
            this, &MainWindow::onDatabaseError);
    connect(&DatabaseManager::instance(), &DatabaseManager::categoriesChanged,
            this, &MainWindow::onCategoriesChanged);
}

void MainWindow::initializeDatabase()
{
    AppConfig &config = AppConfig::instance();
    auto &db = DatabaseManager::instance();

    if (!db.initialize(config.databasePath()))
    {
        QMessageBox::critical(this, Lang.translate("messages.databaseError"),
                              Lang.translate("messages.databaseInitFailed", db.lastError().text(), "Failed to initialize database:\n%1"));
        return;
    }

    // Populate sample data on first run (if enabled in config)
    if (config.enableSampleData())
    {
        db.populateSampleData();
    }
}

void MainWindow::refreshData()
{
    m_model->refresh();
    updateStatusBar();

    // Update sidebar stats
    QLabel *statsLabel = m_splitter->findChild<QLabel *>("statsLabel");
    if (statsLabel)
    {
        int total = m_model->componentCount();
        auto lowStock = DatabaseManager::instance().fetchLowStock();
        statsLabel->setText(Lang.translate("sidebar.totalLabel", QString::number(total), "Total: %1") + "\n" +
                            Lang.translate("sidebar.lowStockLabel", QString::number(lowStock.size()), "Low Stock: %1"));
    }
}

void MainWindow::onSearchTextChanged(const QString &text)
{
    m_proxyModel->setFilterRegularExpression(
        QRegularExpression(text, QRegularExpression::CaseInsensitiveOption));
    updateStatusBar();
}

void MainWindow::onCategoryFilterChanged(int index)
{
    QString category = m_categoryFilter->itemData(index).toString();

    if (category.isEmpty())
    {
        m_proxyModel->setFilterKeyColumn(-1);
        m_proxyModel->setFilterRegularExpression(m_searchEdit->text());
    }
    else
    {
        // Filter by category column
        m_proxyModel->setFilterKeyColumn(ComponentTableModel::ColCategory);
        m_proxyModel->setFilterFixedString(category);
    }

    m_currentCategoryFilter = category;
    updateStatusBar();
}

void MainWindow::onAddComponent()
{
    ComponentDialog dialog(this);
    dialog.setWindowTitle(Lang.translate("dialog.component.titleAdd"));

    if (dialog.exec() == QDialog::Accepted)
    {
        auto component = dialog.getComponent();
        if (component)
        {
            int id = DatabaseManager::instance().addComponent(component.get());
            if (id >= 0)
            {
                component->setId(id);
                m_model->addComponent(std::move(component));
                updateStatusBar();
            }
        }
    }
}

void MainWindow::onEditComponent()
{
    int id = getSelectedComponentId();
    if (id < 0)
    {
        QMessageBox::information(this, Lang.translate("messages.noSelection"),
                                 Lang.translate("messages.selectComponentToEdit"));
        return;
    }

    const Component *component = m_model->getComponentById(id);
    if (!component)
        return;

    ComponentDialog dialog(this);
    dialog.setWindowTitle(Lang.translate("dialog.component.titleEdit"));
    dialog.setComponent(component);

    if (dialog.exec() == QDialog::Accepted)
    {
        auto updatedComponent = dialog.getComponent();
        if (updatedComponent)
        {
            updatedComponent->setId(id);
            if (DatabaseManager::instance().updateComponent(updatedComponent.get()))
            {
                m_model->updateComponent(updatedComponent.get());
                updateStatusBar();
            }
        }
    }
}

void MainWindow::onDeleteComponent()
{
    int id = getSelectedComponentId();
    if (id < 0)
    {
        QMessageBox::information(this, Lang.translate("messages.noSelection"),
                                 Lang.translate("messages.selectComponentToDelete"));
        return;
    }

    const Component *component = m_model->getComponentById(id);
    if (!component)
        return;

    QMessageBox::StandardButton reply = QMessageBox::question(this,
                                                              Lang.translate("dialog.category.confirmDelete"),
                                                              Lang.translate("messages.confirmDeleteComponent", component->getName(), "Are you sure you want to delete '%1'?"),
                                                              QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes)
    {
        if (DatabaseManager::instance().deleteComponent(id))
        {
            m_model->removeComponent(id);
            updateStatusBar();
        }
    }
}

void MainWindow::onSelectionChanged()
{
    bool hasSelection = m_tableView->selectionModel()->hasSelection();
    m_editButton->setEnabled(hasSelection);
    m_deleteButton->setEnabled(hasSelection);
}

void MainWindow::onTableDoubleClicked(const QModelIndex &index)
{
    if (index.isValid())
    {
        onEditComponent();
    }
}

void MainWindow::onShowLowStock()
{
    m_showingLowStockOnly = true;
    m_model->setComponents(DatabaseManager::instance().fetchLowStock());
    m_sidebarList->setCurrentRow(1); // Low Stock Items
    updateStatusBar();
}

void MainWindow::onShowAll()
{
    m_showingLowStockOnly = false;
    m_categoryFilter->setCurrentIndex(0);
    m_searchEdit->clear();
    refreshData();
    m_sidebarList->setCurrentRow(0); // All Components
}

void MainWindow::onSidebarCategorySelected(QListWidgetItem *item)
{
    if (!item || !(item->flags() & Qt::ItemIsSelectable))
        return;

    QString text = item->text();

    if (text.contains("All Components"))
    {
        onShowAll();
    }
    else if (text.contains("Low Stock"))
    {
        onShowLowStock();
    }
    else
    {
        // Find category by matching name in the item text
        for (const auto &cat : m_categories)
        {
            if (text.contains(cat.name()))
            {
                m_showingLowStockOnly = false;
                int index = m_categoryFilter->findData(cat.name());
                if (index >= 0)
                {
                    m_categoryFilter->setCurrentIndex(index);
                }
                break;
            }
        }
    }
}

void MainWindow::updateStatusBar()
{
    int total = m_model->componentCount();
    int visible = m_proxyModel->rowCount();

    QString status;
    if (m_showingLowStockOnly)
    {
        status = Lang.translate("statusBar.showingLowStock", QString::number(visible), "Showing %1 low stock items");
    }
    else if (visible != total)
    {
        status = Lang.translate("statusBar.showingFiltered", QStringList{QString::number(visible), QString::number(total)}, "Showing %1 of %2 components");
    }
    else
    {
        status = Lang.translate("statusBar.totalComponents", QString::number(total), "Total: %1 components");
    }

    m_statusLabel->setText(status);
}

void MainWindow::onDatabaseError(const QString &message)
{
    QMessageBox::warning(this, Lang.translate("messages.databaseError"), message);
}

int MainWindow::getSelectedComponentId() const
{
    QModelIndexList selection = m_tableView->selectionModel()->selectedRows();
    if (selection.isEmpty())
    {
        return -1;
    }

    // Map from proxy to source model
    QModelIndex sourceIndex = m_proxyModel->mapToSource(selection.first());
    const Component *component = m_model->getComponentAt(sourceIndex.row());

    return component ? component->getId() : -1;
}

void MainWindow::updateSidebarCategories()
{
    // Remove existing category items (keep first 3: All, Low Stock, Separator)
    while (m_sidebarList->count() > 3)
    {
        delete m_sidebarList->takeItem(3);
    }

    // Add categories from m_categories
    for (const auto &cat : m_categories)
    {
        m_sidebarList->addItem(cat.name());
    }
}

void MainWindow::updateCategoryFilter()
{
    // Remember current selection
    QString currentCategory = m_categoryFilter->currentData().toString();

    m_categoryFilter->clear();
    m_categoryFilter->addItem(Lang.translate("toolbar.allCategories"), "");

    for (const auto &cat : m_categories)
    {
        m_categoryFilter->addItem(cat.name(), cat.name());
    }

    // Restore selection if possible
    int index = m_categoryFilter->findData(currentCategory);
    if (index >= 0)
    {
        m_categoryFilter->setCurrentIndex(index);
    }
}

void MainWindow::onCategoriesChanged()
{
    loadCategories();
    updateSidebarCategories();
    updateCategoryFilter();
    refreshData();
}

void MainWindow::onAddCategory()
{
    CategoryDialog dialog(this);
    dialog.setWindowTitle(Lang.translate("dialog.category.titleAdd"));

    if (dialog.exec() == QDialog::Accepted)
    {
        CategoryInfo newCat = dialog.getCategory();
        int id = DatabaseManager::instance().addCategory(newCat);
        if (id > 0)
        {
            newCat.setId(id);
            // categoriesChanged signal will trigger refresh
        }
        else
        {
            QMessageBox::warning(this, Lang.translate("messages.error"), Lang.translate("messages.addCategoryFailed"));
        }
    }
}

void MainWindow::onManageCategories()
{
    // Show a dialog to manage (edit/delete) categories
    QDialog manageDialog(this);
    manageDialog.setWindowTitle(Lang.translate("dialog.category.titleManage"));
    manageDialog.setMinimumSize(400, 300);

    QVBoxLayout *layout = new QVBoxLayout(&manageDialog);

    QListWidget *catList = new QListWidget(&manageDialog);
    for (const auto &cat : m_categories)
    {
        QListWidgetItem *item = new QListWidgetItem(cat.name(), catList);
        item->setData(Qt::UserRole, cat.id());
    }
    layout->addWidget(catList);

    QHBoxLayout *buttonLayout = new QHBoxLayout();

    QPushButton *editBtn = new QPushButton(Lang.translate("dialog.category.edit"), &manageDialog);
    QPushButton *deleteBtn = new QPushButton(Lang.translate("dialog.category.delete"), &manageDialog);
    QPushButton *closeBtn = new QPushButton(Lang.translate("dialog.category.close"), &manageDialog);

    buttonLayout->addWidget(editBtn);
    buttonLayout->addWidget(deleteBtn);
    buttonLayout->addStretch();
    buttonLayout->addWidget(closeBtn);
    layout->addLayout(buttonLayout);

    connect(closeBtn, &QPushButton::clicked, &manageDialog, &QDialog::accept);

    connect(editBtn, &QPushButton::clicked, [this, catList, &manageDialog]()
            {
        QListWidgetItem *currentItem = catList->currentItem();
        if (!currentItem)
        {
            QMessageBox::information(&manageDialog, Lang.translate("dialog.category.noSelection"), 
                                     Lang.translate("dialog.category.selectToEdit"));
            return;
        }

        int catId = currentItem->data(Qt::UserRole).toInt();
        
        // Find the category
        CategoryInfo catToEdit;
        bool found = false;
        for (const auto& cat : m_categories)
        {
            if (cat.id() == catId)
            {
                catToEdit = cat;
                found = true;
                break;
            }
        }

        if (!found) return;

        CategoryDialog editDialog(this);
        editDialog.setWindowTitle(Lang.translate("dialog.category.titleEdit"));
        editDialog.setCategory(catToEdit);

        if (editDialog.exec() == QDialog::Accepted)
        {
            CategoryInfo updatedCat = editDialog.getCategory();
            updatedCat.setId(catId);
            
            if (DatabaseManager::instance().updateCategory(updatedCat))
            {
                // Update list item
                currentItem->setText(updatedCat.name());
            }
            else
            {
                QMessageBox::warning(&manageDialog, Lang.translate("messages.error"), Lang.translate("messages.updateCategoryFailed"));
            }
        } });

    connect(deleteBtn, &QPushButton::clicked, [this, catList, &manageDialog]()
            {
        QListWidgetItem *currentItem = catList->currentItem();
        if (!currentItem)
        {
            QMessageBox::information(&manageDialog, Lang.translate("dialog.category.noSelection"),
                                     Lang.translate("dialog.category.selectToDelete"));
            return;
        }

        int catId = currentItem->data(Qt::UserRole).toInt();
        QString catName = currentItem->text();

        // Check if this is the "Other" category
        for (const auto& cat : m_categories)
        {
            if (cat.id() == catId && cat.name() == "Other")
            {
                QMessageBox::warning(&manageDialog, Lang.translate("dialog.category.cannotDelete"),
                    Lang.translate("dialog.category.cannotDeleteOther"));
                return;
            }
        }

        // Check component count
        int componentCount = DatabaseManager::instance().getComponentCountForCategory(catId);
        
        QString message;
        if (componentCount > 0)
        {
            message = Lang.translate("dialog.category.deleteWithComponents", 
                             QStringList{catName, QString::number(componentCount)},
                             "Are you sure you want to delete category '%1'?\n\n%2 component(s) will be reassigned to 'Other'.");
        }
        else
        {
            message = Lang.translate("dialog.category.deleteEmpty", catName,
                             "Are you sure you want to delete category '%1'?");
        }

        QMessageBox::StandardButton reply = QMessageBox::question(
            &manageDialog, Lang.translate("dialog.category.confirmDelete"), message,
            QMessageBox::Yes | QMessageBox::No);

        if (reply == QMessageBox::Yes)
        {
            if (DatabaseManager::instance().deleteCategory(catId))
            {
                delete catList->takeItem(catList->row(currentItem));
            }
            else
            {
                QMessageBox::warning(&manageDialog, Lang.translate("messages.error"), Lang.translate("messages.deleteCategoryFailed"));
            }
        } });

    manageDialog.exec();
}
