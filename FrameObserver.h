#pragma once

#include <VimbaCPP/Include/VimbaCPP.h>
#include <qobject.h>
#include <qmutex.h>

namespace AVT {
namespace VmbAPI {

class FrameObserver : public QObject, virtual public IFrameObserver
{

Q_OBJECT

public:
	FrameObserver( CameraPtr pCamera ) : IFrameObserver( pCamera ) 
	{
		//is_frame_ready = false;	
	}
    
    // This is our callback routine that will be executed on every received frame
    virtual void FrameReceived( const FramePtr pFrame );

    // After the view has been notified about a new frame it can pick it up
    FramePtr GetFrame();

    // Clears the double buffer frame queue
    void ClearFrameQueue();

	//bool is_frame_ready;

    int index;

signals:

    void CallbackFinished(int index);

private:

	QList<FramePtr> frames;
	QMutex mutex;

};

}}

