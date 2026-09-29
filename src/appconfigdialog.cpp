#include "appconfigdialog.h"
#include "../ui/ui_appconfigdialog.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QPushButton>
#include <QFile>
#include <QDir>

AppConfigDialog::AppConfigDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::AppConfigDialog)
    , newApp(nullptr)
    , existingItem(nullptr)
{
    ui->setupUi(this);

    connect(ui->selectIconBtn, &QPushButton::clicked, this, &AppConfigDialog::onSelectIconClicked);
    connect(ui->confirmBtn, &QPushButton::clicked, this, &AppConfigDialog::onConfirmClicked);
    connect(ui->cancelBtn, &QPushButton::clicked, this, &AppConfigDialog::onCancelClicked);

    ui->iconPathEdit->setReadOnly(true);
}

AppConfigDialog::AppConfigDialog(AppItem *existing, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::AppConfigDialog)
    , newApp(nullptr)
    , existingItem(existing)
{
    ui->setupUi(this);

    connect(ui->selectIconBtn, &QPushButton::clicked, this, &AppConfigDialog::onSelectIconClicked);
    connect(ui->confirmBtn, &QPushButton::clicked, this, &AppConfigDialog::onConfirmClicked);
    connect(ui->cancelBtn, &QPushButton::clicked, this, &AppConfigDialog::onCancelClicked);

    ui->iconPathEdit->setReadOnly(true);

    if (existingItem) {
        initFromItem(existingItem);
    }
}

AppConfigDialog::~AppConfigDialog()
{
    delete ui;
}

void AppConfigDialog::initFromItem(AppItem *item)
{
    if (!item) return;

    ui->appNameEdit->setText(item->getName());
    ui->iconPathEdit->setText(item->getIconPath());
}

void AppConfigDialog::onSelectIconClicked()
{
    QString fileName = QFileDialog::getOpenFileName(this,
        tr("Select App Icon"),
        QDir::currentPath(),
        tr("Image Files (*.png *.jpg *.ico *.bmp *.svg)"));

    if (!fileName.isEmpty()) {
        ui->iconPathEdit->setText(fileName);
    }
}

void AppConfigDialog::onConfirmClicked()
{
    QString appName = ui->appNameEdit->text().trimmed();
    QString iconPath = ui->iconPathEdit->text().trimmed();

    if (appName.isEmpty()) {
        QMessageBox::warning(this, tr("Notice"), tr("App name cannot be empty"));
        return;
    }

    if (!iconPath.isEmpty() && !QFile::exists(iconPath)) {
        QMessageBox::warning(this, tr("Notice"), tr("Icon file does not exist"));
        return;
    }

    // No QObject parent: the returned item is handed over to the caller's
    // AppItem tree, which takes ownership via addSubApp(). Parenting it here
    // would delete it together with this (stack-allocated) dialog, leaving a
    // dangling pointer inside the tree.
    newApp = new AppItem(appName, iconPath);
    accept();
}

void AppConfigDialog::onCancelClicked()
{
    reject();
}

AppItem* AppConfigDialog::getNewApp() const
{
    return newApp;
}
