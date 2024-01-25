#ifndef CELLVIEWDAD_H
#define CELLVIEWDAD_H

#include "cell.h"
#include <QWidget>
#include <QLabel>
#include <QMenu>


/*
 * CellViewDad 用来在列表视图中表示一个“分组标题”或“父项”
 * 例如：“我的好友”这就是一个viewdad，点击后会展开；
*/
class CellViewDad : public QWidget
{
    Q_OBJECT

public:
    explicit CellViewDad(QWidget *parent = nullptr);
    void setCell(Cell *c);
    void setPopMenu(QMenu *menu);

signals:
    void onOpenStatusChanged(CellViewDad *);//打开状态发生了改变
    void onPopMenuToShow(Cell*,QMenu*);

protected:
    void mousePressEvent(QMouseEvent *e);

public:
    Cell *cell;             //里面封装了显示CellView所需要的数据
    QLabel *iconLabel;      //显示图片
    QLabel *titleLabel;     //显示标题
    QLabel *subTitleLabel;  //显示副标题
    QMenu *popMenu;
};

#endif // CELLVIEWDAD_H
