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
#include "ui_QGrabScreenImage.h"
#include "ImageView.h"

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
        this->setPalette(QPalette(QColor(20, 20, 50, 128)));
        this->setAutoFillBackground(true);

        btnOK = new QPushButton(this);
        btnOK->hide();
        btnOK->setPalette(QPalette(QColor(20, 20, 50, 128)));
        btnOK->setAutoFillBackground(true);        
        btnOK->setWindowFlags(btnOK->windowFlags()
            | Qt::WindowStaysOnTopHint
            | Qt::FramelessWindowHint);
        connect(btnOK, &QPushButton::clicked, this, &QGrabScreenImage::onClip);
    }
    ~QGrabScreenImage()
    {

    }
protected: 
    QPushButton* btnOK = nullptr;
    ImageView* view = nullptr;

    void closeEvent(QCloseEvent* event) override {
        view->close();
        btnOK->close();
        delete view;
        view = nullptr;
        delete btnOK;
        btnOK = nullptr;
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
            btnOK->setGeometry(imageRect.bottomRight().x(), imageRect.bottomRight().y(), 60, 60);
            btnOK->showNormal();
            btnOK->show();
        }
    }
    void mouseMoveEvent(QMouseEvent* event) override {

        if (pressed) {
            end = event->pos();
            if (moved()) {
                btnOK->hide();
                imageRect.setBottomRight(end);
                regionRectAll.setRects(&this->rect(), 1);
                regionRectShow.setRects(&imageRect, 1);
                QRegion sub = regionRectAll.subtracted(regionRectShow);
                this->setMask(sub);
                imageRectShow = imageRect;
            }          
        }
    }

    void paintEvent(QPaintEvent* event) override {
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

        view->move(start);
        view->setImage(pix);
        view->setWindowFlags(view->windowFlags() | Qt::WindowStaysOnTopHint);
        view->showNormal();
    }
};
