#include "qnchatmessage.h"
#include <QFontMetrics>
#include <QPaintEvent>
#include <QDateTime>
#include <QPainter>
#include <QMovie>
#include <QLabel>
#include <QDebug>

QNChatMessage::QNChatMessage(QWidget *parent)
: QWidget(parent)
{
    QFont te_font = this->font();
    te_font.setFamily("MicrosoftYaHei");
    te_font.setPointSize(12);
    // te_font.setWordSpacing(0);
    // te_font.setLetterSpacing(QFont::PercentageSpacing,0);
    // te_font.setLetterSpacing(QFont::PercentageSpacing, 100); //300%,100为默认 //设置字间距%
    // te_font.setLetterSpacing(QFont::AbsoluteSpacing, 0); //设置字间距为3像素 //设置字间距像素值
    this->setFont(te_font);
    //m_leftPixmap = QPixmap(":/Sulli.jpg");
    //m_rightPixmap = QPixmap(":/ChuxianC.jpg");

    m_loadingMovie = new QMovie(this);
    m_loadingMovie->setFileName(":/img/loading4.gif");
    m_loading = new QLabel(this);
    m_loading->setMovie(m_loadingMovie);
    m_loading->resize(16,16);
    m_loading->setAttribute(Qt::WA_TranslucentBackground , true);
    m_loading->setAutoFillBackground(false);
}

QNChatMessage::QNChatMessage(const QPixmap &rightPixmap, QWidget *parent)
:QWidget(parent), m_rightPixmap(rightPixmap)
{
    QFont te_font = this->font();
    te_font.setFamily("MicrosoftYaHei");
    te_font.setPointSize(12);

    this->setFont(te_font);
    m_leftPixmap = QPixmap(":/Sulli.jpg");
    //m_rightPixmap = QPixmap(":/ChuxianC.jpg");

    m_loadingMovie = new QMovie(this);
    m_loadingMovie->setFileName(":/img/loading4.gif");
    m_loading = new QLabel(this);
    m_loading->setMovie(m_loadingMovie);
    m_loading->resize(16,16);
    m_loading->setAttribute(Qt::WA_TranslucentBackground , true);
    m_loading->setAutoFillBackground(false);
}




void QNChatMessage::SetHTML(const QString &html, User_Type ut)
{
    m_htmlContent = html;
    m_userType    = ut;

    this->update();

}

QSize QNChatMessage::GetSize()
{
    int minHei = 30;
    int iconWH = 40;
    int iconSpaceW = 20;
    int iconRectW = 5;
    int iconTMPH = 10;
    int sanJiaoW = 6;
    int kuangTMP = 20;
    int textSpaceRect = 6;
    m_kuangWidth = this->width() - kuangTMP - 2*(iconWH+iconSpaceW+iconRectW);
    m_textWidth = m_kuangWidth - 2*textSpaceRect;
    m_spaceWid = this->width() - m_textWidth;
    m_iconLeftRect = QRect(iconSpaceW, iconTMPH, iconWH, iconWH);
    m_iconRightRect = QRect(this->width() - iconSpaceW - iconWH, iconTMPH, iconWH, iconWH);

    // 初始化 m_lineHeight
    QFontMetrics fm(this->font());
    m_lineHeight = fm.height();  // 正确获取行高

    QSize htmlSize = GetHTMLSize();

    qDebug()<<"____DEBUG: show html size : "<<htmlSize;

    int hei = htmlSize.height() < minHei ? minHei : htmlSize.height();

    m_sanjiaoLeftRect = QRect(iconWH+iconSpaceW+iconRectW, m_lineHeight/2, sanJiaoW, hei - m_lineHeight);
    m_sanjiaoRightRect = QRect(this->width() - iconRectW - iconWH - iconSpaceW - sanJiaoW, m_lineHeight/2, sanJiaoW, hei - m_lineHeight);

    //new add
    int rightPadding = iconWH + iconSpaceW + iconRectW; // 右侧头像和间距的总宽度
    int leftPadding = iconWH + iconSpaceW + iconRectW + sanJiaoW; // 左侧头像和间距的总宽度

    int textPadding = 2 * textSpaceRect; // 文本的左右内边距总和


    if(htmlSize.width() < (m_textWidth+m_spaceWid)) {
        qDebug()<<"into first if";
        m_kuangLeftRect.setRect(leftPadding,
                                m_lineHeight / 4 * 3,
                                htmlSize.width() + textPadding - 20,
                                hei);
        m_kuangRightRect.setRect(this->width() - rightPadding - htmlSize.width() - textPadding - sanJiaoW + 20, // X坐标
                                 m_lineHeight/4*3, // Y坐标
                                 htmlSize.width() + textPadding - 20, // 宽度 (20是一个人为的偏移量，属于我人工修整，不妥但先用了再说，改它记得该X坐标）
                                 hei ); // 高度
        qDebug()<<"this width:"<<this->width();
        qDebug()<<"html width:"<<htmlSize.width();
        qDebug()<<"m_lineHeight:"<<m_lineHeight;
        qDebug()<<"html height:"<<htmlSize.height();
        qDebug()<<"hei:"<<hei;
        qDebug()<<"\nm_spaceWid:"<<m_spaceWid;
        qDebug()<<"textSpaceRect:"<<textSpaceRect;

    } else {
        qDebug()<<"into second if";
        //m_kuangLeftRect.setRect(m_sanjiaoLeftRect.x()+m_sanjiaoLeftRect.width(), m_lineHeight/4*3, m_kuangWidth, hei-m_lineHeight);
        //m_kuangRightRect.setRect(iconWH + kuangTMP + iconSpaceW + iconRectW - sanJiaoW, m_lineHeight/4*3, m_kuangWidth, hei-m_lineHeight);

        m_kuangLeftRect.setRect(leftPadding,
                                m_lineHeight / 4 * 3,
                                htmlSize.width() + textPadding - 20,
                                hei);
        //m_kuangRightRect.setRect(this->width() - htmlSize.width() + m_spaceWid - 2*textSpaceRect - iconWH - iconSpaceW - iconRectW - sanJiaoW,
        //                         m_lineHeight/4*3,  htmlSize.width() + 2 * textSpaceRect/*htmlSize.width()-m_spaceWid+2*textSpaceRect*/, hei-m_lineHeight);
        m_kuangRightRect.setRect(this->width() - rightPadding - htmlSize.width() - textPadding - sanJiaoW + 20, // X坐标
                                 m_lineHeight/4*3, // Y坐标
                                 htmlSize.width() + textPadding - 20, // 宽度 (20是一个人为的偏移量，属于我人工修整，不妥但先用了再说，改它记得该X坐标）
                                 hei ); // 高度


    }
    m_textLeftRect.setRect(m_kuangLeftRect.x()+textSpaceRect,m_kuangLeftRect.y()+iconTMPH,
                           m_kuangLeftRect.width()-2*textSpaceRect,m_kuangLeftRect.height()-2*iconTMPH);
    m_textRightRect.setRect(m_kuangRightRect.x()+textSpaceRect,m_kuangRightRect.y()+iconTMPH,
                            m_kuangRightRect.width()-2*textSpaceRect,m_kuangRightRect.height()-2*iconTMPH);

    qDebug()<<"____DEBUG: show kuang size : "<<m_kuangLeftRect.size();
    return QSize(htmlSize.width(), hei);
}

QSize QNChatMessage::GetHTMLSize()
{
    QTextDocument doc;
    doc.setDefaultFont(this->font()); // 设置文档的默认字体
    doc.setHtml(m_htmlContent); // 设置 HTML 内容

    doc.setTextWidth(-1); // 设置无限制的宽度以获取内容的自然宽度

    // 获取文档的理想宽度和高度
    QSizeF size = doc.size(); // size() 现在返回的宽度是内容的自然宽度
    int idealWidth = qMin((int)size.width(), m_kuangWidth); // 宽度不超过最大宽度
    int actualHeight = (int)size.height();

    //如果超宽了，要修改其size的高度，因为之前的size是无限制宽度（set-1），永远只有一行的高度
    if(m_kuangWidth < (int)size.width()){
        doc.setTextWidth(m_kuangWidth);
        QSizeF size2 = doc.size();
        actualHeight = (int)size2.height();
    }
    qDebug()<<"ideal width:"<<idealWidth;
    qDebug()<<"doc width:"<<(int)size.width();
    qDebug()<<"m_kuangWidth:"<<m_kuangWidth;

    // 可能需要添加一些额外的边距
    int extraMargin = 10; // 例如，每边 10 像素的边距
    QSize htmlSize(idealWidth + 2 * extraMargin, actualHeight + 2 * extraMargin);

    return htmlSize;
}

void QNChatMessage::SetHeaderImage(User_Type userType, QPixmap& headerPic)
{
    if(userType == User_Me){
        m_rightPixmap = headerPic;
    }else if(userType == User_She){
        m_leftPixmap = headerPic;
    }else{
        qDebug()<<"SetHeaderImage Error";
    }
}



void QNChatMessage::setTextSuccess()
{

    m_loading->hide();
    m_loadingMovie->stop();
    m_isSending = true;
}

void QNChatMessage::setText(QString text, QString time, QSize allSize, QNChatMessage::User_Type userType)
{

    m_msg = text;
    m_userType = userType;
    m_time = time;
    m_curTime = QDateTime::fromSecsSinceEpoch(time.toLongLong()).toString("hh:mm");
    m_allSize = allSize;
    if(userType == User_Me) {

        if(!m_isSending) {

            m_loading->move(m_kuangRightRect.x() - m_loading->width() - 10, m_kuangRightRect.y()+m_kuangRightRect.height()/2- m_loading->height()/2);
            m_loading->show();
            m_loadingMovie->start();
        }
    } else {

        m_loading->hide();
    }

    this->update();
}



QSize QNChatMessage::fontRect(QString str)
{
    qDebug()<<"debug--------"<<this->width();
    m_msg = str;
    int minHei = 30;
    int iconWH = 40;
    int iconSpaceW = 20;
    int iconRectW = 5;
    int iconTMPH = 10;
    int sanJiaoW = 6;
    int kuangTMP = 20;
    int textSpaceRect = 12;
    m_kuangWidth = this->width() - kuangTMP - 2*(iconWH+iconSpaceW+iconRectW);
    m_textWidth = m_kuangWidth - 2*textSpaceRect;
    m_spaceWid = this->width() - m_textWidth;
    m_iconLeftRect = QRect(iconSpaceW, iconTMPH, iconWH, iconWH);
    m_iconRightRect = QRect(this->width() - iconSpaceW - iconWH, iconTMPH, iconWH, iconWH);

    QSize size = getRealString(m_msg); // 整个的size

    qDebug() << "fontRect Size:" << size;
    int hei = size.height() < minHei ? minHei : size.height();

    m_sanjiaoLeftRect = QRect(iconWH+iconSpaceW+iconRectW, m_lineHeight/2, sanJiaoW, hei - m_lineHeight);
    m_sanjiaoRightRect = QRect(this->width() - iconRectW - iconWH - iconSpaceW - sanJiaoW, m_lineHeight/2, sanJiaoW, hei - m_lineHeight);

    if(size.width() < (m_textWidth+m_spaceWid)) {

        m_kuangLeftRect.setRect(m_sanjiaoLeftRect.x()+m_sanjiaoLeftRect.width(), m_lineHeight/4*3, size.width()-m_spaceWid+2*textSpaceRect, hei-m_lineHeight);
        m_kuangRightRect.setRect(this->width() - size.width() + m_spaceWid - 2*textSpaceRect - iconWH - iconSpaceW - iconRectW - sanJiaoW,
                                 m_lineHeight/4*3, size.width()-m_spaceWid+2*textSpaceRect, hei-m_lineHeight);
    } else {

        m_kuangLeftRect.setRect(m_sanjiaoLeftRect.x()+m_sanjiaoLeftRect.width(), m_lineHeight/4*3, m_kuangWidth, hei-m_lineHeight);
        m_kuangRightRect.setRect(iconWH + kuangTMP + iconSpaceW + iconRectW - sanJiaoW, m_lineHeight/4*3, m_kuangWidth, hei-m_lineHeight);
    }
    m_textLeftRect.setRect(m_kuangLeftRect.x()+textSpaceRect,m_kuangLeftRect.y()+iconTMPH,
                           m_kuangLeftRect.width()-2*textSpaceRect,m_kuangLeftRect.height()-2*iconTMPH);
    m_textRightRect.setRect(m_kuangRightRect.x()+textSpaceRect,m_kuangRightRect.y()+iconTMPH,
                            m_kuangRightRect.width()-2*textSpaceRect,m_kuangRightRect.height()-2*iconTMPH);

    return QSize(size.width(), hei);
}

QSize QNChatMessage::getRealString(QString src)
{

    QFontMetricsF fm(this->font());
    m_lineHeight = fm.lineSpacing();
    int nCount = src.count("\n");
    int nMaxWidth = 0;
    if(nCount == 0) {

        nMaxWidth = fm.horizontalAdvance(src);
        QString value = src;
        if(nMaxWidth > m_textWidth) {

            nMaxWidth = m_textWidth;
            int size = m_textWidth / fm.horizontalAdvance(" ");
            int num = fm.horizontalAdvance(value) / m_textWidth;
            int ttmp = num*fm.horizontalAdvance(" ");
            num = ( fm.horizontalAdvance(value) ) / m_textWidth;
            nCount += num;
            QString temp = "";
            for(int i = 0; i < num; i++) {

                temp += value.mid(i*size, (i+1)*size) + "\n";
            }
            src.replace(value, temp);
        }
    } else {

        for(int i = 0; i < (nCount + 1); i++) {

            QString value = src.split("\n").at(i);
            nMaxWidth = fm.horizontalAdvance(value) > nMaxWidth ? fm.horizontalAdvance(value) : nMaxWidth;
            if(fm.horizontalAdvance(value) > m_textWidth) {

                nMaxWidth = m_textWidth;
                int size = m_textWidth / fm.horizontalAdvance(" ");
                int num = fm.horizontalAdvance(value) / m_textWidth;
                num = ((i+num)*fm.horizontalAdvance(" ") + fm.horizontalAdvance(value)) / m_textWidth;
                nCount += num;
                QString temp = "";
                for(int i = 0; i < num; i++) {

                    temp += value.mid(i*size, (i+1)*size) + "\n";
                }
                src.replace(value, temp);
            }
        }
    }
    return QSize(nMaxWidth+m_spaceWid, (nCount + 1) * m_lineHeight+2*m_lineHeight);
}


//显示html版本
void QNChatMessage::paintEvent(QPaintEvent *event)
{
    //qDebug() << "paintEvent called";

    Q_UNUSED(event);




    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);//消锯齿
    painter.setPen(Qt::NoPen);
    painter.setBrush(QBrush(Qt::gray));

    if(m_userType == User_Type::User_She) {
        // 用户
        //头像
        // painter.drawRoundedRect(m_iconLeftRect,m_iconLeftRect.width(),m_iconLeftRect.height());
        painter.drawPixmap(m_iconLeftRect, m_leftPixmap);
        //框
        QColor col_Kuang(245,245,245);
        painter.setBrush(QBrush(col_Kuang));
        painter.drawRoundedRect(m_kuangLeftRect,4,4);

        //三角
        QPointF points[3] = {
            QPointF(m_sanjiaoLeftRect.x(), 30),
            QPointF(m_sanjiaoLeftRect.x()+m_sanjiaoLeftRect.width(), 25),
            QPointF(m_sanjiaoLeftRect.x()+m_sanjiaoLeftRect.width(), 35),
        };
        QPen pen;
        pen.setColor(col_Kuang);
        painter.setPen(pen);
        painter.drawPolygon(points, 3);


        // 绘制 HTML 内容
        QTextDocument doc;
        doc.setDefaultFont(this->font());

        doc.setHtml(m_htmlContent);
        doc.setTextWidth(m_textLeftRect.width()); // 确保文本宽度适应气泡框

        // 调整文档的位置来适应绘制区域
        //painter.translate(m_textRightRect.topLeft());
        //doc.drawContents(&painter);
        painter.save(); // 保存当前画笔状态
        painter.translate(m_textLeftRect.topLeft()); // 平移到文本区域的左上角
        QRect clip(0, 0, m_textLeftRect.width(), m_textLeftRect.height());
        painter.setClipRect(clip); // 设置裁剪区域，防止文本溢出绘制区域
        doc.drawContents(&painter, clip); // 绘制 HTML 内容
        painter.restore(); // 恢复画笔状态

        QWidget::paintEvent(event); // 调用基类的 paintEvent

    }
    else if(m_userType == User_Type::User_Me) {
        // 自己
        //头像
        // painter.drawRoundedRect(m_iconRightRect,m_iconRightRect.width(),m_iconRightRect.height());
        painter.drawPixmap(m_iconRightRect, m_rightPixmap);

        //DEBUG,尝试手动加框的大小，先保证html正常显示
        //m_kuangRightRect.setHeight(m_kuangRightRect.height() + 250);
        //m_kuangRightRect.setWidth(m_kuangRightRect.width() + 250);
        //框
        QColor col_Kuang(35,153,254);
        painter.setBrush(QBrush(col_Kuang));
        painter.drawRoundedRect(m_kuangRightRect,4,4);

        //三角
        QPointF points[3] = {

            QPointF(m_sanjiaoRightRect.x()+m_sanjiaoRightRect.width(), 30),
            QPointF(m_sanjiaoRightRect.x(), 25),
            QPointF(m_sanjiaoRightRect.x(), 35),
        };
        QPen pen;
        pen.setColor(col_Kuang);
        painter.setPen(pen);
        painter.drawPolygon(points, 3);

        // 绘制 HTML 内容
        QTextDocument doc;
        doc.setDefaultFont(this->font());
        //qDebug()<<"_____DEBUG:INTO PAINTEVENT htmlContent: "<<m_htmlContent;
        doc.setHtml(m_htmlContent);
        doc.setTextWidth(m_textRightRect.width()); // 确保文本宽度适应气泡框

        // 调整文档的位置来适应绘制区域
        //painter.translate(m_textRightRect.topLeft());
        //doc.drawContents(&painter);
        painter.save(); // 保存当前画笔状态
        painter.translate(m_textRightRect.topLeft()); // 平移到文本区域的左上角
        QRect clip(0, 0, m_textRightRect.width(), m_textRightRect.height());
        painter.setClipRect(clip); // 设置裁剪区域，防止文本溢出绘制区域
        doc.drawContents(&painter, clip); // 绘制 HTML 内容
        painter.restore(); // 恢复画笔状态

        QWidget::paintEvent(event); // 调用基类的 paintEvent


    }  else if(m_userType == User_Type::User_Time) {
        // 时间
        QPen penText;
        penText.setColor(QColor(153,153,153));
        painter.setPen(penText);
        QTextOption option(Qt::AlignCenter);
        option.setWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere);
        QFont te_font = this->font();
        te_font.setFamily("MicrosoftYaHei");
        te_font.setPointSize(10);
        painter.setFont(te_font);
        painter.drawText(this->rect(),m_curTime,option);
    }
}
//这个函数用于显示单纯文本，无错版本：
/*
void QNChatMessage::paintEvent(QPaintEvent *event)
{
    qDebug() << "paintEvent called";

    Q_UNUSED(event);




    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);//消锯齿
    painter.setPen(Qt::NoPen);
    painter.setBrush(QBrush(Qt::gray));

    if(m_userType == User_Type::User_She) {
        // 用户
        //头像
        // painter.drawRoundedRect(m_iconLeftRect,m_iconLeftRect.width(),m_iconLeftRect.height());
        painter.drawPixmap(m_iconLeftRect, m_leftPixmap);

        //框加边
        QColor col_KuangB(234, 234, 234);
        painter.setBrush(QBrush(col_KuangB));
        painter.drawRoundedRect(m_kuangLeftRect.x()-1,m_kuangLeftRect.y()-1,m_kuangLeftRect.width()+2,m_kuangLeftRect.height()+2,4,4);
        //框
        QColor col_Kuang(255,255,255);
        painter.setBrush(QBrush(col_Kuang));
        painter.drawRoundedRect(m_kuangLeftRect,4,4);

        //三角
        QPointF points[3] = {

            QPointF(m_sanjiaoLeftRect.x(), 30),
            QPointF(m_sanjiaoLeftRect.x()+m_sanjiaoLeftRect.width(), 25),
            QPointF(m_sanjiaoLeftRect.x()+m_sanjiaoLeftRect.width(), 35),
        };
        QPen pen;
        pen.setColor(col_Kuang);
        painter.setPen(pen);
        painter.drawPolygon(points, 3);

        //三角加边
        QPen penSanJiaoBian;
        penSanJiaoBian.setColor(col_KuangB);
        painter.setPen(penSanJiaoBian);
        painter.drawLine(QPointF(m_sanjiaoLeftRect.x() - 1, 30), QPointF(m_sanjiaoLeftRect.x()+m_sanjiaoLeftRect.width(), 24));
        painter.drawLine(QPointF(m_sanjiaoLeftRect.x() - 1, 30), QPointF(m_sanjiaoLeftRect.x()+m_sanjiaoLeftRect.width(), 36));

        //内容
        QPen penText;
        penText.setColor(QColor(51,51,51));
        painter.setPen(penText);
        QTextOption option(Qt::AlignLeft | Qt::AlignVCenter);
        option.setWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere);
        painter.setFont(this->font());
        painter.drawText(m_textLeftRect, m_msg,option);
    }  else if(m_userType == User_Type::User_Me) {
        // 自己
        //头像
        // painter.drawRoundedRect(m_iconRightRect,m_iconRightRect.width(),m_iconRightRect.height());
        painter.drawPixmap(m_iconRightRect, m_rightPixmap);

        //框
        QColor col_Kuang(75,164,242);
        painter.setBrush(QBrush(col_Kuang));
        painter.drawRoundedRect(m_kuangRightRect,4,4);

        //三角
        QPointF points[3] = {

            QPointF(m_sanjiaoRightRect.x()+m_sanjiaoRightRect.width(), 30),
            QPointF(m_sanjiaoRightRect.x(), 25),
            QPointF(m_sanjiaoRightRect.x(), 35),
        };
        QPen pen;
        pen.setColor(col_Kuang);
        painter.setPen(pen);
        painter.drawPolygon(points, 3);

        //内容
        QPen penText;
        penText.setColor(Qt::white);
        painter.setPen(penText);
        QTextOption option(Qt::AlignLeft | Qt::AlignVCenter);
        option.setWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere);
        painter.setFont(this->font());
        painter.drawText(m_textRightRect,m_msg,option);
    }  else if(m_userType == User_Type::User_Time) {
        // 时间
        QPen penText;
        penText.setColor(QColor(153,153,153));
        painter.setPen(penText);
        QTextOption option(Qt::AlignCenter);
        option.setWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere);
        QFont te_font = this->font();
        te_font.setFamily("MicrosoftYaHei");
        te_font.setPointSize(10);
        painter.setFont(te_font);
        painter.drawText(this->rect(),m_curTime,option);
    }
}
*/


#if 0
void QNChatMessage::paintEvent(QPaintEvent *event)
{
    qDebug() << "paintEvent called";

    Q_UNUSED(event);




    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);//消锯齿
    painter.setPen(Qt::NoPen);
    painter.setBrush(QBrush(Qt::gray));

    if(m_userType == User_Type::User_She) {
        // 用户
        //头像
        // painter.drawRoundedRect(m_iconLeftRect,m_iconLeftRect.width(),m_iconLeftRect.height());
        painter.drawPixmap(m_iconLeftRect, m_leftPixmap);

        //框加边
        QColor col_KuangB(234, 234, 234);
        painter.setBrush(QBrush(col_KuangB));
        painter.drawRoundedRect(m_kuangLeftRect.x()-1,m_kuangLeftRect.y()-1,m_kuangLeftRect.width()+2,m_kuangLeftRect.height()+2,4,4);
        //框
        QColor col_Kuang(255,255,255);
        painter.setBrush(QBrush(col_Kuang));
        painter.drawRoundedRect(m_kuangLeftRect,4,4);

        //三角
        QPointF points[3] = {

            QPointF(m_sanjiaoLeftRect.x(), 30),
            QPointF(m_sanjiaoLeftRect.x()+m_sanjiaoLeftRect.width(), 25),
            QPointF(m_sanjiaoLeftRect.x()+m_sanjiaoLeftRect.width(), 35),
        };
        QPen pen;
        pen.setColor(col_Kuang);
        painter.setPen(pen);
        painter.drawPolygon(points, 3);

        //三角加边
        QPen penSanJiaoBian;
        penSanJiaoBian.setColor(col_KuangB);
        painter.setPen(penSanJiaoBian);
        painter.drawLine(QPointF(m_sanjiaoLeftRect.x() - 1, 30), QPointF(m_sanjiaoLeftRect.x()+m_sanjiaoLeftRect.width(), 24));
        painter.drawLine(QPointF(m_sanjiaoLeftRect.x() - 1, 30), QPointF(m_sanjiaoLeftRect.x()+m_sanjiaoLeftRect.width(), 36));

        // 绘制富文本内容
        QTextDocument doc;
        doc.setDefaultFont(painter.font());
        doc.setHtml(m_htmlContent);
        QRect textRect = m_userType == User_Type::User_Me ? m_textRightRect : m_textLeftRect;
        painter.save();
        painter.translate(textRect.topLeft());
        doc.setTextWidth(textRect.width());
        doc.drawContents(&painter);
        painter.restore();
    }  else if(m_userType == User_Type::User_Me) {
        // 自己
        //头像
        // painter.drawRoundedRect(m_iconRightRect,m_iconRightRect.width(),m_iconRightRect.height());
        painter.drawPixmap(m_iconRightRect, m_rightPixmap);

        //框
        QColor col_Kuang(75,164,242);
        painter.setBrush(QBrush(col_Kuang));
        painter.drawRoundedRect(m_kuangRightRect,4,4);

        //三角
        QPointF points[3] = {

            QPointF(m_sanjiaoRightRect.x()+m_sanjiaoRightRect.width(), 30),
            QPointF(m_sanjiaoRightRect.x(), 25),
            QPointF(m_sanjiaoRightRect.x(), 35),
        };
        QPen pen;
        pen.setColor(col_Kuang);
        painter.setPen(pen);
        painter.drawPolygon(points, 3);

        // 绘制富文本内容
        QTextDocument doc;
        doc.setDefaultFont(painter.font());
        doc.setHtml(m_htmlContent);
        QRect textRect = m_userType == User_Type::User_Me ? m_textRightRect : m_textLeftRect;
        painter.save();
        painter.translate(textRect.topLeft());
        doc.setTextWidth(textRect.width());
        doc.drawContents(&painter);
        painter.restore();

    }  else if(m_userType == User_Type::User_Time) {
        // 时间
        QPen penText;
        penText.setColor(QColor(153,153,153));
        painter.setPen(penText);
        QTextOption option(Qt::AlignCenter);
        option.setWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere);
        QFont te_font = this->font();
        te_font.setFamily("MicrosoftYaHei");
        te_font.setPointSize(10);
        painter.setFont(te_font);
        painter.drawText(this->rect(),m_curTime,option);
    }
}

#endif
