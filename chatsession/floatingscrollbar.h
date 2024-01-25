#ifndef FLOATINGSCROLLBAR_H
#define FLOATINGSCROLLBAR_H

#include <QScrollBar>
#include <QFile>

class FloatingScrollBar : public QScrollBar
{
    Q_OBJECT

public:
    explicit FloatingScrollBar(QWidget *parent,Qt::Orientation t);
    ~FloatingScrollBar(){}

public slots:
    void slt_valueChanged(int);
    void slt_rangeChanged(int,int);
};

#endif // FLOATINGSCROLLBAR_H
