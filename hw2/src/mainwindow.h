#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include <QtDebug>
#include <QMainWindow>
#include <vector>
#include <QMenu>
#include <QLabel>
#include <QLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QTreeWidget>
#include <QStringList>
#include <QComboBox>
#include <QTabWidget>
#include <QTableWidget>
#include <QHeaderView>
QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT
    QTreeWidget *local_site;
    QComboBox *local_memo;
    QStringList *local_memo_dir;

    QTreeWidget *remote_site;
    QComboBox *remote_memo;
    QStringList *remote_memo_dir;

    QTreeWidget *local_file_view;
    QTreeWidget *remote_file_view;

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void mainmenu();
    QHBoxLayout* input_field();
    void local_view();
    void remote_view();
    QHBoxLayout* memo_box();
    QWidget* status();

public slots:
    void click_item_inDir(QTreeWidgetItem *);
    void dir_combo_changed(const QString&);
    void remote_click_item_inDir(QTreeWidgetItem *);
    void remote_dir_combo_changed(const QString&);

private:
    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
