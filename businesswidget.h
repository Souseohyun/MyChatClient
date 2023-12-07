#ifndef BUSINESSWIDGET_H
#define BUSINESSWIDGET_H

#include <QWidget>

namespace Ui {
class BusinessWidget;
}

class BusinessWidget : public QWidget
{
    Q_OBJECT

public:
    explicit BusinessWidget(QWidget *parent = nullptr);
    ~BusinessWidget();

private:
    Ui::BusinessWidget *ui;
};

#endif // BUSINESSWIDGET_H
