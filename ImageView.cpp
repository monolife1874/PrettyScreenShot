#include "ImageView.h"
#include <QWheelEvent>
#include <QMouseEvent>
#include <QGraphicsRectItem>
#include <QPaintEvent>
#include <QApplication>
#include <qmath.h>
#ifndef QT_NO_OPENGL
#include <QGLWidget>
#endif

ImageView::ImageView(QWidget *parent)
    : QGraphicsView(parent)
    , m_renderer(Native)
    , m_svgItem(nullptr)
    , m_backgroundItem(nullptr)
    , m_outlineItem(nullptr)
{
    clipboard = QApplication::clipboard();

    ToolBar* bar = new ToolBar(this);
    connect(bar->btn_copy, &QPushButton::clicked, this, &ImageView::onCopy);
    connect(bar->btn_top, &QPushButton::clicked, this, &ImageView::onTop);
    connect(bar->btn_save, &QPushButton::clicked, this, &ImageView::onSave);
    connect(bar->btn_rect, &QPushButton::clicked, this, &ImageView::onDrawRect);
    connect(bar->btn_reset, &QPushButton::clicked, this, &ImageView::onReset);


    setScene(new QGraphicsScene(this));
    setTransformationAnchor(AnchorUnderMouse);
    setDragMode(ScrollHandDrag);
    setViewportUpdateMode(FullViewportUpdate);

    // Prepare background check-board pattern
    QPixmap tilePixmap(64, 64);
    tilePixmap.fill(Qt::white);
    QPainter tilePainter(&tilePixmap);
    QColor color(220, 220, 220);
    tilePainter.fillRect(0, 0, 32, 32, color);
    tilePainter.fillRect(32, 32, 32, 32, color);
    tilePainter.end();

    setBackgroundBrush(tilePixmap);
}

void ImageView::drawBackground(QPainter *p, const QRectF &)
{
    p->save();
    p->resetTransform();
    p->drawTiledPixmap(viewport()->rect(), backgroundBrush().texture());
    p->restore();
}

void ImageView::setImage(QPixmap map)
{
    m_image = map.toImage();
    QGraphicsPixmapItem* item = new QGraphicsPixmapItem(map);
    QGraphicsScene* s = scene();
    s->clear();
    s->addItem(item);
    s->setSceneRect(item->boundingRect()/*.adjusted(-10, -10, 10, 10)*/);
    //this->adjustSize();;
    //this->resize(s->width(),s->height());

}

void ImageView::setRenderer(RendererType type)
{
    m_renderer = type;
    if (m_renderer == OpenGL) {
#ifndef QT_NO_OPENGL
        setViewport(new QGLWidget(QGLFormat(QGL::SampleBuffers)));
#endif
    } else {
        setViewport(new QWidget);
    }
}

void ImageView::setAntialiasing(bool antialiasing)
{
    setRenderHint(QPainter::Antialiasing, antialiasing);
}

void ImageView::setViewBackground(bool enable)
{
    if (!m_backgroundItem)
          return;

    m_backgroundItem->setVisible(enable);
}

void ImageView::setViewOutline(bool enable)
{
    if (!m_outlineItem)
        return;

    m_outlineItem->setVisible(enable);
}

qreal ImageView::zoomFactor() const
{
    return transform().m11();
}

void ImageView::zoomIn()
{
    zoomBy(2);
}

void ImageView::zoomOut()
{
    zoomBy(0.5);
}

void ImageView::resetZoom()
{
    if (!qFuzzyCompare(zoomFactor(), qreal(1))) {
        resetTransform();
        emit zoomChanged();
    }
}

void ImageView::onSave()
{
    QImage image(scene()->width(),scene()->height(),QImage::Format_RGB888);
    QPainter p(&image);
    p.setRenderHint(QPainter::HighQualityAntialiasing);
    //QPixmap map = this->grab();
    //image = map.toImage();
    this->render(&p);
    p.end();
    image.save("output.png");
}

void ImageView::onCopy()
{
    clipboard->setImage(m_image);
}

void ImageView::onTop()
{
    static bool onTop = false;
    onTop = !onTop;
    if (onTop) {
        this->setWindowFlag(Qt::WindowStaysOnTopHint);
        this->showNormal();
    }
    else {
        this->setWindowFlags(this->windowFlags()
            & ~Qt::WindowStaysOnTopHint);
        this->show();
    }
}

void ImageView::onDrawRect()
{
    setDragMode(NoDrag);
    drawType = DrawType::Rect;
}

void ImageView::onReset()
{
    setDragMode(ScrollHandDrag);
    drawType = DrawType::Default;
}

void ImageView::paintEvent(QPaintEvent *event)
{
    if (m_renderer == Image) {
        if (m_image.size() != viewport()->size()) {
            m_image = QImage(viewport()->size(), QImage::Format_ARGB32_Premultiplied);
        }

        QPainter imagePainter(&m_image);
        QGraphicsView::render(&imagePainter);
        imagePainter.end();

        QPainter p(viewport());
        p.drawImage(0, 0, m_image);

    } else {
        QGraphicsView::paintEvent(event);
    }
}

void ImageView::keyPressEvent(QKeyEvent* event) {
    if (event->modifiers() == Qt::ControlModifier && event->key() == Qt::Key_C) {
        onCopy();
    }
    if (event->modifiers() == Qt::ControlModifier && event->key() == Qt::Key_S) {
        onSave();
    }
    if (event->modifiers() == Qt::ControlModifier && event->key() == Qt::Key_T) {
        onTop();
    }
    event->ignore();
    //return QGraphicsView::QKeyEvent(event);
}

void ImageView::mousePressEvent(QMouseEvent* e) {
    pressed = true;

    auto drawRect = [&]() {
        roundedRect = new RoundedRectangleGraphicsItem();
        auto pos = mapToScene(e->pos());
        roundedRect->setRect(pos.x(), pos.y(), 0, 0);
        roundedRect->setCornerRadius(10.0);
        QGraphicsScene* s = scene();
        s->addItem(roundedRect);
    };

    switch (drawType)
    {
    case ImageView::Circle:
        break;
    case ImageView::Rect:
        drawRect();
        break;
    case ImageView::Line:
        break;
    case ImageView::Text:
        break;
    default:
        break;
    }

    return QGraphicsView::mousePressEvent(e);
}

void ImageView::mouseMoveEvent(QMouseEvent* e) {
    if (pressed) {
        auto addRect = [&]() {
            QRectF rect = roundedRect->rect();
            auto pos = mapToScene(e->pos());
            rect.setBottomRight(pos);
            roundedRect->setRect(rect);
        };

        switch (drawType)
        {
        case ImageView::Circle:
            break;
        case ImageView::Rect:
            addRect();
            break;
        case ImageView::Line:
            break;
        case ImageView::Text:
            break;
        case ImageView::Default:
            break;
        default:
            break;
        }

    }
    return QGraphicsView::mouseMoveEvent(e);
}

void ImageView::mouseReleaseEvent(QMouseEvent* e) {
    pressed = false;
    return QGraphicsView::mouseReleaseEvent(e);
}

void ImageView::wheelEvent(QWheelEvent *event)
{
    zoomBy(qPow(1.2, event->angleDelta().y() / 240.0));
}

void ImageView::zoomBy(qreal factor)
{
    const qreal currentZoom = zoomFactor();
    if ((factor < 1 && currentZoom < 0.1) || (factor > 1 && currentZoom > 10))
        return;
    scale(factor, factor);
    emit zoomChanged();
}
