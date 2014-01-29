#pragma once

#include <qthread.h>
#include "Vimba_Wrapper.h"

class Camera_Thread : public QThread
{

Q_OBJECT

public:

	Camera_Thread(void);
	~Camera_Thread(void);

	void Initialize(AVT::VmbAPI::Vimba_Wrapper* vw);

	void run();

	int camera_index;
	
	bool keep_capturing;

	bool do_front_camera;

	bool processing_finished;

	int capture_time;

    QMutex mutex;

signals:

	void FrameReceived(int camera_index);

private:

	AVT::VmbAPI::Vimba_Wrapper* vvw;

};

