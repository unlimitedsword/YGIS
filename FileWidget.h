#pragma once
#include <QWidget>
#include <QDockWidget>
#include <QTreeView>
#include <QStandardItemModel>

class FileWidget : public QDockWidget 
{
    Q_OBJECT
public:
    explicit FileWidget(QWidget* parent = nullptr);

    void updateFileListSignal();

public slots:
    void appendFile();

signals:
    void filePathDelivered(const QString& filePath); // 中间函数发送信号

    void fileListUpdated(const QMap<QString, bool>& fileList);




private:
    QWidget* container; // 新增容器控件
    QTreeView* treeview;
    QStandardItemModel* model;
    QModelIndex contextMenuIndex; // 保存右键时的项索引

private slots:
    // 新增右键菜单槽函数
    void onCustomContextMenuRequested(const QPoint& pos);
    void deleteSelectedItem();
    void toggleVisibility();
    void deliverDataPath();
};