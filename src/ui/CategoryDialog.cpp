#include "ui/CategoryDialog.h"
#include "config/LanguageManager.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QLabel>
#include <QDialogButtonBox>
#include <QMessageBox>

CategoryDialog::CategoryDialog(QWidget *parent)
    : QDialog(parent), m_nameEdit(nullptr), m_passiveCheck(nullptr), m_activeCheck(nullptr), m_unitEdit(nullptr), m_okButton(nullptr), m_cancelButton(nullptr), m_categoryId(-1), m_isBuiltInCategory(false)
{
    setWindowTitle(Lang.translate("menu.category.title"));
    setMinimumWidth(400);
    setModal(true);

    setupUi();
    setupConnections();
}

CategoryDialog::~CategoryDialog() = default;

void CategoryDialog::setupUi()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    // Basic info group
    QGroupBox *basicGroup = new QGroupBox(Lang.translate("dialog.category.categoryInfo"), this);
    QFormLayout *basicForm = new QFormLayout(basicGroup);

    m_nameEdit = new QLineEdit(this);
    m_nameEdit->setPlaceholderText(Lang.translate("dialog.category.namePlaceholder"));
    basicForm->addRow(Lang.translate("dialog.category.nameLabel"), m_nameEdit);

    mainLayout->addWidget(basicGroup);

    // Type group
    QGroupBox *typeGroup = new QGroupBox(Lang.translate("dialog.category.componentType"), this);
    QVBoxLayout *typeLayout = new QVBoxLayout(typeGroup);

    QLabel *typeInfo = new QLabel(Lang.translate("dialog.category.typeInfo"), typeGroup);
    typeInfo->setWordWrap(true);
    typeLayout->addWidget(typeInfo);

    m_passiveCheck = new QCheckBox(Lang.translate("dialog.category.passiveCheck"), this);
    typeLayout->addWidget(m_passiveCheck);

    m_activeCheck = new QCheckBox(Lang.translate("dialog.category.activeCheck"), this);
    typeLayout->addWidget(m_activeCheck);

    QLabel *noteLabel = new QLabel(
        QString("<i>%1</i>").arg(Lang.translate("dialog.category.typeNote")), typeGroup);
    noteLabel->setWordWrap(true);
    typeLayout->addWidget(noteLabel);

    mainLayout->addWidget(typeGroup);

    // Unit group (for passive)
    QGroupBox *unitGroup = new QGroupBox(Lang.translate("dialog.category.defaultUnit"), this);
    QFormLayout *unitForm = new QFormLayout(unitGroup);

    m_unitEdit = new QLineEdit(this);
    m_unitEdit->setPlaceholderText(Lang.translate("dialog.category.unitPlaceholder"));
    m_unitEdit->setMaxLength(10);
    unitForm->addRow(Lang.translate("dialog.category.unitLabel"), m_unitEdit);

    mainLayout->addWidget(unitGroup);

    // Button box
    QDialogButtonBox *buttonBox = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    m_okButton = buttonBox->button(QDialogButtonBox::Ok);
    m_cancelButton = buttonBox->button(QDialogButtonBox::Cancel);

    mainLayout->addWidget(buttonBox);
}

void CategoryDialog::setupConnections()
{
    connect(m_okButton, &QPushButton::clicked, this, &CategoryDialog::validateAndAccept);
    connect(m_cancelButton, &QPushButton::clicked, this, &QDialog::reject);
    connect(m_passiveCheck, &QCheckBox::toggled, this, &CategoryDialog::onTypeChanged);
    connect(m_activeCheck, &QCheckBox::toggled, this, &CategoryDialog::onTypeChanged);
}

void CategoryDialog::onTypeChanged()
{
    m_unitEdit->setEnabled(m_passiveCheck->isChecked());
}

void CategoryDialog::setCategory(const CategoryInfo &category)
{
    m_categoryId = category.id();
    m_nameEdit->setText(category.name());
    m_passiveCheck->setChecked(category.isPassive());
    m_activeCheck->setChecked(category.isActive());
    m_unitEdit->setText(category.defaultUnit());
    m_unitEdit->setEnabled(category.isPassive());
}

void CategoryDialog::setSystemCategory(bool isBuiltIn)
{
    m_isBuiltInCategory = isBuiltIn;
    if (isBuiltIn)
    {
        // System categories can't have their name changed
        m_nameEdit->setReadOnly(true);
        m_nameEdit->setStyleSheet("background-color: #f0f0f0;");
    }
}

CategoryInfo CategoryDialog::getCategory() const
{
    return CategoryInfo(
        m_categoryId,
        m_nameEdit->text().trimmed(),
        m_passiveCheck->isChecked(),
        m_activeCheck->isChecked(),
        m_unitEdit->text().trimmed());
}

bool CategoryDialog::validateInputs()
{
    if (m_nameEdit->text().trimmed().isEmpty())
    {
        QMessageBox::warning(this, Lang.translate("dialog.component.validationError"),
                             Lang.translate("dialog.category.nameRequired"));
        m_nameEdit->setFocus();
        return false;
    }

    // Check for reserved names if creating new
    if (m_categoryId < 0)
    {
        QString name = m_nameEdit->text().trimmed();
        QStringList reserved = {"Resistor", "Capacitor", "Inductor", "IC",
                                "Transistor", "Diode", "Connector", "Other"};
        if (reserved.contains(name, Qt::CaseInsensitive))
        {
            QMessageBox::warning(this, Lang.translate("dialog.component.validationError"),
                                 QString("'%1' is a reserved category name.").arg(name));
            m_nameEdit->setFocus();
            return false;
        }
    }

    return true;
}

void CategoryDialog::validateAndAccept()
{
    if (validateInputs())
    {
        accept();
    }
}
