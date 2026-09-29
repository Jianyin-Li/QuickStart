#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QListWidgetItem>
#include <QCloseEvent>
#include <QEvent>
#include <QTranslator>
#include <QComboBox>
#include <QLabel>
#include <QToolButton>
#include "appitem.h"
#include "appconfigdialog.h"
#include "funcconfigdialog.h"
#include "config.h"
#include "iconlistdelegate.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(AppItem *initialItem = nullptr, MainWindow *parentWindow = nullptr);
    ~MainWindow() override;

protected:
    void closeEvent(QCloseEvent *event) override;
    void changeEvent(QEvent *event) override;

private slots:
    void onIconListItemClicked(QListWidgetItem *item);
    void onAddAppClicked();
    void onAddFuncClicked();
    void showContextMenu(const QPoint &pos);
    void onEditItem();
    void onDeleteItem();
    void onOpenConfig();
    void onExit();
    void onAboutQuickStart();
    void onAboutQt();
    void onLanguageChanged(int index);
    void onToggleDarkMode(bool checked);
    void onBackClicked();

private:
    void refreshIconList();
    void saveConfig();
    void loadConfig();
    void setupContextMenu();
    void setupLanguageToggle();
    void retranslateLanguageToggle();
    void setupHeaderBar();

    Ui::MainWindow *ui;
    IconListDelegate *delegate;
    AppItem *currentItem;
    AppItem *rootItem;
    // True only for the window that loaded the tree; child windows share
    // rootItem by pointer and must never free it.
    bool ownsRootItem = false;
    // Window to return to when the back button is pressed. Tracked explicitly
    // rather than through parent() so child windows stay top-level QObjects.
    MainWindow *m_parentWindow = nullptr;
    QTranslator *currentTranslator = nullptr;

    QMenu *contextMenu;
    QAction *editAction;
    QAction *deleteAction;
    // Raw on purpose: it is never dereferenced after being cleared, and
    // ui->iconListWidget's destroyed() handler resets it (QPointer cannot be
    // used because QListWidgetItem is not a QObject).
    QListWidgetItem *contextMenuItem;

    QAction *actionOpen_config;
    QAction *actionExit;
    QAction *actionQuickStart;
    QAction *actionQt;
    QAction *actionApp;
    QAction *actionFunc;
    QAction *actionToggleDarkMode;

    QLabel *languageLabel = nullptr;
    QComboBox *languageCombo = nullptr;
    QWidget *m_headerWidget = nullptr;
    QToolButton *m_backBtn = nullptr;
    QLabel *m_titleLabel = nullptr;
    QString m_savedLanguage;
    bool m_darkMode = false;
    QString m_lightStyleSheet;
};
#endif // MAINWINDOW_H

