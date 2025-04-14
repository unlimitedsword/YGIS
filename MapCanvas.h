#pragma once  
#include <QGraphicsView>  
#include <QWheelEvent>  
#include <QMouseEvent>  
#include <QScrollBar> 

class MapCanvas : public QGraphicsView {  
   Q_OBJECT  

public:  
   explicit MapCanvas(QWidget* parent = nullptr)  
       : QGraphicsView(parent), initialScale(1.0), currentScale(1.0), isPanning(false) {  
   }  

signals:
    void zoomChanged(qreal scale); // 缩放比例变化信号

protected:  
   // 鼠标滚轮缩放  
    void wheelEvent(QWheelEvent* event) override {
        const double scaleFactor = 1.15;
        const double minScale = 0.1;
        const double maxScale = 10.0;

        if (event->angleDelta().y() > 0 && currentScale < maxScale) {
            // 放大视图
            scale(scaleFactor, scaleFactor);
            currentScale *= scaleFactor;
            emit zoomChanged(currentScale); // 发射信号
        }
        else if (event->angleDelta().y() < 0 && currentScale > minScale) {
            // 缩小视图
            scale(1.0 / scaleFactor, 1.0 / scaleFactor);
            currentScale /= scaleFactor;
            emit zoomChanged(currentScale); // 发射信号
        }
    }

   // 鼠标按下事件  
   void mousePressEvent(QMouseEvent* event) override {  
       if (event->button() == Qt::LeftButton) {  
           isPanning = true; // 开始平移  
           lastMousePos = event->pos(); // 记录鼠标位置  
           setCursor(Qt::ClosedHandCursor); // 设置鼠标样式为抓手  
       }  
       QGraphicsView::mousePressEvent(event); // 保留默认行为  
   }  

   // 鼠标移动事件  
   void mouseMoveEvent(QMouseEvent* event) override {  
       if (isPanning) {  
           QPoint delta = event->pos() - lastMousePos; // 计算鼠标移动的偏移量  
           lastMousePos = event->pos(); // 更新鼠标位置  
           horizontalScrollBar()->setValue(horizontalScrollBar()->value() - delta.x()); // 平移水平滚动条  
           verticalScrollBar()->setValue(verticalScrollBar()->value() - delta.y()); // 平移垂直滚动条  
       }  
       QGraphicsView::mouseMoveEvent(event); // 保留默认行为  
   }  

   // 鼠标释放事件  
   void mouseReleaseEvent(QMouseEvent* event) override {  
       if (event->button() == Qt::LeftButton) {  
           isPanning = false; // 停止平移  
           setCursor(Qt::ArrowCursor); // 恢复鼠标样式  
       }  
       QGraphicsView::mouseReleaseEvent(event); // 保留默认行为  
   }  

private:  
   qreal initialScale; // 初始比例  
   qreal currentScale; // 当前比例  
   bool isPanning; // 是否正在平移  
   QPoint lastMousePos; // 上一次鼠标位置  
};
