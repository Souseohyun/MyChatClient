#ifndef LISTWIDGET_H
#define LISTWIDGET_H

#include <QListWidget>
#include <QScrollBar>
#include <QDateTime>
#include "clientapi/status.h"


#include "cell.h"
#include "floatingscrollbar.h"
#include "cellviewdad.h"
#include "cellviewson.h"


class ListWidget : public QListWidget
{
    Q_OBJECT

public:
    explicit ListWidget(QWidget* parent = nullptr,int tag = 0);


    QList<Cell*> GetAllCells();
    Cell *GetRightClickedCell();//获取右击选中的格子
    //插入格子
    void insertCell(Cell *cell);
    //移除格子
    void RemoveCell(Cell *cell);
    //移除所有格子
    void RemoveAllCells();


    /*未知区域，son，dad等*/
    void setDadPopMenu(QMenu *menu);
    void setSonPopMenu(QMenu *menu);


    //刷新
    void refreshList();
    void UpdateThisCellDisplay(Cell* cellToUpdate);

    //增加sonitem
    void addSonItem(Cell *cell);

    Cell *GetCellById(const int id);
    Cell* GetDadCellFromName(QString &name);

    void MakeCellToTop_NoTopImage(Cell*);
    void SetCellToTop(Cell*);
    void CancelCellOnTop(Cell*);

    QList<Cell*> GetAllCells() const;
    void ResetCellState();
    void RefreshCellTime(int id,qint64 time,QString msg);



    void changeSonSelectionState(Cell *c);
signals:
    void popMenuToShow(Cell*,QMenu *);
    void sonDoubleClicked(Cell *son);
    void signalSonRightClicked();

    void signalOpenDialog(Cell* cell);



    void signalSonSelected(int id,QString name);
public slots:
    void onDadOpenChanged(CellViewDad *dad);
    void onSonSelected(Cell *son);
    void onCellRightClicked(Cell *cell);
    void slt_valueChanged(int);




protected:
    void enterEvent(QEnterEvent *event);
    void leaveEvent(QEvent *);
    void resizeEvent(QResizeEvent*);



private:
    //tag == 0表示聊天列表
    //tag == 1表示群成员列表
    //tag == 2表示联系人列表
    int           tag = 0;

    //浮动滚动条
    FloatingScrollBar * scrollBar;
    //包含列表中所有的格子
public:
    QList<Cell *> cells;
    //右键选中的格子
    Cell *rightClickedCell;


    /*位置区域，son，dad将来必须改*/
    QList<CellViewSon*> sonItems;
    QMenu *cellDadMenu;
    QMenu *cellSonMenu;
};

#endif // LISTWIDGET_H
