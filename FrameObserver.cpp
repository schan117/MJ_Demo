#include "FrameObserver.h"

namespace AVT {
namespace VmbAPI {


void FrameObserver::FrameReceived( const FramePtr pFrame )
{
	VmbFrameStatusType eReceiveStatus;

	pFrame->GetReceiveStatus(eReceiveStatus);

	if (eReceiveStatus == VmbErrorSuccess)
	{
		//printf("callback!\n\r");

		mutex.lock();
		/*
		VmbUint32_t w, h, s;

		pFrame->GetWidth(w);
		pFrame->GetHeight(h);
		pFrame->GetImageSize(s);

		printf("Image captured with parameters:\n\r");
		printf(" Width: %d\n\r", w);
		printf(" Height %d\n\r", h);
		printf(" Size: %d\n\r", s);
		*/

		frames.append(pFrame);
		//VmbErrorType error = m_pCamera->QueueFrame(pFrame);
		//printf("error: %d\n\r", error);
			
		//is_frame_ready = true;
		mutex.unlock();

        emit CallbackFinished(index);
		
	}	
	else
	{
		printf("Frame receive status not successful!\n\r");
		mutex.lock();
		m_pCamera->QueueFrame(pFrame);
		mutex.unlock();
	}
	
}

FramePtr FrameObserver::GetFrame()
{
	   // Lock the frame queue
    mutex.lock();
    // Pop frame from queue
    FramePtr res = frames.front();
	frames.removeFirst();
    // Unlock frame queue

	//is_frame_ready = false;
	
    mutex.unlock();

	
	return res;
}

}}
