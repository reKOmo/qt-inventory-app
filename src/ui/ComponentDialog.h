#ifndef COMPONENTDIALOG_H
#define COMPONENTDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QComboBox>
#include <QStackedWidget>
#include <memory>
#include "models/Component.h"
#include "models/CategoryInfo.h"

class ComponentDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ComponentDialog(QWidget *parent = nullptr);
    ~ComponentDialog() override;

    void setComponent(const Component *component);
    std::unique_ptr<Component> getComponent() const;

private slots:
    void onCategoryChanged(int index);
    void validateAndAccept();

private:
    void setupUi();
    void setupConnections();
    void updateFieldsForCategory(const CategoryInfo &catInfo);

    bool validateInputs();

    QLineEdit *m_nameEdit;
    QLineEdit *m_manufacturerEdit;
    QComboBox *m_categoryCombo;
    QSpinBox *m_quantitySpin;

    QStackedWidget *m_stackedWidget;

    // Passive component fields (page 0)
    QDoubleSpinBox *m_valueSpin;
    QComboBox *m_valueMultiplier;
    QLineEdit *m_packageEdit;

    // Active component fields (page 1)
    QDoubleSpinBox *m_voltageSpin;
    QSpinBox *m_pinCountSpin;
    QLineEdit *m_datasheetEdit;

    QPushButton *m_okButton;
    QPushButton *m_cancelButton;
};

#endif // COMPONENTDIALOG_H
