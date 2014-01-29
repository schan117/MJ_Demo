#include "Camera_Thread.h"


Camera_Thread::Camera_Thread(void)
{
	keep_capturing = false;
	camera_index = 0;
    processing_finished = true;
    do_front_camera = false;
}


Camera_Thread::~Camera_Thread(void)
{

}

void Camera_Thread::Initialize(AVT::VmbAPI::Vimba_Wrapper* vw)
{
	vvw = vw;
}

void Camera_Thread::run()
{
	keep_capturing = true;

	QTime qtime;

    QEventLoop q;

	qtime.start();

    do_front_camera = true;

	while (keep_capturing)
	{
        qtime.restart();

		if (do_front_camera)
		{	

            mutex.lock();

			vvw->Trigger_Frame(camera_index);


            // block thread until frame is ready
            connect(vvw, SIGNAL(Frame_Received(int)), &q, SLOT(quit()));
            q.exec();
            disconnect(vvw, SIGNAL(Frame_Received(int)), &q, SLOT(quit()));

            vvw->Transfer_Frame(camera_index);
			emit FrameReceived(camera_index);
        }

		capture_time = qtime.elapsed();

		// wait unit the main thread has finished processing the previous frame before emitting new signal
			
		// toggle flag immediately so that the next loop will be block if image comes faster than the processing

        /*mutex.lock();
        processing_finished = false;
        mutex.unlock();
        */
	}
}
