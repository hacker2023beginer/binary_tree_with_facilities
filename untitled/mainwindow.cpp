#include "mainwindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QInputDialog>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), bst(new BSTWidget(this)), log(new QTextEdit(this))
{
    QWidget *central = new QWidget(this);
    setCentralWidget(central);

    QVBoxLayout *mainLayout = new QVBoxLayout(central);
    QHBoxLayout *btnLayout = new QHBoxLayout();

    QPushButton *addBtn = new QPushButton("Добавить", this);
    QPushButton *delBtn = new QPushButton("Удалить (в прошивке)", this);
    QPushButton *trvBtn = new QPushButton("Показать обходы", this);
    QPushButton *thrBtn = new QPushButton("Показать правую прошивку", this);

    btnLayout->addWidget(addBtn);
    btnLayout->addWidget(delBtn);
    btnLayout->addWidget(trvBtn);
    btnLayout->addWidget(thrBtn);

    log->setReadOnly(true);
    log->setFixedHeight(140);

    mainLayout->addLayout(btnLayout);
    mainLayout->addWidget(bst);
    mainLayout->addWidget(log);

    connect(addBtn, &QPushButton::clicked, this, &MainWindow::addNode);
    connect(delBtn, &QPushButton::clicked, this, &MainWindow::deleteNode);
    connect(trvBtn, &QPushButton::clicked, this, &MainWindow::showTraversals);
    connect(thrBtn, &QPushButton::clicked, this, &MainWindow::showRightThread);
    connect(bst, &BSTWidget::logUpdated, this, &MainWindow::updateLog);

    setWindowTitle("BST с правой прошивкой — демонстрация");
    resize(1100, 760);
}

MainWindow::~MainWindow() {}

void MainWindow::addNode() {
    bool ok;
    int val = QInputDialog::getInt(this, "Добавить узел", "Введите значение:", 0, -10000, 10000, 1, &ok);
    if (ok) bst->insertNode(val);
}

void MainWindow::deleteNode() {
    // удаление "из прошитого дерева": мы сперва строим прошивку, потом удаляем и показываем обновлённую прошивку
    // (пользователь вводит значение для удаления)
    bool ok;
    int val = QInputDialog::getInt(this, "Удалить узел", "Введите значение для удаления:", 0, -10000, 10000, 1, &ok);
    if (!ok) return;

    // сначала строим прошивку (если нужно)
    bst->showRightThreadOverlay(); // это покажет прошивку и авто-скроет через таймер
    // затем удаляем узел и показываем снова прошивку
    bst->deleteNode(val);
    bst->showRightThreadOverlay();
}

void MainWindow::showTraversals() {
    bst->showTraversals();
}

void MainWindow::showRightThread() {
    bst->showRightThreadOverlay();
}

void MainWindow::updateLog(const QString &text) {
    log->setText(text);
}
