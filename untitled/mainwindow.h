#pragma once
#include <QMainWindow>
#include <QPushButton>
#include <QTextEdit>
#include "bstwidget.h"

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    BSTWidget *bst;
    QTextEdit *log;

private slots:
    void addNode();
    void deleteNode();
    void showTraversals();
    void showRightThread();
    void updateLog(const QString &text);
};
