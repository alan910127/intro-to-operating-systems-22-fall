#include "mainwindow.h"

#include <iostream>

#include "client.h"
#include "ui_mainwindow.h"

enum type_F { _DIR = 0, _FILE = 1 };
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      local_site(new QTreeWidget),
      local_memo(new QComboBox),
      local_memo_dir(new QStringList),
      remote_site(new QTreeWidget),
      remote_memo(new QComboBox),
      remote_memo_dir(new QStringList),
      local_file_view(new QTreeWidget),
      remote_file_view(new QTreeWidget),
      ui(new Ui::MainWindow) {
  ui->setupUi(this);

  // about menu
  mainmenu();
  auto widget = new QWidget();
  auto overall_vertical = new QVBoxLayout();

  // directory view
  auto dir_horizon = new QHBoxLayout();

  local_view();
  remote_view_init();
  dir_horizon->addWidget(local_site);
  dir_horizon->addWidget(remote_site);
  // file view
  auto file_horizon = new QHBoxLayout();

  file_horizon->addWidget(local_file_view);
  file_horizon->addWidget(remote_file_view);

  // line up the component vertically
  overall_vertical->addLayout(input_field());
  overall_vertical->addLayout(memo_box());
  overall_vertical->addLayout(dir_horizon);
  overall_vertical->addLayout(file_horizon);
  overall_vertical->addWidget(status());
  widget->setLayout(overall_vertical);
  // set the centralwidget in ui to your widget.
  setCentralWidget(widget);
}

void MainWindow::closeEvent(QCloseEvent *event) {
  if (ftp) {
    ftp->LogoutTest(currentSession);
    delete ftp;
    ftp = nullptr;
  }

  event->accept();
}

void MainWindow::remote_dir_combo_changed(const QString &text) {
  QTreeWidgetItemIterator it(remote_site);
  while (*it) {
    if ((*it)->text(0) == text) remote_site->setCurrentItem((*it));
    ++it;
  }
  QTreeWidgetItemIterator it_f(remote_file_view);
  while (*it_f) {
    if ((*it_f)->text(0) == text) remote_file_view->setCurrentItem((*it_f));
    ++it_f;
  }
  remote_file_view->currentItem()->setExpanded(true);
}

void MainWindow::remote_click_item_inDir(QTreeWidgetItem *item) {
  if (!remote_memo_dir->contains(item->text(0))) {
    remote_memo->addItem(item->text(0));
    remote_memo_dir->append(item->text(0));
  }
  remote_memo->setCurrentText(item->text(0));
}

void MainWindow::remote_site_click_item(QTreeWidgetItem *item) {
  if (ftp) {
    std::string response = ftp->ChangeWorkingDirectoryTest(
        currentSession, item->text(0).toStdString());
    if (response == "0") {
      std::vector<FtpClient::DirectoryVec> listResponse =
          ftp->ListDirectoryTest(currentSession);

      item->takeChildren();
      for (std::vector<FtpClient::DirectoryVec>::iterator iter =
               listResponse.begin();
           iter != listResponse.end(); iter++) {
        if ((*iter).type == "_DIR") {
          auto temp = new QTreeWidgetItem(
              *new QStringList() << (*iter).name.c_str(), _DIR);
          item->addChild(temp);
        }
      }
      item->setExpanded(true);
      remote_file_view_update(listResponse);
    }
  }
}

void MainWindow::dir_combo_changed(const QString &text) {
  QTreeWidgetItemIterator it(local_site);
  while (*it) {
    if ((*it)->text(0) == text) local_site->setCurrentItem((*it));
    ++it;
  }
  QTreeWidgetItemIterator it_f(local_file_view);
  while (*it_f) {
    if ((*it_f)->text(0) == text) local_file_view->setCurrentItem((*it_f));
    ++it_f;
  }
  local_file_view->currentItem()->setExpanded(true);
}
void MainWindow::click_item_inDir(QTreeWidgetItem *item) {
  if (!local_memo_dir->contains(item->text(0))) {
    local_memo->addItem(item->text(0));
    local_memo_dir->append(item->text(0));
  }
  local_memo->setCurrentText(item->text(0));
}
// menu implement
void MainWindow::mainmenu() {
  auto menu = new QMenu("File");
  auto submenu = menu->addMenu("New");
  submenu->addAction("New tab");
  menu->addMenu(submenu);

  auto actLogout = menu->addAction("Logout");
  auto actExit = menu->addAction("Exit");
  ui->menubar->addMenu(menu);

  connect(actLogout, SIGNAL(triggered()), this, SLOT(logout_event()));
  connect(actExit, SIGNAL(triggered()), this, SLOT(close()));

  auto transfer_menu = new QMenu("Transfer");
  transfer_menu->addAction("Process Queue");
  transfer_menu->addAction("Default File exist action....");
  auto tr_submenu = transfer_menu->addMenu("Transfer Type");
  tr_submenu->addAction("AUTO");
  tr_submenu->addAction("ASCII");
  tr_submenu->addAction("Binary");
  ui->menubar->addMenu(transfer_menu);
}

QHBoxLayout *MainWindow::input_field() {
  // input field

  auto input_horizon = new QHBoxLayout();

  input_horizon->addWidget(new QLabel("Host: "));
  connection_info.push_back(new QLineEdit("127.0.0.1"));
  input_horizon->addWidget(connection_info.back());

  input_horizon->addWidget(new QLabel("Username: "));
  connection_info.push_back(new QLineEdit(""));
  input_horizon->addWidget(connection_info.back());

  input_horizon->addWidget(new QLabel("Password: "));
  connection_info.push_back(new QLineEdit(""));
  input_horizon->addWidget(connection_info.back());

  input_horizon->addWidget(new QLabel("Port: "));
  connection_info.push_back(new QLineEdit(""));
  input_horizon->addWidget(connection_info.back());

  auto button = new QPushButton("Connect");
  input_horizon->addWidget(button);
  connect(button, SIGNAL(clicked()), this, SLOT(connect_event()));

  return input_horizon;
}

void MainWindow::connect_event() {
  std::string target_str = connection_info[0]->text().toStdString() + ":" +
                           connection_info[3]->text().toStdString();
  if (ftp) {
    // std::cout<<"close ftp"<<std::endl;
    ftp->LogoutTest(currentSession);
    delete ftp;
    ftp = nullptr;
    remote_view_clear();
  }

  ftp = new FtpClient(
      grpc::CreateChannel(target_str, grpc::InsecureChannelCredentials()));
  if (ftp) {
    currentSession = ftp->LoginTest(connection_info[1]->text().toStdString(),
                                    connection_info[2]->text().toStdString());
    if (currentSession != "0") {
      std::string workingDirectory =
          ftp->GetWorkingDirectoryTest(currentSession);
      std::vector<FtpClient::DirectoryVec> listResponse =
          ftp->ListDirectoryTest(currentSession);
      remote_view_update(listResponse);
    }
  } else
    std::cerr << "Create channel to " << target_str << "failed!" << std::endl;
}

void MainWindow::logout_event() {
  if (ftp) {
    // std::cout<<"close ftp"<<std::endl;
    ftp->LogoutTest(currentSession);
    delete ftp;
    ftp = nullptr;
    remote_view_clear();
  }
}

void MainWindow::local_view() {
  local_site->header()->hide();

  auto first_layer = new QTreeWidgetItem(*new QStringList() << "usr", _DIR);
  local_site->addTopLevelItem(first_layer);
  auto second_layer = new QTreeWidgetItem(*new QStringList() << "bin", _DIR);
  first_layer->addChild(second_layer);
  second_layer->addChild(new QTreeWidgetItem(*new QStringList() << "apt"
                                                                << "18824",
                                             _FILE));

  QTreeWidgetItemIterator local_it(local_site);
  while (*local_it) {
    if ((*local_it)->type() == _FILE) (*local_it)->setHidden(true);
    ++local_it;
  }
  connect(local_site, SIGNAL(itemClicked(QTreeWidgetItem *, int)), this,
          SLOT(click_item_inDir(QTreeWidgetItem *)));
  local_file_view->setHeaderLabels(*new QStringList() << "Filename"
                                                      << "Filesize");
  local_file_view->addTopLevelItem(first_layer->clone());

  connect(local_file_view, SIGNAL(itemClicked(QTreeWidgetItem *, int)), this,
          SLOT(click_item_inDir(QTreeWidgetItem *)));
}
void MainWindow::remote_view_init() {
  remote_site->header()->hide();
  auto remote_first_layer =
      new QTreeWidgetItem(*new QStringList() << "/", _DIR);
  remote_site->addTopLevelItem(remote_first_layer);

  remote_file_view->setHeaderLabels(*new QStringList() << "Filename"
                                                       << "Filesize");

  connect(remote_site, SIGNAL(itemClicked(QTreeWidgetItem *, int)), this,
          SLOT(remote_site_click_item(QTreeWidgetItem *)));
  connect(remote_file_view, SIGNAL(itemClicked(QTreeWidgetItem *, int)), this,
          SLOT(remote_click_item_inDir(QTreeWidgetItem *)));
}

void MainWindow::remote_view_clear() {
  remote_site->clear();
  remote_file_view->clear();

  remote_site->header()->hide();
  auto remote_first_layer =
      new QTreeWidgetItem(*new QStringList() << "/", _DIR);
  remote_site->addTopLevelItem(remote_first_layer);

  remote_file_view->addTopLevelItem(remote_first_layer->clone());
}

void MainWindow::remote_view_update(
    std::vector<FtpClient::DirectoryVec> directoryVec) {
  auto first_layer = remote_site->topLevelItem(0);
  for (std::vector<FtpClient::DirectoryVec>::iterator iter =
           directoryVec.begin();
       iter != directoryVec.end(); iter++) {
    if ((*iter).type == "_DIR") {
      auto temp =
          new QTreeWidgetItem(*new QStringList() << (*iter).name.c_str(), _DIR);
      first_layer->addChild(temp);
    } else if ((*iter).type == "_FILE") {
      auto temp = new QTreeWidgetItem(
          *new QStringList()
              << (*iter).name.c_str() << std::to_string((*iter).size).c_str(),
          _FILE);
      first_layer->addChild(temp);
    }
  }
  remote_file_view->clear();

  QTreeWidgetItemIterator remote_it(remote_site);
  while (*remote_it) {
    if ((*remote_it)->type() == _FILE) (*remote_it)->setHidden(true);
    ++remote_it;
  }
}

void MainWindow::remote_file_view_update(
    std::vector<FtpClient::DirectoryVec> directoryVec) {
  remote_file_view->clear();
  for (std::vector<FtpClient::DirectoryVec>::iterator iter =
           directoryVec.begin();
       iter != directoryVec.end(); iter++) {
    if ((*iter).type == "_DIR") {
      // char name[] = (*iter).name;
      auto temp =
          new QTreeWidgetItem(*new QStringList() << (*iter).name.c_str(), _DIR);
      remote_file_view->addTopLevelItem(temp);
    } else if ((*iter).type == "_FILE") {
      auto temp = new QTreeWidgetItem(
          *new QStringList()
              << (*iter).name.c_str() << std::to_string((*iter).size).c_str(),
          _FILE);
      remote_file_view->addTopLevelItem(temp);
    }
  }
}
QHBoxLayout *MainWindow::memo_box() {
  auto memo_horizon = new QHBoxLayout();
  local_memo->addItems(*local_memo_dir);
  memo_horizon->addWidget(new QLabel("Local site:"), 10);
  memo_horizon->addWidget(local_memo, 40);

  connect(local_memo, SIGNAL(currentTextChanged(QString)), this,
          SLOT(dir_combo_changed(QString)));

  remote_memo->addItems(*local_memo_dir);
  memo_horizon->addWidget(new QLabel("Remote site:"), 10);
  memo_horizon->addWidget(remote_memo, 40);

  connect(remote_memo, SIGNAL(currentTextChanged(QString)), this,
          SLOT(remote_dir_combo_changed(QString)));
  return memo_horizon;
}
QWidget *MainWindow::status() {
  auto transfer_status = new QTabWidget();
  auto queue = new QTableWidget(1, 6);
  auto failed = new QTableWidget(1, 6);

  queue->setHorizontalHeaderLabels(
      *new QStringList({"Server/Local file", "Direction", "Remote file", "Size",
                        "Priority", "Status"}));
  queue->verticalHeader()->hide();
  transfer_status->setTabPosition(QTabWidget::South);
  transfer_status->addTab(queue, "Queued");

  queue->horizontalHeader()->setStretchLastSection(true);
  queue->verticalHeader()->setStretchLastSection(true);

  failed->setHorizontalHeaderLabels(
      *new QStringList({"Server/Local file", "Direction", "Remote file", "Size",
                        "Priority", "Status"}));
  failed->verticalHeader()->hide();
  transfer_status->setTabPosition(QTabWidget::South);
  transfer_status->addTab(failed, "Failed");

  failed->horizontalHeader()->setStretchLastSection(true);
  failed->verticalHeader()->setStretchLastSection(true);
  return transfer_status;
}
MainWindow::~MainWindow() { delete ui; }
