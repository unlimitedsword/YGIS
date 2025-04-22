#pragma once
#include <QMainWindow>
#include <QTableView>
#include <QMenuBar>

class VectorElement :public QMainWindow {
	Q_OBJECT
public:
	explicit VectorElement(QWidget* parent = nullptr);

	void vectorElementInfo(const QString filePath);

	void deleteElement();
	void saveElement();

private:
	QTableView* m_elementView;
	QMenuBar* m_MenuBar;
	QAction* m_deleteAction;
	QAction* m_saveAction;
	QString m_filePath; // 用于存储文件路径
	QList<int> m_deletedFeatureIds; // 存储被删除的要素 ID
};