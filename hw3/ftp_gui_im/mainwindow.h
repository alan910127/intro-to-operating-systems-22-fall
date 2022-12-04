#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include <QCloseEvent>
#include <QComboBox>
#include <QHeaderView>
#include <QLabel>
#include <QLayout>
#include <QLineEdit>
#include <QMainWindow>
#include <QMenu>
#include <QPushButton>
#include <QStringList>
#include <QTabWidget>
#include <QTableWidget>
#include <QTreeWidget>
#include <QtDebug>
#include <vector>

#include "client.h"
QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
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
  QHBoxLayout *input_field();
  void local_view();
  void remote_view_init();
  void remote_view_clear();
  void remote_view_update(std::vector<FtpClient::DirectoryVec> directoryVec);
  void remote_file_view_update(
      std::vector<FtpClient::DirectoryVec> directoryVec);
  void closeEvent(QCloseEvent *event);
  QHBoxLayout *memo_box();
  QWidget *status();

 public slots:
  void click_item_inDir(QTreeWidgetItem *);
  void connect_event();
  void logout_event();
  void dir_combo_changed(const QString &);
  void remote_click_item_inDir(QTreeWidgetItem *);
  void remote_site_click_item(QTreeWidgetItem *);
  void remote_dir_combo_changed(const QString &);

 private:
  Ui::MainWindow *ui;
  std::vector<QLineEdit *> connection_info;
  FtpClient *ftp = nullptr;
  std::string currentSession;
};
#endif  // MAINWINDOW_H
