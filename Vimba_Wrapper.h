#pragma once

#include <VimbaCPP\Include\VimbaCPP.h>
#include <QtCore>
#include <QtGui>
#include <QtTest\qtest.h>
#include "opencv2\core\core.hpp"
#include "opencv2\imgproc\imgproc.hpp"
#include "opencv2\highgui\highgui.hpp"
#include "FrameObserver.h"


using namespace cv;

namespace AVT {
namespace VmbAPI {


class Vimba_Wrapper : public QObject
{

Q_OBJECT

public:
	Vimba_Wrapper(void);

	bool Load_Settings();

	bool Startup();
	void List_Cameras();
	void ShutDown();

	
	 bool Open_Cameras();
	 bool Start_Acquisition(int camera_index);
	 bool Stop_Acquisition(int camera_index);
	 bool Trigger_Frame(int camera_index);
	 bool Transfer_Frame(int camera_index);
	 bool Set_Exposure(int camera_index, int value);
	 //bool Save_Frame(QString filename);

	~Vimba_Wrapper(void);

	Mat* internal_image_rgb;
    Mat* internal_image_gray;
	
	VmbUint32_t current_image_width;
	VmbUint32_t current_image_height;
	unsigned char* current_image_data;

	int cameras_opened;
	int cameras_to_open;

	bool frame_ready;

	int* found_list;

public slots:

    void On_Callback_Finished(int index);

private:

	// A reference to our Vimba singleton
	VimbaSystem &m_system;
    // The currently streaming camera
	CameraPtrVector camera_vector;
	// A readable value for every Vimba error code
    QMap<VmbErrorType, QString> m_ErrorCodeToMessage;

	QString ErrorCodeToMessage( VmbErrorType eErr );
	
	QStringList id_list;
	CameraPtr* cameras;

	FramePtr frame;

	// Reserve two frame observer for now, one for front camera and one for back camera
	FrameObserver** FrameObservers;

signals:

    void Frame_Received(int index);

};

}}

