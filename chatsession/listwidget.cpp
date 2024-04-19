#include "listwidget.h"

ListWidget::ListWidget(QWidget* parent,int tag)
:QListWidget(parent),tag(tag){

    this->setVerticalScrollMode(QListWidget::ScrollPerPixel);
    //    this->verticalScrollBar()->setSingleStep(10);
    this->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    this->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    //tag == 0表示聊天列表
    //tag == 1表示群成员列表
    //tag == 2表示联系人列表
    if(tag == 0 || tag == 2) this->setStyleSheet("ListWidget{background:#ebeae8;border:none;}");//对话列表的颜色
    else if(tag == 1) this->setStyleSheet("ListWidget{background:#f0f0f0;border:none;}");//群成员列表的颜色,f0f0f0

    //侧垂直滑条
    scrollBar = new FloatingScrollBar(this,Qt::Vertical);

    connect(this->verticalScrollBar(),SIGNAL(valueChanged(int)),
            scrollBar,SLOT(slt_valueChanged(int)));
    connect(scrollBar,SIGNAL(valueChanged(int)),this,SLOT(slt_valueChanged(int)));
    connect(this->verticalScrollBar(),SIGNAL(rangeChanged(int,int)),
            scrollBar,SLOT(slt_rangeChanged(int,int)));

}

QList<Cell *> ListWidget::GetAllCells()
{
    return this->cells;
}

//获取右击选中的格子
Cell *ListWidget::GetRightClickedCell()
{
    return rightClickedCell;
}

void ListWidget::insertCell(Cell *cell)
{
    qDebug()<<"I AM INTO insertCell";
    if(cell->type == Cell_GroupDrawer || cell->type == Cell_FriendDrawer){
        cells.append(cell);
    }else if(cell->type == Cell_FriendContact || cell->type == Cell_GroupContact){
        if(tag == 2){
            qDebug()<<"I AM INTO insertCell tag == 2:";
            foreach(Cell *group,cells){
                qDebug()<<"I GOT THE SUBGROUP";
                if(!group->groupName.compare(cell->groupName)){
                    group->childs.append(cell);
                    break;
                }
            }
        }else if(tag == 1){
            cells.append(cell);
        }

    }else if(cell->type == Cell_FriendChat || cell->type == Cell_GroupChat){
        cells.append(cell);
    }else if(cell->type == Cell_AddFriend || cell->type == Cell_AddGroup){
        cells.append(cell);
    }

    //刷新列表
    refreshList();
}

void ListWidget::RemoveCell(Cell *cell)
{
    if(cell->type == Cell_GroupDrawer || cell->type == Cell_FriendDrawer){
        cells.removeOne(cell);
    }else if(cell->type == Cell_FriendChat || cell->type == Cell_GroupChat){
        cells.removeOne(cell);
    }else if(cell->type == Cell_AddFriend || cell->type == Cell_AddGroup){
        cells.removeOne(cell);
    }else if(cell->type == Cell_FriendContact || cell->type == Cell_GroupContact){
        if(tag == 2){
            foreach(Cell *group,cells){
                if(!group->groupName.compare(cell->groupName)){
                    group->childs.removeOne(cell);
                    break;
                }
            }
        }else if(tag == 1){
            cells.removeOne(cell);
        }
    }
    refreshList();
}

//仅聊天列表
void ListWidget::RemoveAllCells()
{
    if(tag == 0){
        cells.clear();
        refreshList();
    }
}








void ListWidget::setDadPopMenu(QMenu *menu)
{
    cellDadMenu = menu;
}

void ListWidget::setSonPopMenu(QMenu *menu)
{
    qDebug()<<"into List SetSonPopMenu";
    cellSonMenu = menu;
}




void ListWidget::refreshList()
{
    this->clear();//首先移除所有格子
    sonItems.clear();
    for(Cell *cell : cells){
        if(cell->type == Cell_GroupDrawer || cell->type == Cell_FriendDrawer) {
            qDebug() << "Processing Drawer - Name:" << cell->groupName << "Is Open:" << cell->isOpen;
            CellViewDad *group = new CellViewDad();
            group->setGeometry(0,0,310,30);
            if(cell->type == Cell_FriendDrawer){
                int onLineCnt = 0;
                //统计在线好友数
                for(Cell *child : cell->childs){
                    if(child->status == Status::OnLine){
                        onLineCnt++;
                    }
                }
                group->titleLabel->setText(cell->groupName);
                group->subTitleLabel->setText(QString("[%1/%2]").arg(onLineCnt).arg(cell->childs.size()));
            }else{
                group->titleLabel->setText(QString("%1[%2/%2]").arg(cell->groupName).arg(cell->childs.size()));
                group->subTitleLabel->setText("");
            }

            group->setCell(cell);//cell中封装的是数据，CellView负责显示，此处是把数据传递给界面显示
            group->setPopMenu(cellDadMenu);

            connect(group, &CellViewDad::onOpenStatusChanged,
                    this, &ListWidget::onDadOpenChanged);
            //链接俩信号-->信号转发，不处理on信号，包装成pop信号转发走
            connect(group, &CellViewDad::onPopMenuToShow,
                    this, &ListWidget::popMenuToShow);

            QListWidgetItem *item = new QListWidgetItem("");
            item->setBackground(QBrush(QColor(235, 234, 232)));
            this->addItem(item);
            this->setItemWidget(item,group);
            item->setSizeHint(group->geometry().size());

            //如果抽屉被打开的话则显示下面的格子
            if(cell->isOpen){
                qDebug()<<"__DEBUG FOR CELL->CHILDS COUNT : "<<cell->childs.count();
                //先加载在线好友
                for(Cell *c : cell->childs){
                    if(c->status == Status::OnLine)
                    {
                        qDebug()<<"ONLINE";
                        addSonItem(c);
                    }
                }

                //再加载离线好友
                for(Cell *c : cell->childs){
                    if(c->status == Status::OffLine){
                        qDebug()<<"OFFLINE";
                        addSonItem(c);
                    }

                }
            }
        }
        else{
            addSonItem(cell);
        }
    }
}

void ListWidget::UpdateThisCellDisplay(Cell *cellToUpdate)
{
    // 遍历 QListWidget 的所有项
    for (int i = 0; i < this->count(); ++i) {
        QListWidgetItem* item = this->item(i);
        QWidget* widget = this->itemWidget(item);
        CellViewSon* cellView = dynamic_cast<CellViewSon*>(widget);

        if (cellView != nullptr && cellView->cell == cellToUpdate) {
            // 找到对应的 CellViewSon，更新显示内容
            cellView->updateDisplay();
            break; // 找到后退出循环
        }
    }
}

//虽然接受的是cell，但本质是添加一个cellviewson
void ListWidget::addSonItem(Cell *cell)
{
    //此处为手操代码，可能引发冲突，随时待删 确保 Cell 对象被添加到 cells 成员中
    /*
    if (!cells.contains(cell)) {
        cells.append(cell);
    }
*/
    CellViewSon *son = new CellViewSon(nullptr,cell,tag);
    //cellviewson类型（tag）不同，她们ui的尺寸也不同
    if(tag == 0 || tag == 2)
        son->setGeometry(0,0,350,60);
    else if(tag == 1)
        son->setGeometry(0,0,200,40);
    son->setPopMenu(cellSonMenu);//设置了一个右键菜单
    sonItems.append(son);
//cells.append(cell);
    //槽连接，消息传递给上层类进行具体处理


    connect(son, &CellViewSon::onSelected,
            this, &ListWidget::onSonSelected); // 单元格被单击选中
    connect(son, &CellViewSon::onRightClicked,
            this, &ListWidget::onCellRightClicked); // 单元格被右击
    connect(son, &CellViewSon::onPopMenuToShow,
            this, &ListWidget::popMenuToShow); // 发出右键菜单请求
    connect(son, &CellViewSon::onDoubleClicked,
            this, &ListWidget::sonDoubleClicked); // 单元格被双击

    QListWidgetItem *item = new QListWidgetItem("");
    this->addItem(item);
    this->setItemWidget(item,son);
    item->setSizeHint(son->geometry().size());

    qDebug() << "ListWidget::addSonItem - Adding Cell:" << cell->name << "Status:" << (int)cell->status;


}

Cell *ListWidget::GetCellById(const int id)
{
    for (Cell *cell : cells) {
        if (cell->id == id) {
            return cell;
        }
    }
    return nullptr;
}

Cell *ListWidget::GetDadCellFromName(QString &name)
{
    for(Cell *group : cells) {
        if(group->groupName.compare(name) == 0) {
            return group;
        }
    }
    return nullptr;
}

//消息到来被动置顶
void ListWidget::MakeCellToTop_NoTopImage(Cell *cell)
{
    cells.removeOne(cell);
    cells.push_front(cell);
    refreshList();
}

//手动置顶
void ListWidget::SetCellToTop(Cell *cell)
{
    cell->stayOnTop = true;
    cells.removeOne(cell);
    cells.push_front(cell);
    refreshList();
}

void ListWidget::CancelCellOnTop(Cell *cell)
{
    cell->stayOnTop = false;
    cells.removeOne(cell);
    cells.push_back(cell);
    refreshList();
}


/*--------------------------------------------------------*/

QList<Cell*> ListWidget::GetAllCells() const
{
    return cells;
}

void ListWidget::ResetCellState()
{
    int cnt = cells.size();
    for(int i = 0;i < cnt;i++){
        cells.at(i)->isClicked = false;
    }
}

/*
    此时Refresh的cells.at(i)->id永远会*/
void ListWidget::RefreshCellTime(int id, qint64 time,QString msg)
{
    qDebug()<<"into Refresh Cell     "<<"id: "<<id<<"; msg: "<<msg;
    int cnt = cells.size();
    for(int i = 0;i < cnt;i++){
        if(cells.at(i)->id == id && (cells.at(i)->type == Cell_FriendChat
                                      || cells.at(i)->type == Cell_GroupChat)){
            cells.at(i)->subTitle = QDateTime::fromSecsSinceEpoch(time).toString("hh:mm:ss");

            msg.replace("\n"," ");

            cells.at(i)->msg = msg;
            //收到或发出新消息使其暂时回到顶层
            MakeCellToTop_NoTopImage(cells.at(i));

            if(!cells.at(i)->isClicked)
                cells.at(i)->showNewMsg = true;
            refreshList();
            return;
        }
    }
}


void ListWidget::changeSonSelectionState(Cell *c)
{
    for(CellViewSon* son: sonItems){
        if(son->cell != c){
            son->cell->isClicked = false;
            son->update();
        }
    }
}


/*slots*/

void ListWidget::onDadOpenChanged(CellViewDad* dad)
{
    qDebug() << "ListWidget::onDadOpenChanged - Cell Open Status:" << dad->cell->isOpen;

    int cnt = dad->cell->childs.size();
    for(int i = 0;i < cnt;i++){
        //源代码是=false;
        dad->cell->childs.at(i)->isClicked = false;//每一个子项的isClicked为false
    }
    refreshList();
}

//左键选中listwidget中的某个sonitem后,cell为选中soncell信息
void ListWidget::onSonSelected(Cell *cell)
{
    //qDebug()<<"into onSonSelected............";
    //qDebug()<<cell->name<<" "<<cell->iconPath;
    cell->isClicked = true;//选中该项
    changeSonSelectionState(cell);

    if(cell->type == Cell_AddFriend || cell->type == Cell_AddGroup)
        emit signalOpenDialog(cell);
    else if(cell->type == Cell_FriendChat || cell->type == Cell_GroupChat)
    {
        emit sonDoubleClicked(cell);
        //需要在这通告chatwindow，改成合适的聊天界面
        //qDebug()<<"into Cell_FriendChat";
        emit signalSonSelected(cell->id,cell->name);
    }
}

void ListWidget::onCellRightClicked(Cell *cell)
{
    rightClickedCell = cell;
    emit signalSonRightClicked();
}


void ListWidget::slt_valueChanged(int value)
{
    this->verticalScrollBar()->setValue(value);
}




/*protected functions*/

void ListWidget::enterEvent(QEnterEvent *event)
{
    if(scrollBar->maximum() > 0)
        scrollBar->show();
    return QListWidget::enterEvent(event);
}

void ListWidget::leaveEvent(QEvent *e)
{
    scrollBar->hide();
    return QListWidget::leaveEvent(e);
}

void ListWidget::resizeEvent(QResizeEvent *e)
{
    int x = this->width() - 8;
    scrollBar->setGeometry(x,1,8,this->height()-2);
    return QListWidget::resizeEvent(e);
}
