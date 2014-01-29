#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include <QtTest/QTest>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    template_images = NULL;

    QImage qp("National-Instruments_PWRD.bmp");
    QImage qp2("Metron logo.bmp");

    ui->label_2->setPixmap(QPixmap::fromImage(qp));
    ui->label_3->setPixmap(QPixmap::fromImage(qp2));

    motion_done = true;

    if (!Initialize_Cameras())
    {
        ui->Start->setEnabled(false);
        ui->Stop->setEnabled(false);

        QMessageBox::warning(this, tr("Camera"), tr("Cannot find AVT camera!"), QMessageBox::Ok);
    }

    camera_thread.Initialize(&vw);

    Setup_Lookup_Table();
    Load_Templates();

    bool temp = Setup_Serial_Port();

    qDebug() << "Open serial port:" << temp ;

    if (!temp)
    {
        QMessageBox::warning(this, tr("Serial Port"), tr("Cannot open serial port!"), QMessageBox::Ok);
    }


    Connect_Signals();

    matcher = new FlannBasedMatcher();



}

MainWindow::~MainWindow()
{
    if (camera_thread.isRunning())
    {
        camera_thread.keep_capturing = false;
    }

    if (camera_thread.isRunning())
    {
        while (camera_thread.isFinished() != true)
        {
            QTest::qWait(50);
        }
    }


    if (template_images != NULL)
    {
        delete [] template_images;
    }

    delete matcher;

    delete ui;
}

bool MainWindow::Initialize_Cameras()
{
    bool ok;

    ok = vw.Load_Settings();

    if (!ok) return false;

    ok = vw.Startup();
    if (!ok) return false;

    vw.List_Cameras();

    ok = vw.Open_Cameras();
    if (!ok) return false;

    ok = vw.Start_Acquisition(0);
    if (!ok) return false;

    return true;
}

bool MainWindow::Load_Templates()
{
    QSettings set("templates/templates.ini", QSettings::IniFormat);

    bool ok;
    QString temp;

    template_count = set.value("Main/Count").toInt(&ok);
    if (!ok) return false;

    template_images = new Mat[template_count];

    hessian_threshold = set.value("Vision/Hessian").toInt(&ok);
    if (!ok) return false;

    threshold = set.value("Vision/Threshold").toInt(&ok);
    if (!ok) return false;

    min_area = set.value("Vision/Min_Area").toInt(&ok);
    if (!ok) return false;

    // set feature threshold
    detector.hessianThreshold = hessian_threshold;

    vector<KeyPoint> keypoints;
    Mat descriptors;

    for (int i=0 ;i<template_count; i++)
    {
        temp = set.value(QString("Template%1/File_name").arg(i)).toString();

        template_images[i] = imread(QString("templates/" + temp).toStdString()  , 0);

        temp = set.value(QString("Template%1/MJ_name").arg(i)).toString();
        MJ_names.append(temp);

        // extractor features
        qDebug() << "Extract features for template:" << i;
        detector.detect(template_images[i], keypoints);


        Mat t;
        drawKeypoints(template_images[i], keypoints, t, Scalar::all(-1), DrawMatchesFlags::NOT_DRAW_SINGLE_POINTS);
        imwrite(QString("c%1.bmp").arg(i).toStdString(), t);

        qDebug() << "Compute descriptors for template:" << i;
        extractor.compute(template_images[i], keypoints, descriptors);
        ref_descriptors.append(descriptors);

    }

    return true;
}

void MainWindow::Setup_Lookup_Table()
{
    // Setup Wan

    map.insert("1W", tr("1W"));
    map.insert("2W", tr("2W"));
    map.insert("3W", tr("3W"));
    map.insert("4W", tr("4W"));
    map.insert("5W", tr("5W"));
    map.insert("6W", tr("6W"));
    map.insert("7W", tr("8W"));
    map.insert("8W", tr("7W"));
    map.insert("9W", tr("9W"));

    // Setup Suo

    map.insert("1S", tr("1S"));
    map.insert("2S", tr("2S"));
    map.insert("3S", tr("3S"));
    map.insert("4S", tr("4S"));
    map.insert("5S", tr("5S"));
    map.insert("6S", tr("6S"));
    map.insert("7S", tr("7S"));
    map.insert("8S", tr("8S"));
    map.insert("9S", tr("9S"));

    // Setup Tong

    map.insert("1T", tr("1T"));
    map.insert("2T", tr("2T"));
    map.insert("3T", tr("3T"));
    map.insert("4T", tr("4T"));
    map.insert("5T", tr("5T"));
    map.insert("6T", tr("6T"));
    map.insert("7T", tr("7T"));
    map.insert("8T", tr("8T"));
    map.insert("9T", tr("9T"));

    // Setup Fan
    map.insert("East", tr("East"));
    map.insert("West", tr("West"));
    map.insert("South", tr("South"));
    map.insert("North", tr("North"));
    map.insert("Centre", tr("Centre"));
    map.insert("Fa", tr("Fa"));
    map.insert("Ban", tr("Ban"));

    // Setup Flower
    map.insert("2F", tr("2F"));

}

bool MainWindow::Setup_Serial_Port()
{
    QSettings set("settings/com.ini", QSettings::IniFormat);

    QString port = set.value("Serial/Com_Port").toString();

    serial_port.setPortName((QString("\\\\.\\%1").arg(port)));

    qDebug() << "Attempt to open:" << port;

    bool temp = serial_port.open(QIODevice::ReadWrite);

    if (!temp) return false;

    serial_port.setBaudRate(BAUD9600);
    serial_port.setFlowControl(FLOW_OFF);
    serial_port.setParity(PAR_NONE);
    serial_port.setDataBits(DATA_8);
    serial_port.setStopBits(STOP_1);
    serial_port.setQueryMode(QextSerialBase::EventDriven);

    serial_port.close();

    return serial_port.open(QIODevice::ReadWrite);

}

void MainWindow::Point_MJ(uchar index)
{
    motion_done = false;

    const char a = index + 48; // + 48 will translate actual number to ASCII code number
    serial_port.write(&a, 1);
    serial_port.waitForBytesWritten(500);

}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (camera_thread.isRunning())
    {
        event->ignore();
    }
    else
    {
        event->accept();
    }

}

void MainWindow::Connect_Signals()
{
    connect(&camera_thread, SIGNAL(FrameReceived(int)), this, SLOT(On_Frame_Received(int)));
    connect(&serial_port, SIGNAL(readyRead()), this, SLOT(On_Serial_Received()));
}

void MainWindow::Process_Frame(int camera_index)
{
    // process frame here
    // frame to process is vw.internal_image_gray

    Mat image_binary;
    Mat image_display;
    Mat original_image = vw.internal_image_gray[camera_index];

    cvtColor(original_image, image_display, CV_GRAY2BGR);
    cv::threshold(original_image, image_binary, ui->Threshold->value(), 255, cv::THRESH_BINARY );

    vector<vector<Point> > contours;
    vector<vector<Point>> filtered_contours;
    vector<vector<Point>> process_contours;
    vector<Vec4i> hierarchy;

    findContours(image_binary, contours, cv::RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);

    // filter out contours based on areas

    QList<Rect> rects;
    QList<Mat> crops;

    for (int i=0; i<contours.size(); i++)
    {
        double area = contourArea(contours[i]);

        qDebug() << "Contour Area:" << i << area;

        if ((area > min_area) && (area < (min_area*1.5)))
        {
            filtered_contours.push_back(contours[i]);
            rects.append(boundingRect(contours[i]));
        }
    }

    drawContours(image_display, filtered_contours, -1, Scalar(255, 0, 0),2);

    // Crop out filtered contours and extract their descriptors

    vector<KeyPoint> keypoints;
    Mat descriptors;

    img_descriptors.clear();

    bool ok = true;

    if (filtered_contours.size() > 0)
    {
        //filtered_contours.erase(filtered_contours.begin() + 1, filtered_contours.end());

        for (int i=0; i<filtered_contours.size(); i++)
        {
            Mat contour_mask(original_image.rows, original_image.cols, CV_8UC1);

            drawContours(contour_mask, filtered_contours, i, Scalar(255,255,255), -1);

            Mat original_image_clone;

            // mask out the contour
            original_image.copyTo(original_image_clone, contour_mask);

            crops.append(original_image_clone(rects[i]));
            detector.detect(crops[i], keypoints);

            if (keypoints.size() == 0)
            {
                ok = false;
                break;
            }
            else
            {
                // prepare a mask and mask out contour

                extractor.compute(crops[i], keypoints, descriptors);
                img_descriptors.append(descriptors);
            }
        }

        if (ok)
        {

           // Perform matching

            vector<DMatch> matches;
            QList<float> distances;
            QList<QList<float>> single_distances_list;
            QList<QList<QList<float>>> distances_list;

            for (int i=0; i<filtered_contours.size(); i++)
            {
                single_distances_list.clear();

                for (int q=0; q<template_count; q++)
                {

                    matcher->match(ref_descriptors[q], img_descriptors[i], matches);
                    distances.clear();

                    for (int j=0; j<matches.size(); j++)
                    {
                        distances.append(((DMatch)matches[j]).distance);
                    }

                    qSort(distances.begin(), distances.end());

                    single_distances_list.append(distances);
                }

                distances_list.append(single_distances_list);
            }

            // Calculate average distances for each img contours

            QList<QList<float>> full_average_list;
            QList<float> individual_average_list;

            for (int i=0; i<distances_list.size(); i++)
            {
                //qDebug() << QString("Contour %1:").arg(i);

                individual_average_list.clear();

                for (int j=0; j<distances_list[i].size(); j++ )
                {
                    // qDebug() << QString("Template %1").arg(j);

                    // determine number of distances to use for averaging

                    int count = distances_list[i][j].size();

                    if (count > 20)
                    {
                        count = 20; // use the first 10 distances for averaging if there are more then 10 values
                    }

                    float temp = 0;

                    for (int k=0; k<count; k++)
                    {
                        temp += distances_list[i][j][k];
                    }

                    individual_average_list.append(temp/count);
                }

                full_average_list.append(individual_average_list);
            }

            QList<int> decisions;

            for (int i=0; i<full_average_list.size(); i++)
            {
                float min_score = 10000;
                int min_index;

                for (int j=0; j<full_average_list[i].size(); j++)
                {
                   // qDebug() << "Scores" << j << full_average_list[i][j];
                    if (full_average_list[i][j] < min_score)
                    {
                        min_score = full_average_list[i][j];
                        min_index = j;
                    }
                }

                decisions.append(min_index);
            }

            for(int i=0; i<decisions.size(); i++)
            {
                qDebug() << "Decision Contour:" << i << decisions[i] << full_average_list[i][decisions[i]];
            }

            // Draw the result on the image

             QImage qi(image_display.data, image_display.cols, image_display.rows, QImage::Format_RGB888);

             QPainter qp(&qi);
             qp.setRenderHint(QPainter::Antialiasing, 1);
             QPen qpen;
             QBrush qbrush;
             QFont font("Calibri", 50);
             font.setBold(true);
             qpen.setColor(QColor(0, 125, 255, 230));
             qpen.setWidth(4);
             qpen.setStyle(Qt::SolidLine);
             qp.setPen(qpen);
             qp.setBrush(qbrush);
             qp.setFont(font);



             for (int i=1; i<decisions.size(); i++)
             {
                 qp.drawRect(rects[i].x, rects[i].y, rects[i].width, rects[i].height );
                 qp.drawText(rects[i].x, rects[i].y + (rects[i].height/3), map[MJ_names[decisions[i]]]);
             }

             // use special color for the first contour

             qpen.setColor(QColor(80, 255, 80, 230));
             qp.setPen(qpen);

             qp.drawRect(rects[0].x, rects[0].y, rects[0].width, rects[0].height );
             qp.drawText(rects[0].x, rects[0].y + (rects[0].height/3), map[MJ_names[decisions[0]]]);

             ui->label->setPixmap(QPixmap::fromImage(qi.scaled(ui->label->width(), ui->label->height(), Qt::IgnoreAspectRatio)));

             // only serial if motion_doe is true

             if (motion_done)
             {
                qDebug() << "Point to:" << decisions[0];
                Point_MJ((uchar) decisions[0]);
             }
        }
        else
        {
            QImage qi(image_display.data, image_display.cols, image_display.rows, QImage::Format_RGB888);
            ui->label->setPixmap(QPixmap::fromImage(qi.scaled(ui->label->width(), ui->label->height(), Qt::IgnoreAspectRatio)));
        }
    }
    else
    {
        QImage qi(image_display.data, image_display.cols, image_display.rows, QImage::Format_RGB888);
        ui->label->setPixmap(QPixmap::fromImage(qi.scaled(ui->label->width(), ui->label->height(), Qt::IgnoreAspectRatio)));
    }



}

void MainWindow::On_Frame_Received(int camera_index)
{
    Process_Frame(camera_index);



    // release the thread so that the next signal can be emitted
    camera_thread.mutex.unlock();

}



void MainWindow::on_Start_clicked()
{
    camera_thread.start();

}

void MainWindow::on_Stop_clicked()
{
    camera_thread.keep_capturing = false;
}

void MainWindow::On_Serial_Received()
{
    int count = serial_port.bytesAvailable();

    qDebug() << "Bytes Available:" << count;

    QByteArray data;

    if (count == 4)
    {
        data = serial_port.read(4);

        QString s(data);

        if (s == "done")
        {
            motion_done = true;
        }

        // flush all data
         serial_port.readAll();
    }


}
