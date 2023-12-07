#ifndef SHOWTEXT_H
#define SHOWTEXT_H
#include <QTextEdit>

class ShowText :public QTextEdit
{
    Q_OBJECT
public:
    ShowText();
    using QTextEdit::QTextEdit;

protected:

};

#endif // SHOWTEXT_H
