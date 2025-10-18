#include "bstwidget.h"
#include <QPen>
#include <QBrush>
#include <QGraphicsTextItem>
#include <QDebug>

BSTWidget::BSTWidget(QWidget *parent)
    : QGraphicsView(parent), scene(new QGraphicsScene(this)), root(nullptr), rightThreadVisible(false)
{
    setScene(scene);
    setRenderHint(QPainter::Antialiasing);

    // не обязательно: автоскрытие наложения через 5 секунд (если нужно)
    connect(&overlayTimer, &QTimer::timeout, this, [this](){
        rightThreadVisible = false;
        redrawTree();
    });
}

void BSTWidget::insertNode(int value) {
    root = insert(root, value);
    rightThreadVisible = false;
    redrawTree();
}

Node* BSTWidget::insert(Node* r, int value) {
    if (!r) return new Node(value);
    if (value < r->value) r->left = insert(r->left, value);
    else if (value > r->value) r->right = insert(r->right, value);
    // если равно — игнорируем (можно изменить по желанию)
    return r;
}

void BSTWidget::deleteNode(int value) {
    // удаляем из BST (предполагаем, что дерево было прошито ранее)
    root = deleteNodeRecursive(root, value);
    rightThreadVisible = false;
    redrawTree();
}

Node* BSTWidget::deleteNodeRecursive(Node* r, int key) {
    if (!r) return nullptr;
    if (key < r->value) r->left = deleteNodeRecursive(r->left, key);
    else if (key > r->value) r->right = deleteNodeRecursive(r->right, key);
    else {
        if (!r->left) { Node* t = r->right; delete r; return t; }
        else if (!r->right) { Node* t = r->left; delete r; return t; }
        else {
            Node* succ = r->right;
            while (succ->left) succ = succ->left;
            r->value = succ->value;
            r->right = deleteNodeRecursive(r->right, succ->value);
        }
    }
    return r;
}

// ===== Детальные обходы: вывод промежуточных шагов =====
//
// Правило: при попытке зайти в пустое поддерево в контексте узла parent
// выводим "0(parent_value)". При посещении узла выводим "(value)".
// Это даёт полную картину переходов.
//void BSTWidget::preorderDetailed(Node* n, QString& out, Node* parent, const char *) {
//    if (!n) {
//        if (parent) out += QString("0(%1) ").arg(parent->value);
//        return;
//    }
//    out += QString("(%1) ").arg(n->value); // посещение узла
//    // левый
//    if (n->left) preorderDetailed(n->left, out, n, "L");
//    else out += QString("0(%1) ").arg(n->value); // пустой левый шаг
//    // правый
//    if (n->right) preorderDetailed(n->right, out, n, "R");
//    else out += QString("0(%1) ").arg(n->value); // пустой правый шаг
//}
void BSTWidget::preorderDetailed(Node* n, QString &out, Node* parent) {
    if(!n) {
        if(parent) out += QString("0 %1 ").arg(parent->value);
        return;
    }

    // посещение вершины
    out += QString("(%1) ").arg(n->value);

    // левое поддерево
    if(n->left) preorderDetailed(n->left, out, n);
    else out += QString("0 %1 ").arg(n->value);

    // правое поддерево
    if(n->right) preorderDetailed(n->right, out, n);
    else out += QString("0 %1 ").arg(n->value);

    // возврат к родителю
    if(parent) out += QString("%1 ").arg(parent->value);
}


void BSTWidget::inorderDetailed(Node* n, QString &out, Node* parent) {
    if(!n) {
        if(parent) out += QString("0 %1 ").arg(parent->value);
        return;
    }

    // левое поддерево
    if(n->left) inorderDetailed(n->left, out, n);
    else out += QString("0 %1 ").arg(n->value);

    // посещение вершины
    out += QString("(%1) ").arg(n->value);

    // правое поддерево
    if(n->right) inorderDetailed(n->right, out, n);
    else out += QString("0 %1 ").arg(n->value);

    // возврат к родителю
    if(parent) out += QString("%1 ").arg(parent->value);
}


void BSTWidget::postorderDetailed(Node* n, QString &out, Node* parent) {
    if(!n) {
        if(parent) out += QString("0 %1 ").arg(parent->value);
        return;
    }

    // левое поддерево
    if(n->left) postorderDetailed(n->left, out, n);
    else out += QString("0 %1 ").arg(n->value);

    // правое поддерево
    if(n->right) postorderDetailed(n->right, out, n);
    else out += QString("0 %1 ").arg(n->value);

    // посещение вершины
    out += QString("(%1) ").arg(n->value);

    // возврат к родителю
    if(parent) out += QString("%1 ").arg(parent->value);
}


void BSTWidget::showTraversals() {
    QString out;
    out += "Preorder: ";
    preorderDetailed(root, out);
    out += "\nInorder: ";
    inorderDetailed(root, out);
    out += "\nPostorder: ";
    postorderDetailed(root, out);
    emit logUpdated(out);
}

// ===== Построение правой прошивки (по in-order) =====
// Устанавливаем rightThread только у узлов, у которых нет правого сына.
// Циклически: последний указывает на первый.
void BSTWidget::buildRightThread(Node* r) {
    QVector<Node*> nodes;
    std::function<void(Node*)> collect = [&](Node* n){
        if (!n) return;
        collect(n->left);
        nodes.append(n);
        collect(n->right);
    };
    collect(r);
    int n = nodes.size();
    for (int i=0;i<n;i++) {
        nodes[i]->rightThread = nullptr;
    }
    for (int i=0;i<n;i++) {
        Node* node = nodes[i];
        if (!node->right) {
            // next cyclic
            if (n>0) node->rightThread = nodes[(i+1)%n];
        }
    }
}

// Вывод прошивки (для лога)
void BSTWidget::printRightThread(Node* r, QString& out) {
    if (!r) return;
    std::function<void(Node*)> walk = [&](Node* n){
        if (!n) return;
        walk(n->left);
        if (n->rightThread)
            out += QString("%1 -> %2\n").arg(n->value).arg(n->rightThread->value);
        //else
        //    out += QString("%1 -> nullptr\n").arg(n->value);
        walk(n->right);
    };
    walk(r);
}

// ===== Рисование дерева и построение posMap =====
void BSTWidget::drawTree(Node* node, int x, int y, int dx) {
    if (!node) return;
    int r = 22;
    // рисуем круг и текст
    scene->addEllipse(x-r, y-r, 2*r, 2*r, QPen(Qt::black), QBrush(Qt::yellow));
    QGraphicsTextItem *txt = scene->addText(QString::number(node->value));
    txt->setPos(x-8, y-12);
    // сохраняем позицию центра узла
    posMap[node] = QPointF(x, y);

    if (node->left) {
        scene->addLine(x, y+r, x-dx, y+80-r, QPen(Qt::black, 2));
        drawTree(node->left, x-dx, y+80, dx/1.5);
    } else {
        // optionally show small marker for empty left (not necessary visually)
    }
    if (node->right) {
        scene->addLine(x, y+r, x+dx, y+80-r, QPen(Qt::black, 2));
        drawTree(node->right, x+dx, y+80, dx/1.5);
    }
}

void BSTWidget::drawRightThreadOverlay(Node* node) {
    if (!node) return;
    // если есть нить — рисуем линию к целевому узлу и небольшой кружок с цифрой
    if (node->rightThread) {
        QPointF from = posMap.value(node, QPointF());
        QPointF to = posMap.value(node->rightThread, QPointF());
        if (from.isNull() || to.isNull()) {
            // если позиций нет (не нарисовано) — пропускаем
        } else {
            // линия (пунктир) от нижней точки до верхней точки целевого круга
            QPen pen(Qt::red);
            pen.setStyle(Qt::DashLine);
            pen.setWidth(2);
            scene->addLine(from.x(), from.y()+22, to.x(), to.y()-22, pen);

            // кружок на целевом узле (немного смещаем вправо и вниз чтобы не перекрывать основной)
            int rr = 14;
            scene->addEllipse(to.x()-rr, to.y()-rr, 2*rr, 2*rr, QPen(Qt::red), QBrush(Qt::white));

            // цифра
            QGraphicsTextItem *txt = scene->addText(QString::number(node->rightThread->value));
            txt->setDefaultTextColor(Qt::red);
            txt->setPos(to.x()-6, to.y()-10);
        }
    }
    if (node->left) drawRightThreadOverlay(node->left);
    if (node->right) drawRightThreadOverlay(node->right);
}

void BSTWidget::redrawTree() {
    scene->clear();
    posMap.clear();
    drawTree(root, 500, 50, 200);
    // если видим правые нити, нарисуем overlay
    if (rightThreadVisible) {
        drawRightThreadOverlay(root);
    }
    // подогнать вид
    QRectF r = scene->itemsBoundingRect();
    if (!r.isNull()) fitInView(r, Qt::KeepAspectRatio);
}

// Запуск показа правых нитей (строим нити и показываем)
void BSTWidget::showRightThreadOverlay() {
    if (!root) return;
    buildRightThread(root);
    rightThreadVisible = true;
    redrawTree();

    QString out;
    printRightThread(root, out);
    emit logUpdated(QString("Правая прошивка (inorder):\n") + out);

    // авто-скрытие через 6 секунд (опционально). если не нужно — закомментируй.
    //overlayTimer.start(6000);
}
