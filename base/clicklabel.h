#ifndef CLICKLABEL_H
#define CLICKLABEL_H

#include <QLabel>
#include <QWidget>


class ClickLabel : public QLabel
{
    Q_OBJECT
public:
    explicit ClickLabel(QWidget *parent = nullptr);

protected:
    virtual void mousePressEvent(QMouseEvent *) override;
    virtual void enterEvent(QEvent *);
    virtual void leaveEvent(QEvent *) override;

signals:
    void clickSignal();

public slots:

};


#endif // CLICKLABEL_H
