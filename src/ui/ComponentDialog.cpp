#include "ui/ComponentDialog.h"
#include "models/PassiveComponent.h"
#include "models/ActiveComponent.h"
#include "database/DatabaseManager.h"
#include "config/LanguageManager.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QLabel>
#include <QPushButton>
#include <QMessageBox>
#include <QDialogButtonBox>

ComponentDialog::ComponentDialog(QWidget *parent)
    : QDialog(parent), m_nameEdit(nullptr), m_manufacturerEdit(nullptr), m_categoryCombo(nullptr), m_quantitySpin(nullptr), m_stackedWidget(nullptr), m_valueSpin(nullptr), m_valueMultiplier(nullptr), m_packageEdit(nullptr), m_voltageSpin(nullptr), m_pinCountSpin(nullptr), m_datasheetEdit(nullptr), m_okButton(nullptr), m_cancelButton(nullptr)
{
    setWindowTitle(Lang.translate("menu.component.title"));
    setMinimumWidth(450);
    setModal(true);

    setupUi();
    setupConnections();

    // Default to first category
    onCategoryChanged(0);
}

ComponentDialog::~ComponentDialog() = default;

void ComponentDialog::setupUi()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    // Common fields group
    QGroupBox *commonGroup = new QGroupBox(Lang.translate("dialog.component.basicInfo"), this);
    QFormLayout *commonForm = new QFormLayout(commonGroup);

    m_nameEdit = new QLineEdit(this);
    m_nameEdit->setPlaceholderText(Lang.translate("dialog.component.namePlaceholder"));
    commonForm->addRow(Lang.translate("dialog.component.nameLabel"), m_nameEdit);

    m_manufacturerEdit = new QLineEdit(this);
    m_manufacturerEdit->setPlaceholderText(Lang.translate("dialog.component.manufacturerPlaceholder"));
    commonForm->addRow(Lang.translate("dialog.component.manufacturerLabel"), m_manufacturerEdit);

    m_categoryCombo = new QComboBox(this);
    // Load categories from database
    auto categories = DatabaseManager::instance().fetchAllCategories();
    for (const auto &cat : categories)
    {
        m_categoryCombo->addItem(cat.name());
    }
    commonForm->addRow(Lang.translate("dialog.component.categoryLabel"), m_categoryCombo);

    m_quantitySpin = new QSpinBox(this);
    m_quantitySpin->setRange(0, 999999);
    m_quantitySpin->setValue(0);
    m_quantitySpin->setSuffix(Lang.translate("dialog.component.unitsSuffix"));
    commonForm->addRow(Lang.translate("dialog.component.quantityLabel"), m_quantitySpin);

    mainLayout->addWidget(commonGroup);

    // Stacked widget for type-specific fields
    m_stackedWidget = new QStackedWidget(this);

    // Page 0: Passive component fields
    QGroupBox *passiveGroup = new QGroupBox(Lang.translate("dialog.component.passiveParams"), this);
    QFormLayout *passiveForm = new QFormLayout(passiveGroup);

    QHBoxLayout *valueLayout = new QHBoxLayout();
    m_valueSpin = new QDoubleSpinBox(this);
    m_valueSpin->setRange(0.001, 999999);
    m_valueSpin->setDecimals(3);
    m_valueSpin->setValue(1.0);
    valueLayout->addWidget(m_valueSpin);

    m_valueMultiplier = new QComboBox(this);
    m_valueMultiplier->addItem(Lang.translate("multipliers.pico"), 1e-12);
    m_valueMultiplier->addItem(Lang.translate("multipliers.nano"), 1e-9);
    m_valueMultiplier->addItem(Lang.translate("multipliers.micro"), 1e-6);
    m_valueMultiplier->addItem(Lang.translate("multipliers.milli"), 1e-3);
    m_valueMultiplier->addItem(Lang.translate("multipliers.base"), 1.0);
    m_valueMultiplier->addItem(Lang.translate("multipliers.kilo"), 1e3);
    m_valueMultiplier->addItem(Lang.translate("multipliers.mega"), 1e6);
    m_valueMultiplier->addItem(Lang.translate("multipliers.giga"), 1e9);
    m_valueMultiplier->setCurrentIndex(4); // Default to base
    valueLayout->addWidget(m_valueMultiplier);

    QWidget *valueWidget = new QWidget(this);
    valueWidget->setLayout(valueLayout);
    passiveForm->addRow(Lang.translate("dialog.component.valueLabel"), valueWidget);

    m_packageEdit = new QLineEdit(this);
    passiveForm->addRow(Lang.translate("dialog.component.packageLabel"), m_packageEdit);

    m_stackedWidget->addWidget(passiveGroup);

    // Page 1: Active component fields
    QGroupBox *activeGroup = new QGroupBox(Lang.translate("dialog.component.activeParams"), this);
    QFormLayout *activeForm = new QFormLayout(activeGroup);

    m_voltageSpin = new QDoubleSpinBox(this);
    m_voltageSpin->setRange(0.1, 1000.0);
    m_voltageSpin->setDecimals(1);
    m_voltageSpin->setValue(5.0);
    m_voltageSpin->setSuffix(Lang.translate("dialog.component.voltageSuffix"));
    activeForm->addRow(Lang.translate("dialog.component.voltageLabel"), m_voltageSpin);

    m_pinCountSpin = new QSpinBox(this);
    m_pinCountSpin->setRange(1, 500);
    m_pinCountSpin->setValue(8);
    m_pinCountSpin->setSuffix(Lang.translate("dialog.component.pinsSuffix"));
    activeForm->addRow(Lang.translate("dialog.component.pinCountLabel"), m_pinCountSpin);

    m_datasheetEdit = new QLineEdit(this);
    m_datasheetEdit->setPlaceholderText(Lang.translate("dialog.component.datasheetPlaceholder"));
    activeForm->addRow(Lang.translate("dialog.component.datasheetLabel"), m_datasheetEdit);

    m_stackedWidget->addWidget(activeGroup);

    // Page 2: Generic (Connector, Other)
    QGroupBox *otherGroup = new QGroupBox(Lang.translate("dialog.component.otherParams"), this);
    QFormLayout *otherForm = new QFormLayout(otherGroup);

    QLabel *infoLabel = new QLabel("No additional parameters required for this category.", this);
    infoLabel->setWordWrap(true);
    otherForm->addRow(infoLabel);

    m_stackedWidget->addWidget(otherGroup);

    mainLayout->addWidget(m_stackedWidget);

    // Button box
    QDialogButtonBox *buttonBox = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    m_okButton = buttonBox->button(QDialogButtonBox::Ok);
    m_cancelButton = buttonBox->button(QDialogButtonBox::Cancel);

    mainLayout->addWidget(buttonBox);
}

void ComponentDialog::setupConnections()
{
    connect(m_categoryCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &ComponentDialog::onCategoryChanged);
    connect(m_okButton, &QPushButton::clicked, this, &ComponentDialog::validateAndAccept);
    connect(m_cancelButton, &QPushButton::clicked, this, &QDialog::reject);
}

void ComponentDialog::onCategoryChanged(int index)
{
    QString categoryName = m_categoryCombo->itemText(index);
    CategoryInfo catInfo = DatabaseManager::instance().fetchCategoryByName(categoryName);
    updateFieldsForCategory(catInfo);
}

void ComponentDialog::updateFieldsForCategory(const CategoryInfo &catInfo)
{
    if (catInfo.isPassive())
    {
        m_stackedWidget->setCurrentIndex(0);
    }
    else if (catInfo.isActive())
    {
        m_stackedWidget->setCurrentIndex(1);
    }
    else
    {
        m_stackedWidget->setCurrentIndex(2);
    }
}

void ComponentDialog::setComponent(const Component *component)
{
    if (!component)
        return;

    // Set common fields
    m_nameEdit->setText(component->getName());
    m_manufacturerEdit->setText(component->getManufacturer());
    m_quantitySpin->setValue(component->getQuantity());

    // Set category
    int catIndex = m_categoryCombo->findText(component->getCategory());
    if (catIndex >= 0)
    {
        m_categoryCombo->setCurrentIndex(catIndex);
    }

    // Set type-specific fields
    if (auto *passive = dynamic_cast<const PassiveComponent *>(component))
    {
        double value = passive->getValue();

        // Find appropriate multiplier
        int multiplierIndex = 4; // base
        if (value >= 1e9)
        {
            value /= 1e9;
            multiplierIndex = 7;
        }
        else if (value >= 1e6)
        {
            value /= 1e6;
            multiplierIndex = 6;
        }
        else if (value >= 1e3)
        {
            value /= 1e3;
            multiplierIndex = 5;
        }
        else if (value >= 1)
        {
            multiplierIndex = 4;
        }
        else if (value >= 1e-3)
        {
            value *= 1e3;
            multiplierIndex = 3;
        }
        else if (value >= 1e-6)
        {
            value *= 1e6;
            multiplierIndex = 2;
        }
        else if (value >= 1e-9)
        {
            value *= 1e9;
            multiplierIndex = 1;
        }
        else
        {
            value *= 1e12;
            multiplierIndex = 0;
        }

        m_valueSpin->setValue(value);
        m_valueMultiplier->setCurrentIndex(multiplierIndex);

        m_packageEdit->setText(passive->getPackage());
    }
    else if (auto *active = dynamic_cast<const ActiveComponent *>(component))
    {
        m_voltageSpin->setValue(active->getOperatingVoltage());
        m_pinCountSpin->setValue(active->getPinCount());
        m_datasheetEdit->setText(active->getDatasheetLink());
    }
}

std::unique_ptr<Component> ComponentDialog::getComponent() const
{
    QString name = m_nameEdit->text().trimmed();
    QString manufacturer = m_manufacturerEdit->text().trimmed();
    int quantity = m_quantitySpin->value();
    QString categoryName = m_categoryCombo->currentText();

    CategoryInfo catInfo = DatabaseManager::instance().fetchCategoryByName(categoryName);

    if (catInfo.isPassive())
    {
        double value = m_valueSpin->value();
        double multiplier = m_valueMultiplier->currentData().toDouble();
        value *= multiplier;

        QString unit = catInfo.defaultUnit();
        QString package = m_packageEdit->text().trimmed();

        return std::make_unique<PassiveComponent>(
            -1, name, manufacturer, quantity, categoryName,
            value, unit, package);
    }
    else if (catInfo.isActive())
    {
        double voltage = m_voltageSpin->value();
        int pinCount = m_pinCountSpin->value();
        QString datasheet = m_datasheetEdit->text().trimmed();

        return std::make_unique<ActiveComponent>(
            -1, name, manufacturer, quantity, categoryName,
            voltage, pinCount, datasheet);
    }
    else
    {
        // For Connector/Other or custom categories, create a passive component with minimal data
        return std::make_unique<PassiveComponent>(
            -1, name, manufacturer, quantity, categoryName,
            0.0, "", "");
    }
}

bool ComponentDialog::validateInputs()
{
    // Name is required
    if (m_nameEdit->text().trimmed().isEmpty())
    {
        QMessageBox::warning(this, Lang.translate("dialog.component.validationError"),
                             Lang.translate("dialog.component.nameRequired"));
        m_nameEdit->setFocus();
        return false;
    }

    // Quantity cannot be negative (already enforced by spinbox, but double-check)
    if (m_quantitySpin->value() < 0)
    {
        QMessageBox::warning(this, Lang.translate("dialog.component.validationError"),
                             Lang.translate("dialog.component.quantityNegative"));
        m_quantitySpin->setFocus();
        return false;
    }

    CategoryInfo catInfo = DatabaseManager::instance().fetchCategoryByName(m_categoryCombo->currentText());

    // Passive-specific validation
    if (catInfo.isPassive())
    {
        if (m_valueSpin->value() <= 0)
        {
            QMessageBox::warning(this, Lang.translate("dialog.component.validationError"),
                                 "Please enter a positive component value.");
            m_valueSpin->setFocus();
            return false;
        }
        if (m_packageEdit->text().trimmed().isEmpty())
        {
            QMessageBox::warning(this, Lang.translate("dialog.component.validationError"),
                                 "Please select or enter a package type.");
            m_packageEdit->setFocus();
            return false;
        }
    }

    // Active-specific validation
    if (catInfo.isActive())
    {
        if (m_voltageSpin->value() <= 0)
        {
            QMessageBox::warning(this, Lang.translate("dialog.component.validationError"),
                                 "Please enter a positive operating voltage.");
            m_voltageSpin->setFocus();
            return false;
        }
        if (m_pinCountSpin->value() < 1)
        {
            QMessageBox::warning(this, Lang.translate("dialog.component.validationError"),
                                 "Pin count must be at least 1.");
            m_pinCountSpin->setFocus();
            return false;
        }
    }

    return true;
}

void ComponentDialog::validateAndAccept()
{
    if (validateInputs())
    {
        accept();
    }
}
