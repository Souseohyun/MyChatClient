#include "addsubgroup.h"
#include "sqlite/sqldatabase.h"

#include <QSqlQuery>
#include <QMessageBox>
#include <QDateTime>
#include <QVBoxLayout>

AddSubGroup::AddSubGroup(QWidget *parent) : QDialog(parent) {
    setWindowTitle("新建分组");
    setFixedSize(350, 150);
    setWindowFlags(Qt::WindowCloseButtonHint);

    QFont font("Microsoft YaHei", 12, 50, false);
    QFont font2("Microsoft YaHei", 10, 50, false);

    nameLabel = new QLabel("请输入分组名：", this);
    nameLabel->setFont(font);

    nameEdit = new QLineEdit(this);
    nameEdit->setFont(font2);

    okBtn = new QPushButton("确定", this);
    cancelBtn = new QPushButton("取消", this);

    QString buttonStyle = "QPushButton{border:1px solid #86949e;background-color:#0188fb;border-radius:5px}"
                          "QPushButton:hover{background-color:#289cff;}"
                          "QPushButton:pressed{background-color:#0081ef}";
    okBtn->setStyleSheet(buttonStyle);
    cancelBtn->setStyleSheet(buttonStyle);

    connect(okBtn, &QPushButton::clicked, this, &AddSubGroup::sltBtnClicked);
    connect(cancelBtn, &QPushButton::clicked, this, &AddSubGroup::close);

    // Layout
    QVBoxLayout* layout = new QVBoxLayout(this);
    QHBoxLayout* inputLayout = new QHBoxLayout;
    QHBoxLayout* buttonsLayout = new QHBoxLayout;

    inputLayout->addWidget(nameLabel);
    inputLayout->addWidget(nameEdit);

    buttonsLayout->addWidget(okBtn);
    buttonsLayout->addWidget(cancelBtn);

    layout->addLayout(inputLayout);
    layout->addLayout(buttonsLayout);

    setLayout(layout);
}


void AddSubGroup::sltBtnClicked()
{
    QString text = nameEdit->text().trimmed(); // 使用trimmed()去除前后空格
    if(text.size() > 20){
        QMessageBox::information(this, "错误", "分组名不能多于20个字符!");
        return;
    } else {
        auto jsonArray = SqlDataBase::Instance()->getMySubgroup(); // 确保这一行正确地获取了分组信息
        for (const auto& item : jsonArray) {
            QString name = QString::fromStdString(item["name"]); // 将std::string转换为QString
            if(text == name){
                QMessageBox::information(this, "错误", "分组中已存在该分组名!");
                return;
            }
        }

        // 插入新分组到数据库
        QSqlQuery query;
        QString sql = QString("INSERT INTO MySubgroup (name, datetime) VALUES ('%1', '%2')")
                          .arg(text)
                          .arg(QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss"));
        if (!query.exec(sql)) {
            QMessageBox::critical(this, "错误", "无法添加分组到数据库!");
            return;
        }

        emit updateList(text); // 发射信号，更新分组列表

        this->close(); // 关闭窗口
    }
}

