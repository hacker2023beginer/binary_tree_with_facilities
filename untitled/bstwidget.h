#pragma once
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QHash>
#include <QPointF>
#include <QTimer>

#include <functional>

struct Node {
    int value;
    Node* left;
    Node* right;
    Node* rightThread; // правая нить (второй указатель, не заменяющий right)
    Node(int v) : value(v), left(nullptr), right(nullptr), rightThread(nullptr) {}
};

class BSTWidget : public QGraphicsView {
    Q_OBJECT
public:
    explicit BSTWidget(QWidget *parent = nullptr);

    // операции из GUI
    void insertNode(int value);
    void deleteNode(int value);
    void showTraversals();        // соберёт и эмитит лог обходов
    void showRightThreadOverlay(); // построит прошивку и нарисует временно

signals:
    void logUpdated(const QString &text);

private:
    QGraphicsScene *scene;
    Node *root;
    QHash<Node*, QPointF> posMap; // позиция для каждого узла после рисования
    bool rightThreadVisible;
    QTimer overlayTimer; // если хотим авто-скрытие

    // вспомогательные
    Node* insert(Node* root, int value);
    Node* deleteNodeRecursive(Node* root, int key);
    void preorderDetailed(Node* n, QString& out, Node* parent = nullptr);
    void inorderDetailed(Node* n, QString& out, Node* parent = nullptr);
    void postorderDetailed(Node* n, QString& out, Node* parent = nullptr);

    void buildRightThread(Node* root);
    void printRightThread(Node* root, QString& out);

    // рисование
    void drawTree(Node* node, int x, int y, int dx);
    void drawRightThreadOverlay(Node* node);

    void redrawTree();

    // освобождение дерева (не обязательный, но полезный)
    void freeTree(Node* node);
};
