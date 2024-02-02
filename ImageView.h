#ifndef IMAGEVIEW_H
#define IMAGEVIEW_H

#include <QGraphicsView>
#include <QPushButton>
#include <QLayout>
#include <QClipBoard>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QDebug>
QT_BEGIN_NAMESPACE
class QGraphicsSvgItem;
class QSvgRenderer;
class QWheelEvent;
class QPaintEvent;
QT_END_NAMESPACE


#include <QGraphicsRectItem>
#include <QPainterPath>
#include <QGraphicsSceneMouseEvent>

class RoundedRectangleGraphicsItem : public QGraphicsRectItem
{
public:
    RoundedRectangleGraphicsItem(QGraphicsItem* parent = nullptr)
        : QGraphicsRectItem(parent), m_cornerRadius(10.0) {}

    void setCornerRadius(qreal radius) { m_cornerRadius = radius; }

protected:
    QRectF boundingRect() const override
    {
        return rect();
        qreal extra = qMax(m_cornerRadius, (qreal)2);
        return QRectF(-extra, -extra, rect().width() + 2 * extra,
            rect().height() + 2 * extra);
    }

    void paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*) override
    {
        QPainterPath path;
        path.addRoundedRect(rect(), m_cornerRadius, m_cornerRadius);
        painter->setRenderHint(QPainter::HighQualityAntialiasing);
        QPen pen(Qt::cyan);
        pen.setWidth(2);
        painter->setPen(pen);
        painter->drawPath(path);
    }

    QVariant itemChange(GraphicsItemChange change, const QVariant& value) override
    {
        update(); 
        return QGraphicsRectItem::itemChange(change, value);
    }

    void mousePressEvent(QGraphicsSceneMouseEvent* e) {
        press = true;
        start = e->pos();
        mapToScene(e->pos());
    }
    void mouseMoveEvent(QGraphicsSceneMouseEvent* e) override {
        if (!press)
            return;
        QPointF temp = mapToScene(e->pos() - start);
        this->setPos(temp);
    }
    void mouseReleaseEvent(QGraphicsSceneMouseEvent* e) override {
        press = false;
    }

private:
    qreal m_cornerRadius;
    QPointF start;
    bool press = false;
};

class ToolBar : public QWidget {
    Q_OBJECT
private:


public:
    explicit ToolBar(QWidget* parent = nullptr) :QWidget(parent) {
        this->resize(80,200);

        int fixWidth = 80;

        btn_copy = new QPushButton("Copy", this);
        btn_copy->setFixedHeight(30);
        btn_copy->setFixedWidth(fixWidth);

        btn_save = new QPushButton("Save", this);
        btn_save->setFixedHeight(30);
        btn_save->setGeometry(0,0,1200,30);
        //btn_save->setFixedWidth(fixWidth);

        btn_top = new QPushButton("Top", this);
        btn_top->setFixedHeight(30);
        btn_top->setFixedWidth(fixWidth);

        btn_circle = new QPushButton("Circle", this);
        btn_circle->setFixedHeight(30);
        btn_circle->setFixedWidth(fixWidth);

        btn_rect = new QPushButton("Rect", this);
        btn_rect->setFixedHeight(30);
        btn_rect->setFixedWidth(fixWidth);

        btn_line = new QPushButton("Line", this);
        btn_line->setFixedHeight(30);
        btn_line->setFixedWidth(fixWidth);

        btn_text = new QPushButton("Text", this);
        btn_text->setFixedHeight(30);
        btn_text->setFixedWidth(fixWidth);

        btn_reset = new QPushButton("Reset", this);
        btn_reset->setFixedHeight(30);
        btn_reset->setFixedWidth(fixWidth);

        QVBoxLayout* layout = new QVBoxLayout(this);
        layout->addWidget(btn_top);
        layout->addWidget(btn_save);
        layout->addWidget(btn_copy);
        layout->addWidget(btn_circle);
        layout->addWidget(btn_rect);
        layout->addWidget(btn_line);
        layout->addWidget(btn_text);
        layout->addWidget(btn_reset);

        //!TODO:Implement Draw Functions

        this->adjustSize();
        this->setLayout(layout);
    }
    QPushButton* btn_copy;
    QPushButton* btn_save;
    QPushButton* btn_top;
    QPushButton* btn_circle;
    QPushButton* btn_rect;
    QPushButton* btn_line;
    QPushButton* btn_text;
    QPushButton* btn_reset;

};

class ImageView : public QGraphicsView
{
    Q_OBJECT

public:
    enum RendererType { Native, OpenGL, Image };
    enum DrawType {Circle,Rect,Line,Text,Default};
    explicit ImageView(QWidget *parent = nullptr);

    void setImage(QPixmap map);
    void setRenderer(RendererType type = Native);
    void drawBackground(QPainter *p, const QRectF &rect) override;
    qreal zoomFactor() const;

public slots:
    void setAntialiasing(bool antialiasing);
    void setViewBackground(bool enable);
    void setViewOutline(bool enable);
    void zoomIn();
    void zoomOut();
    void resetZoom();

    void onSave();
    void onCopy();
    void onTop();
    void onDrawRect();
    void onReset();
signals:
    void zoomChanged();

protected:
    void wheelEvent(QWheelEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
    void keyPressEvent(QKeyEvent*event) override;
    void mousePressEvent(QMouseEvent* e) override;
    void mouseMoveEvent(QMouseEvent* e) override;
    void mouseReleaseEvent(QMouseEvent* e) override;
private:
    void zoomBy(qreal factor);

    RendererType m_renderer;
    bool pressed = false;
    QGraphicsSvgItem *m_svgItem;
    QGraphicsRectItem *m_backgroundItem;
    QGraphicsRectItem *m_outlineItem;
    RoundedRectangleGraphicsItem* roundedRect;
    QClipboard* clipboard;
    QImage m_image;
    DrawType drawType = Default;
};
#endif // IMAGEVIEW_H
