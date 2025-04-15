
#include <QVBoxLayout>
#include <QFileDialog>
#include <QMenu>
#include <gdal.h>
#include <gdal_priv.h>
#include <ogrsf_frmts.h>
#include "FileWidget.h"
#include "TextWidget.h"



namespace CustomRole {
	enum {
		
		FilePathRole = Qt::UserRole + 1,	//文件完整路径
		FileTypeRole,	//文件种类
		FileBaseName,	//文件名
		GraphicStatus,	//显示状态
		Color	//矢量的颜色
	};
}

FileWidget::FileWidget(QWidget* parent)
	: QDockWidget("文件列表", parent), treeview(nullptr) { // 初始化 treeview
	container = new QWidget(this);

	treeview = new QTreeView(container);

	QVBoxLayout* layout = new QVBoxLayout(container);
	layout->setContentsMargins(0, 0, 0, 0); // 去除边距
	layout->addWidget(treeview);

	model = new QStandardItemModel(container);

	treeview->setModel(model);

	// 连接 itemChanged 信号
	connect(model, &QStandardItemModel::itemChanged, this, &FileWidget::onItemChanged);

	//右键菜单
	treeview->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(treeview, &QTreeView::customContextMenuRequested,
		this, &FileWidget::onCustomContextMenuRequested);

	this->setWidget(container);
}

void FileWidget::appendFile() {
	QString fileName = QFileDialog::getOpenFileName(this, "选择文件",
		"/",
		"栅格数据(*.tif *.tiff);;矢量数据(*.shp)");

	if (fileName.isEmpty()) {
		return; // 如果未选择文件，则返回
	}

	QString baseName = QFileInfo(fileName).fileName(); // "example.tif"

	QStandardItem* fileItem = new QStandardItem(baseName);
	fileItem->setData(baseName, CustomRole::FileBaseName);
	fileItem->setData(fileName, CustomRole::FilePathRole);
	fileItem->setData(true, CustomRole::GraphicStatus);
	fileItem->setFlags(fileItem->flags() | Qt::ItemIsUserCheckable); // 启用复选框
	fileItem->setCheckState(Qt::Checked); // 设置初始状态

	if (fileName.endsWith(".tif", Qt::CaseInsensitive) || fileName.endsWith(".tiff", Qt::CaseInsensitive)) {
		fileItem->setData("Raster", CustomRole::FileTypeRole);
	}
	else if (fileName.endsWith(".shp", Qt::CaseInsensitive)) {
		fileItem->setData("Vector", CustomRole::FileTypeRole);
		GDALDataset* poDS = (GDALDataset*)GDALOpenEx(
			fileName.toUtf8(), GDAL_OF_VECTOR, nullptr, nullptr, nullptr);
		if (!poDS) {
			qDebug() << "打开SHP文件失败";
			return;
		}

		OGRLayer* poLayer = poDS->GetLayer(0);
		if (!poLayer) {
			GDALClose(poDS);
			return;
		}
		// 判断矢量要素类型
        OGRFeature* poFeature = poLayer->GetNextFeature(); // 获取第一个要素
		if (poFeature) {
			OGRGeometry* poGeometry = poFeature->GetGeometryRef();
			if (poGeometry) {
				switch (wkbFlatten(poGeometry->getGeometryType())) {
					case wkbPoint: {
						fileItem->setData("red", CustomRole::Color);
						break;
					}
					case wkbLineString: {
						fileItem->setData("blue", CustomRole::Color);
						break;
					}
					case wkbPolygon: {
						fileItem->setData("green", CustomRole::Color);
						break;
					}
				}
			}
			OGRFeature::DestroyFeature(poFeature); // 清理要素
		}

        OGRGeometry* poGeometry = poFeature->GetGeometryRef();  
        if (!poGeometry) {  
           OGRFeature::DestroyFeature(poFeature); // Clean up the feature  
           GDALClose(poDS);  
           return;  
        }    

	}
	else {
		fileItem->setData("Unknown", CustomRole::FileTypeRole);
	}
	model->appendRow(fileItem);

	updateFileListSignal();
}


void FileWidget::onCustomContextMenuRequested(const QPoint& pos) {
	QModelIndex index = treeview->indexAt(pos);
	if (!index.isValid()) return;

	contextMenuIndex = index; // 保存当前操作的项索引

	QMenu menu;

	QAction* visibilityAction = menu.addAction(
		index.data(CustomRole::GraphicStatus).toBool() ? "隐藏文件" : "显示文件"
	);
	QAction* deleteAction = menu.addAction("删除文件");
	QAction* showPropertiesAction = menu.addAction("属性");


	// 连接菜单动作到槽函数
	connect(deleteAction, &QAction::triggered, this, &FileWidget::deleteSelectedItem);
	connect(visibilityAction, &QAction::triggered, this, &FileWidget::toggleVisibility);
	connect(showPropertiesAction, &QAction::triggered, this, &FileWidget::deliverDataPath);

	menu.exec(treeview->viewport()->mapToGlobal(pos));
}

void FileWidget::deleteSelectedItem() {
	if (contextMenuIndex.isValid()) {
		model->removeRow(contextMenuIndex.row());
		updateFileListSignal();
	}
}

void FileWidget::toggleVisibility() {
    if (contextMenuIndex.isValid()) {
        QStandardItem* item = model->itemFromIndex(contextMenuIndex);
        bool newStatus = !item->data(CustomRole::GraphicStatus).toBool();

        // 更新数据角色和复选框状态
        item->setData(newStatus, CustomRole::GraphicStatus);
        item->setCheckState(newStatus ? Qt::Checked : Qt::Unchecked);

        // 获取文件路径
        QString filePath = item->data(CustomRole::FilePathRole).toString();

        // 更新 QMap 状态
        QMap<QString, Information> fileList;
        for (int i = 0; i < model->rowCount(); ++i) {
            QStandardItem* currentItem = model->item(i);
            QString currentFilePath = currentItem->data(CustomRole::FilePathRole).toString();
            bool currentStatus = currentItem->data(CustomRole::GraphicStatus).toBool();
			QColor color = currentItem->data(CustomRole::Color).value<QColor>();
            fileList.insert(currentFilePath, Information{currentStatus,color});
        }

        // 发射信号，传递更新后的文件列表
        emit fileListUpdated(fileList);
    }
}

void FileWidget::onItemChanged(QStandardItem* item) {
	static bool inProgress = false;
	if (inProgress) return;
	inProgress = true;

	if (item->isCheckable()) {
		contextMenuIndex = model->indexFromItem(item);
		toggleVisibility();
	}

	inProgress = false;
}

void FileWidget::deliverDataPath() {
	if (contextMenuIndex.isValid()) {
		QString filePath = model->data(contextMenuIndex, CustomRole::FilePathRole).toString();
		emit filePathDelivered(filePath); // 发射信号
	}
}

void FileWidget::updateFileListSignal() {
   QMap<QString, Information> fileList;
   for (int i = 0; i < model->rowCount(); ++i) {
       QStandardItem* item = model->item(i);
       QString filePath = item->data(CustomRole::FilePathRole).toString();
       bool status = item->data(CustomRole::GraphicStatus).toBool();
       QColor color = item->data(CustomRole::Color).value<QColor>(); // Explicitly convert QVariant to QColor
       fileList.insert(filePath, Information{status, color});
   }
   emit fileListUpdated(fileList);
}