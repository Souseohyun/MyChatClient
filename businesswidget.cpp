#include "businesswidget.h"
#include "ui_businesswidget.h"

BusinessWidget::BusinessWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::BusinessWidget)
{
    ui->setupUi(this);
}

BusinessWidget::~BusinessWidget()
{
    delete ui;
}
