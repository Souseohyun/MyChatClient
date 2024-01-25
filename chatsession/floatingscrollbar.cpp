#include "floatingscrollbar.h"

FloatingScrollBar::FloatingScrollBar(QWidget *parent, Qt::Orientation t)
    :QScrollBar (parent)
{
    this->setOrientation(t);

    QString style_file = ":/qss/scrollbar.qss";
    QFile styleFile(style_file);//路径名
    if(styleFile.open( QFile::ReadOnly )){
        QString style( styleFile.readAll() );
        this->setStyleSheet(style);
    }

    this->setRange(0,0);
    this->hide();
}

void FloatingScrollBar::slt_valueChanged(int value)
{
    this->setValue(value);
}

void FloatingScrollBar::slt_rangeChanged(int min,int max)
{
    this->setMinimum(min);
    this->setRange(0,max);
    this->setPageStep(int(0.75*(this->height())) + max);
    if(max <= 0)
        this->hide();
}
