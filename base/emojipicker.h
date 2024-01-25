#ifndef EMOJIPICKER_H
#define EMOJIPICKER_H

#include <QWidget>

class EmojiPicker : public QWidget
{
    Q_OBJECT
public:
    explicit EmojiPicker(QWidget *parent = nullptr);

    QList<QIcon> getEmojiIcons();
    //获取所有表情路径
    QStringList getEmojiPaths();
signals:
    void emojiSelected(const QString &imagePath);

};

#endif // EMOJIPICKER_H
