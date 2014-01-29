#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "Vimba_Wrapper.h"
#include "Camera_Thread.h"
#include "opencv2/core/core.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/nonfree/features2d.hpp"
#include "opencv2/features2d/features2d.hpp"
#include "opencv2/flann/flann.hpp"
#include <QMap>
#include "qextserialport.h"


namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();


    AVT::VmbAPI::Vimba_Wrapper vw;

    bool Initialize_Cameras();
    bool Load_Templates();
    void Setup_Lookup_Table();
    bool Setup_Serial_Port();

    void Point_MJ(uchar index);


protected:

    void closeEvent(QCloseEvent* event);



private:
    Ui::MainWindow *ui;

    void Connect_Signals();

    Camera_Thread camera_thread;

    QImage image_for_process;

    int template_count;
    Mat* template_images;

    QStringList MJ_names;

    SurfFeatureDetector detector;
    SurfDescriptorExtractor extractor;
    FlannBasedMatcher* matcher;

    QList<Mat> ref_descriptors;
    QList<Mat> img_descriptors;


    // vision variables

    int hessian_threshold;
    int threshold;
    int min_area;

    void Process_Frame(int camera_index);

    QMap<QString, QString> map;

    QextSerialPort serial_port;

    bool motion_done;

private slots:

    void On_Frame_Received(int camera_index);

    void on_Start_clicked();

    void on_Stop_clicked();

    void On_Serial_Received();
};

#endif // MAINWINDOW_H
