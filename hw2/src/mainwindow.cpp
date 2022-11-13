#include "mainwindow.h"

#include "ui_mainwindow.h"

enum type_F { _DIR = 0, _FILE = 1 };

MainWindow::MainWindow(QWidget* parent)
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
  remote_view();
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

void MainWindow::remote_dir_combo_changed(const QString& text) {
  for (QTreeWidgetItemIterator it(remote_site); *it; ++it) {
    if ((*it)->text(0) == text) {
      remote_site->setCurrentItem((*it));
    }
  }

  for (QTreeWidgetItemIterator it_f(remote_file_view); *it_f; ++it_f) {
    if ((*it_f)->text(0) == text) {
      remote_file_view->setCurrentItem((*it_f));
    }
  }

  remote_file_view->currentItem()->setExpanded(true);
}

void MainWindow::remote_click_item_inDir(QTreeWidgetItem* item) {
  if (!remote_memo_dir->contains(item->text(0))) {
    remote_memo->addItem(item->text(0));
    remote_memo_dir->append(item->text(0));
  }
  remote_memo->setCurrentText(item->text(0));
}

void MainWindow::dir_combo_changed(const QString& text) {
  for (QTreeWidgetItemIterator it(local_site); *it; ++it) {
    if ((*it)->text(0) == text) {
      local_site->setCurrentItem((*it));
    }
  }

  for (QTreeWidgetItemIterator it_f(local_file_view); *it_f; ++it_f) {
    if ((*it_f)->text(0) == text) {
      local_file_view->setCurrentItem((*it_f));
    }
  }

  local_file_view->currentItem()->setExpanded(true);
}

void MainWindow::click_item_inDir(QTreeWidgetItem* item) {
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
  //
  auto action = menu->addAction("Exit");
  connect(action, SIGNAL(triggered()), this, SLOT(close()));

  ui->menubar->addMenu(menu);

  auto transfer_menu = new QMenu("Transfer");
  transfer_menu->addAction("Process Queue");
  transfer_menu->addAction("Default File exist action...");
  auto tr_submenu = transfer_menu->addMenu("Transfer Type");
  tr_submenu->addAction("AUTO");
  tr_submenu->addAction("ASCII");
  tr_submenu->addAction("BINARY");
  ui->menubar->addMenu(transfer_menu);
}

QHBoxLayout* MainWindow::input_field() {
  // input field

  auto input_horizon = new QHBoxLayout();

  std::vector<QWidget*> userinput_field = {
      new QLabel("Host: "),      new QLineEdit(),  // Hostname
      new QLabel("Username: "),  new QLineEdit(),  // Username
      new QLabel("Password: "),  new QLineEdit(),  // Password
      new QLabel("Port: "),      new QLineEdit(),  // Port
      new QPushButton("Connect")                   // Connect
  };

  for (auto uf : userinput_field) {
    input_horizon->addWidget(uf);
  }

  return input_horizon;
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

  for (QTreeWidgetItemIterator local_it(local_site); *local_it; ++local_it) {
    if ((*local_it)->type() == _FILE) {
      (*local_it)->setHidden(true);
    }
  }

  connect(local_site, SIGNAL(itemClicked(QTreeWidgetItem*, int)), this,
          SLOT(click_item_inDir(QTreeWidgetItem*)));
  local_file_view->setHeaderLabels(*new QStringList() << "Filename"
                                                      << "Filesize");
  local_file_view->addTopLevelItem(first_layer->clone());

  connect(local_file_view, SIGNAL(itemClicked(QTreeWidgetItem*, int)), this,
          SLOT(click_item_inDir(QTreeWidgetItem*)));
}

void MainWindow::remote_view() {
  remote_site->header()->hide();

  auto remote_first_layer =
      new QTreeWidgetItem(*new QStringList() << "For_Windows", _DIR);
  remote_site->addTopLevelItem(remote_first_layer);

  auto remote_second =
      new QTreeWidgetItem(*new QStringList() << "Anti_Virus", _DIR);
  remote_first_layer->addChild(remote_second);
  remote_second->addChild(new QTreeWidgetItem(*new QStringList() << "Kaspersky"
                                                                 << "42881",
                                              _FILE));

  for (QTreeWidgetItemIterator remote_it(remote_site); *remote_it;
       ++remote_it) {
    if ((*remote_it)->type() == _FILE) {
      (*remote_it)->setHidden(true);
    }
  }

  connect(remote_site, SIGNAL(itemClicked(QTreeWidgetItem*, int)), this,
          SLOT(remote_click_item_inDir(QTreeWidgetItem*)));
  remote_file_view->setHeaderLabels(*new QStringList() << "Filename"
                                                       << "Filesize");
  remote_file_view->addTopLevelItem(remote_first_layer->clone());

  connect(remote_file_view, SIGNAL(itemClicked(QTreeWidgetItem*, int)), this,
          SLOT(remote_click_item_inDir(QTreeWidgetItem*)));
}

QHBoxLayout* MainWindow::memo_box() {
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

QWidget* MainWindow::status() {
  auto transfer_status = new QTabWidget();
  auto queue = new QTableWidget(1, 6);

  queue->setHorizontalHeaderLabels(*new QStringList() << "Server/Local file"
                                                      << "Direction"
                                                      << "Remote file"
                                                      << "Size"
                                                      << "Priority"
                                                      << "Status");
  queue->verticalHeader()->hide();
  transfer_status->setTabPosition(QTabWidget::South);
  transfer_status->addTab(queue, "Queued");
  transfer_status->addTab(new QTabWidget(), "Failed");
  transfer_status->setTabPosition(QTabWidget::South);

  queue->horizontalHeader()->setStretchLastSection(true);
  queue->verticalHeader()->setStretchLastSection(true);

  return transfer_status;
}

MainWindow::~MainWindow() { delete ui; }
