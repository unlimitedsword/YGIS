// TextWidget.h
#pragma once
#include <QDockWidget>
#include <QTextEdit>

class TextWidget : public QDockWidget {
    Q_OBJECT
public:
    explicit TextWidget(QWidget* parent = nullptr);

public slots:
    void dataPathReceived(QString datapath);

private:
    QTextEdit* textEdit;
    QWidget* container; // 新增容器控件
};