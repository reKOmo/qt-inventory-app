#ifndef CATEGORYDIALOG_H
#define CATEGORYDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QCheckBox>
#include <QComboBox>
#include <QPushButton>
#include "models/CategoryInfo.h"

class CategoryDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CategoryDialog(QWidget *parent = nullptr);
    ~CategoryDialog() override;

    void setCategory(const CategoryInfo &category);
    CategoryInfo getCategory() const;
    void setSystemCategory(bool isSystem);

private slots:
    void validateAndAccept();
    void onTypeChanged();

private:
    void setupUi();
    void setupConnections();
    bool validateInputs();

    QLineEdit *m_nameEdit;
    QCheckBox *m_passiveCheck;
    QCheckBox *m_activeCheck;
    QLineEdit *m_unitEdit;
    QPushButton *m_okButton;
    QPushButton *m_cancelButton;

    int m_categoryId;
    bool m_isBuiltInCategory;
};

#endif // CATEGORYDIALOG_H
