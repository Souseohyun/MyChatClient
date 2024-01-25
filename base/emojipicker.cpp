#include "emojipicker.h"

#include <QGridLayout>
#include <QPushButton>

EmojiPicker::EmojiPicker(QWidget *parent)
    : QWidget{parent}
{
    const int numPerRow = 8; // 假设您想要每行显示8个表情
    const int emojiSize = 32; // 表情的尺寸
    const int padding = 5; // 表情之间的间距

    QGridLayout *layout = new QGridLayout(this);
    layout->setContentsMargins(padding, padding, padding, padding);
    layout->setSpacing(padding); // 设置表情之间的间距

    // 假设您有一个获取表情图标列表的函数: getEmojiIcons()
    //QList<QIcon> emojis = getEmojiIcons();
    QStringList emojiPaths = getEmojiPaths();

    for (int i = 0; i < emojiPaths.size(); ++i) {
        QPushButton *emojiButton = new QPushButton;
        QIcon emojiIcon(emojiPaths[i]);
        emojiButton->setIcon(emojiIcon);
        emojiButton->setIconSize(QSize(emojiSize, emojiSize));
        emojiButton->setFlat(true);
        emojiButton->setCursor(Qt::PointingHandCursor); // 设置手型光标
        layout->addWidget(emojiButton, i / numPerRow, i % numPerRow);

        // 连接信号到槽函数，传递路径
        connect(emojiButton, &QPushButton::clicked, [this, path = emojiPaths[i]]() {
            emit emojiSelected(path);
        });
    }
}

QList<QIcon> EmojiPicker::getEmojiIcons() {
    QList<QIcon> icons;
    // 添加表情图标到列表
    icons << QIcon(":/emoji/smile.png") << QIcon(":/emoji/kl.png")
          << QIcon(":/emoji/fn.png") << QIcon(":/emoji/doge.png")
          << QIcon(":/emoji/am.png") << QIcon(":/emoji/ct.png")
          << QIcon(":/emoji/cy.png") << QIcon(":/emoji/dy.png")
          << QIcon(":/emoji/jk.png") << QIcon(":/emoji/jx.png")
          << QIcon(":/emoji/kun.png") << QIcon(":/emoji/pz.png")
          << QIcon(":/emoji/qq.png") << QIcon(":/emoji/qz.png")
          << QIcon(":/emoji/se.png") << QIcon(":/emoji/ts.png")
          << QIcon(":/emoji/tx.png") << QIcon(":/emoji/wq.png")
          << QIcon(":/emoji/wzm.png") << QIcon(":/emoji/xk.png")
          << QIcon(":/emoji/yw.png") << QIcon(":/emoji/zj.png");
    // icons << QIcon(":/path/to/emoji1.png") << QIcon(":/path/to/emoji2.png") ...;
    return icons;
}

QStringList EmojiPicker::getEmojiPaths()
{
    QStringList paths;
    // 添加表情图标的路径到列表
    paths << ":/emoji/smile.png" << ":/emoji/kl.png"<<":/emoji/fn.png"
          <<":/emoji/doge.png"<<":/emoji/am.png"<<":/emoji/ct.png"
          <<":/emoji/cy.png"<<":/emoji/dy.png"<<":/emoji/jk.png"
          <<":/emoji/jx.png"<<":/emoji/kun.png"<<":/emoji/pz.png"
          <<":/emoji/qq.png"<<":/emoji/qz.png"<<":/emoji/se.png"
          <<":/emoji/ts.png"<<":/emoji/tx.png"<<":/emoji/wq.png"
          <<":/emoji/wzm.png"<<":/emoji/xk.png"<<":/emoji/yw.png"
          <<":/emoji/zj.png";

    return paths;
}
