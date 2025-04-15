#include <QHBoxLayout>
#include <QVBoxLayout>
#include <gdal.h>
#include <gdal_priv.h>
#include <ogrsf_frmts.h>
#include "YGIS.h"

YGIS::YGIS(QWidget* parent) : QMainWindow(parent) {
    createMenus();

    mapWidget = new MapWidget; 
    fileWidget = new FileWidget;  
    textWidget = new TextWidget(this);  
    textWidget->setEnabled(true); // 控件启用状态

    QWidget* centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    // 主布局结构
    QHBoxLayout* mainLayout = new QHBoxLayout(centralWidget);
    QVBoxLayout* rightLayout = new QVBoxLayout();

    // 配置右侧布局
    rightLayout->addWidget(mapWidget, 3);  // 3/4 的空间给地图
    rightLayout->addWidget(textWidget, 1);  // 1/4 的空间给文本

    mainLayout->addWidget(fileWidget, 1);    // 左侧文件窗口占1份宽度
    mainLayout->addLayout(rightLayout, 3);   // 右侧布局占3份宽度

    // 设置最小尺寸
    setMinimumSize(800, 600);

    // 初始化GDAL/OGR库 
    GDALAllRegister();
    OGRRegisterAll();

/*----------------------------------------------------------以下为信号槽连接部分----------------------------------------------------------------*/

    connect(openFile, &QAction::triggered, fileWidget, &FileWidget::appendFile);  //打开并添加文件
    connect(fileWidget, &FileWidget::filePathDelivered, textWidget, &TextWidget::dataPathReceived);  //传输路径给编辑框
    connect(fileWidget, &FileWidget::fileListUpdated, mapWidget, &MapWidget::updateFilePathList); //同步文件列表与mapCanvas文件列表
}

YGIS::~YGIS()
{}

void YGIS::createMenus() {
    // 获取主窗口的菜单栏（自动创建）
    QMenuBar* mainMenuBar = menuBar();

    // 创建菜单
    QMenu* fileMenu = mainMenuBar->addMenu(tr("&File"));
    QMenu* aboutMenu = mainMenuBar->addMenu(tr("&About"));

    // 创建菜单项
    openFile = new QAction(tr("&Open File"), this);
    refresh = new QAction(tr("&Refresh"), this);

    // 添加菜单项到 File 菜单
    fileMenu->addAction(openFile);
    fileMenu->addAction(refresh);

    // 添加分隔线
    //fileMenu->addSeparator();
}
