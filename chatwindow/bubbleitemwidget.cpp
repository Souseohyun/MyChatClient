// BubbleItemWidget.cpp
#include "BubbleItemWidget.h"


BubbleItemWidget::BubbleItemWidget(QWidget *parent) : QWidget(parent) {
    // 初始化布局和控件
    QVBoxLayout* vLayout = new QVBoxLayout(this);
    // 初始化水平布局
    QHBoxLayout* hLayout = new QHBoxLayout();


    avatarLabel = new QLabel(this);
    messageLabel = new QLabel(this);
    //timestampLabel = new QLabel(this);



    avatarLabel->setFixedSize(30, 30); // 设置头像大小

    // 设置消息文本自动换行
    messageLabel->setWordWrap(true);



    // 将头像和消息标签添加到水平布局
    hLayout->setAlignment(Qt::AlignLeft | Qt::AlignTop);

    hLayout->addWidget(avatarLabel);
    hLayout->addSpacing(5); // 添加头像和文本标签之间的间隔
    hLayout->addWidget(messageLabel);
    // 将水平布局添加到垂直布局
    vLayout->addLayout(hLayout);

    // 将时间戳标签添加到垂直布局的末尾
    //vLayout->addWidget(timestampLabel);

    // 设置整体布局
    setLayout(vLayout);
}

//不仅设置了文本信息，还设置了文本宽度不能过chatwindow的2/3
void BubbleItemWidget::setInfo(const BubbleInfo &info) {
    bubbleInfo = info;

    // parent是QListWidget也就是chatwindow
    qDebug()<<parentWidget()->width();
    int maxTextWidth = static_cast<int>(parentWidget()->width() *0.6);
    messageLabel->setMaximumWidth(maxTextWidth);
    messageLabel->setText(info.message);
    messageLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    //timestampLabel->setText(info.timestamp.toString("HH:mm"));
    // 设置头像
    if (!info.avatar.isNull()) {
        QPixmap rounded;
        QPainterPath path;
        path.addEllipse(0, 0, avatarLabel->width(), avatarLabel->height());
        rounded = info.avatar.scaled(avatarLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
        QBitmap mask(rounded.size());
        QPainter painter(&mask);
        painter.fillRect(rounded.rect(), Qt::white);
        painter.setBrush(Qt::black);
        painter.drawPath(path);
        rounded.setMask(mask);
        avatarLabel->setPixmap(rounded);
    }


    // 强制更新布局以确保大小被正确计算
    messageLabel->adjustSize();
    //timestampLabel->adjustSize();
    avatarLabel->adjustSize();
    // 更新气泡尺寸
    updateGeometry();
}

QSize BubbleItemWidget::sizeHint() const {
    // 使用QFontMetrics计算文本所需的宽度和高度
    QFontMetrics metrics(messageLabel->font());
    int textWidth = metrics.horizontalAdvance(bubbleInfo.message); // Qt 5.11及以上版本
    // int textWidth = metrics.width(bubbleInfo.message); // Qt 5.10及以下版本

    // 计算并返回气泡所需的理想大小
    int width = std::min(textWidth, this->width() * 2 / 3) + 20; // 气泡宽度加上一些边距
    int height = messageLabel->height() + /*timestampLabel->height()*/ + 20; // 所有子部件的高度加上一些边距
    return QSize(width, height);
}



void BubbleItemWidget::paintEvent(QPaintEvent *event) {
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // 根据sizeHint来绘制气泡
    QRect rect(avatarLabel->width() + 10, 0, sizeHint().width() - avatarLabel->width() - 20, sizeHint().height()-5);
    //painter.setBrush(bubbleInfo.isSender ? Qt::white : Qt::green);
    if(bubbleInfo.isSender){
        //此处随后再想搭配
        painter.setBrush(Qt::white); // 气泡背景为白色
        painter.setPen(Qt::lightGray); // 气泡边框为浅灰色
    }else{
        painter.setBrush(Qt::white); // 气泡背景为白色
        painter.setPen(Qt::lightGray); // 气泡边框为浅灰色
    }

    painter.drawRoundedRect(rect, 15, 15);
}
