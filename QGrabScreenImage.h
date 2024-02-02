#pragma once

#include <QtWidgets/QMainWindow>
#include <QPainter>
#include <QMouseEvent>
#include <QCloseEvent>
#include <QDebug>
#include <QRegion>
#include <QPixmap>
#include <QScreen>
#include <QDesktopWidget>
#include <QPushButton>
#include <QQueue>
#include <QFocusEvent>
#include <windows.h>
#include "ui_QGrabScreenImage.h"
#include "ImageView.h"

class ImageStack {
private:
    int width = 0, height = 0;
    int totalWidth = 0, totalHeight = 0;
    bool vertical = false;
    QQueue<QImage> images;
public:
    ImageStack(bool vertical = true) :vertical(vertical) {
    
    }

    bool add(const QImage& image) {
        if (image.isNull() || image.width() == 0 || image.height() == 0)
            return false;
        if (height == 0 || width == 0) {
            height = image.height();
            width = image.width();
        }
        else {
            if (vertical && image.width() != width)
                return false;
            if (!vertical && image.height() != height)
                return false;
        }
        //if (image.format() != QImage::Format_RGB888) {
        //    return false;
        //}
        if (vertical) {
            totalHeight += image.height();
            totalWidth = width;
        }
        else {
            totalWidth += image.width();
            totalHeight = height;
        }
        images.append(image);
        return true;
    }

    int size() {
        return images.size();
    }

    void clear() {
        width = 0;
        height = 0;
        totalWidth = 0;
        totalHeight = 0;
        images.clear();
    }

    QImage combine() {

        QImage imageOverlay = QImage(totalWidth, totalHeight, QImage::Format_RGB888);
        QPainter painter(&imageOverlay);

        int offset = 0;
        for (auto it = images.begin(); it != images.end(); it++) {
            QImage image = *it;
            if (vertical) {
                painter.drawImage(0, offset, image);
                offset += image.height();
            }
            else {
                painter.drawImage(offset, 0, image);
                offset += image.width();
            }
        }
        return imageOverlay;
    }
};

//TODO:add 4 control bars, so user can adjust show region

class QGrabScreenImage : public QWidget
{
    Q_OBJECT

public:
    QGrabScreenImage(QWidget* parent = nullptr)
        : QWidget(parent)
    {
        view = new ImageView;
        //btnOK->setOpacity(1);
        //btnOK->setWindowFlags(Qt::FramelessWindowHint);


        int screenWidth = QApplication::desktop()->width();
        int screenHeight = QApplication::desktop()->height();
        this->setGeometry(0, 0, screenWidth, screenHeight);
        this->setWindowFlags(this->windowFlags()
            | Qt::WindowStaysOnTopHint
            | Qt::FramelessWindowHint);

        this->setWindowOpacity(128 / 255.0);
        this->setPalette(QPalette(QColor(20, 20, 50, 20)));
        this->setAutoFillBackground(true);

        btnOK = new QPushButton("Ok", this);
        btnOK->hide();
        btnOK->setPalette(QPalette(QColor(200, 20, 50, 128)));
        btnOK->setAutoFillBackground(true);        
        btnOK->setWindowFlags(btnOK->windowFlags()
            | Qt::WindowStaysOnTopHint
            | Qt::FramelessWindowHint);
        connect(btnOK, &QPushButton::clicked, this, &QGrabScreenImage::onClip);

        btnVerticalAdd = new QPushButton("Add", this);
        btnVerticalAdd->hide();
        btnVerticalAdd->setPalette(QPalette(QColor(200, 20, 50, 128)));
        btnVerticalAdd->setAutoFillBackground(true);
        btnVerticalAdd->setWindowFlags(/*btnVerticalAdd->windowFlags()
            | */Qt::WindowStaysOnTopHint
            | Qt::FramelessWindowHint);
        connect(btnVerticalAdd, &QPushButton::clicked, this, &QGrabScreenImage::onAdd);

        imageStack = new ImageStack();
    }
    ~QGrabScreenImage()
    {
        delete imageStack;
    }
protected: 
    QPushButton* btnOK = nullptr;
    QPushButton* btnVerticalAdd = nullptr;
    ImageView* view = nullptr;

    void closeEvent(QCloseEvent* event) override {
        view->close();
        btnOK->close();
        btnVerticalAdd->close();
        delete view;
        view = nullptr;
        delete btnOK;
        btnOK = nullptr;
        delete btnVerticalAdd;
        btnVerticalAdd = nullptr;
    }

    void mousePressEvent(QMouseEvent* event) override {
        start = event->pos();
        imageRect.setTopLeft(start);
        pressed = true;
    }
    void mouseReleaseEvent(QMouseEvent* event) override {
        pressed = false;
        end = event->pos();
        if (moved()) {
            btnOK->setGeometry(imageRect.bottomRight().x() - 60, imageRect.bottomRight().y(), 60, 30);
            btnOK->showNormal();
            btnOK->show();

            btnVerticalAdd->setGeometry(imageRect.bottomRight().x() - 120, imageRect.bottomRight().y(), 60, 30);
            btnVerticalAdd->showNormal();
            btnVerticalAdd->show();
            //btnVerticalAdd
        }
    }
    void mouseMoveEvent(QMouseEvent* event) override {

        if (pressed) {
            end = event->pos();
            if (moved()) {
                btnOK->hide();
                btnVerticalAdd->hide();
                imageRect.setBottomRight(end);
                regionRectAll.setRects(&this->rect(), 1);
                regionRectShow.setRects(&imageRect, 1);
                QRegion sub = regionRectAll.subtracted(regionRectShow);
                this->setMask(sub);
                imageRectShow = imageRect;
                update();
            }          
        }
    }

    void paintEvent(QPaintEvent* event) override {
        QPen pen;
        pen.setStyle(Qt::PenStyle::DashLine);
        pen.setColor(Qt::gray);
        pen.setWidth(4);
        QPainter painter(this);
        painter.setPen(pen);
        painter.drawRect(imageRect);
        //QPainter painter(this);
    }

private:
    QRegion regionRectShow;
    QRegion regionRectAll;
    bool pressed = false;
    QRect imageRect;
    QRect imageRectShow;

    QPoint start;
    QPoint end;
    Ui::QGrabScreenImageClass ui;
    ImageStack* imageStack;

    bool moved() {
        return start != end;
    }

private slots:
    void onClip() {
        QScreen* scr = QGuiApplication::primaryScreen();
        QPixmap pix = scr->grabWindow(0).copy(imageRectShow);
        QImage clipImage = pix.toImage();
        clipImage.save("./img.bmp");
        this->showMinimized();
        btnOK->showMinimized();
        btnVerticalAdd->showMinimized();

        if (imageStack->size() > 0) {
            view->setImage(QPixmap::fromImage(imageStack->combine()));
            imageStack->clear();
        }
        else {
            view->setImage(pix);
        }

        view->move(start);
        view->setWindowFlags(view->windowFlags() | Qt::WindowStaysOnTopHint);
        view->showNormal();
    }
    void onAdd() {
        QScreen* scr = QGuiApplication::primaryScreen();
        QPixmap pix = scr->grabWindow(0).copy(imageRectShow);
        QImage clipImage = pix.toImage();
        imageStack->add(clipImage);
    }
};
