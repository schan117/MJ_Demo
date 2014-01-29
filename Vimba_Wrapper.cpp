#include "Vimba_Wrapper.h"


namespace AVT {
namespace VmbAPI {

	
Vimba_Wrapper::Vimba_Wrapper(void)
	// Get a reference to the Vimba singleton
    : m_system ( VimbaSystem::GetInstance() )
{
	m_ErrorCodeToMessage[ VmbErrorSuccess ] =           "Success.";
    m_ErrorCodeToMessage[ VmbErrorInternalFault ] =     "Unexpected fault in VmbApi or driver.";    
    m_ErrorCodeToMessage[ VmbErrorApiNotStarted ] =     "API not started.";     
    m_ErrorCodeToMessage[ VmbErrorNotFound ] =          "Not found.";
    m_ErrorCodeToMessage[ VmbErrorBadHandle ] =         "Invalid handle ";
    m_ErrorCodeToMessage[ VmbErrorDeviceNotOpen ] =     "Device not open.";
    m_ErrorCodeToMessage[ VmbErrorInvalidAccess ] =     "Invalid access.";
    m_ErrorCodeToMessage[ VmbErrorBadParameter ] =      "Bad parameter.";
    m_ErrorCodeToMessage[ VmbErrorStructSize ] =        "Wrong DLL version.";
    m_ErrorCodeToMessage[ VmbErrorMoreData ] =          "More data returned than memory provided.";
    m_ErrorCodeToMessage[ VmbErrorWrongType ] =         "Wrong type.";
    m_ErrorCodeToMessage[ VmbErrorInvalidValue ] =      "Invalid value.";
    m_ErrorCodeToMessage[ VmbErrorTimeout ] =           "Timeout.";
    m_ErrorCodeToMessage[ VmbErrorOther ] =             "TL error.";
    m_ErrorCodeToMessage[ VmbErrorResources ] =         "Resource not available.";
    m_ErrorCodeToMessage[ VmbErrorInvalidCall ] =       "Invalid call.";
    m_ErrorCodeToMessage[ VmbErrorNoTL ] =              "TL not loaded.";
    m_ErrorCodeToMessage[ VmbErrorNotImplemented ] =    "Not implemented.";
    m_ErrorCodeToMessage[ VmbErrorNotSupported ] =      "Not supported.";

	cameras_opened = 0;
	frame_ready = false;
	

}

Vimba_Wrapper::~Vimba_Wrapper(void)
{
	ShutDown();

	if (cameras_opened != 0)
	{
		delete [] cameras;
		
		delete [] FrameObservers;
	}

	
}

bool Vimba_Wrapper::Startup()
{
	VmbErrorType res;

    // Start Vimba
    res = m_system.Startup();

    if ( VmbErrorSuccess != res )
	{
        qDebug("Vimba startup not successful!");
		return false;
	}
	else
	{
        qDebug("Vimba startup successful!");
		return true;
	}

}

QString Vimba_Wrapper::ErrorCodeToMessage( VmbErrorType eErr )
{
	return m_ErrorCodeToMessage.value(eErr);
}

void Vimba_Wrapper::ShutDown()
{
	
    // Release Vimba
    m_system.Shutdown();
}

void Vimba_Wrapper::List_Cameras()
{
	std::string s;
	QString qs;

	m_system.GetCameras(camera_vector);

    qDebug("%d AVT cameras found!", camera_vector.size());

	for (int i=0; i<camera_vector.size(); i++)
	{
        qDebug("Camera %d: ", i);
		camera_vector[i]->GetID(s);
		qs = QString::fromStdString(s);
        qDebug() << "ID:" << qs;
		camera_vector[i]->GetName(s);
		qs = QString::fromStdString(s);
        qDebug() << "Name:" << qs;
		camera_vector[i]->GetModel(s);
		qs = QString::fromStdString(s);
        qDebug() << "Model:" << qs;
		camera_vector[i]->GetSerialNumber(s);
		qs = QString::fromStdString(s);
        qDebug() << "Serial Number:" << qs;
	}
}

bool Vimba_Wrapper::Open_Cameras()
{
	VmbErrorType res;

	cameras_opened = 0;

	// declare an array of FrameObserver pointers according to cameras needed to be opened
	FrameObservers = new FrameObserver*[cameras_to_open];
	internal_image_rgb = new Mat[cameras_to_open];
    internal_image_gray = new Mat[cameras_to_open];
	found_list = new int[cameras_to_open];
	cameras = new CameraPtr[cameras_to_open];
	FeaturePtr feature;

	for (int i=0; i<cameras_to_open; i++)
	{
        qDebug() << "Attempt to open:" << id_list[i];
		res = m_system.OpenCameraByID( id_list[i].toStdString().c_str(), VmbAccessModeFull, cameras[i] );

		if (res == VmbErrorSuccess)
		{
            qDebug("	Succeeded! Register frame callback for camera index: %d", i);
            FrameObservers[i] = new FrameObserver( cameras[i] );
            FrameObservers[i]->index = i;
            QObject::connect(FrameObservers[i], SIGNAL(CallbackFinished(int)), this, SLOT(On_Callback_Finished(int)));
			cameras_opened++;
			found_list[i] = 1;

			// change the camera to software to make sure that things are going to be cool

			res = cameras[i]->GetFeatureByName("TriggerSource", feature);
			res = feature->SetValue("Software");

			if (res == VmbErrorSuccess)
			{
                qDebug("	Camera changed to software trigger mode!");
			}
			else
			{
                qDebug("	Fail to chane to software trigger mode!");
			}

        }
		else
		{
            qDebug("	Failed!");
			found_list[i] = 0;
		}
	}

    qDebug("Cameras opened in total: %d", cameras_opened);

	if (cameras_opened == 0)
	{
		return false;
	}
	else
	{
		return true;
	}
}

bool Vimba_Wrapper::Start_Acquisition(int camera_index)
{
	
	VmbErrorType res;

	res = cameras[camera_index]->StartContinuousImageAcquisition( 3, IFrameObserverPtr( FrameObservers[camera_index] ));

	if (res == VmbErrorSuccess)
	{
        qDebug("Acquisition started!");
		return true;
	}
	else
	{
        qDebug("Acquisition cannot be started!");
		return false;
	}
	
}

bool Vimba_Wrapper::Stop_Acquisition(int camera_index)
{
	
	VmbErrorType res;

	res = cameras[camera_index]->StopContinuousImageAcquisition();  

	if (res == VmbErrorSuccess)
	{
        qDebug("Acquisition stopped!");
		return true;
	}
	else
	{
        qDebug("Acquisition cannot be stopped!");
		return false;
	}
	
}

bool Vimba_Wrapper::Trigger_Frame(int camera_index)
{
	FeaturePtr feature;
	VmbErrorType res;
	
	frame_ready = false;

	res = cameras[camera_index]->GetFeatureByName("TriggerSoftware", feature);
	res = feature->RunCommand();

	if (res == VmbErrorSuccess)
	{
        //printf("Trigger camera index: %d", camera_index);
	}
	else
	{
        qDebug("Cannot trigger camera index: %d", camera_index);
		return false;
	}

	return true;

}

bool Vimba_Wrapper::Set_Exposure(int camera_index, int value)
{
	FeaturePtr feature;
	VmbErrorType res;

	res = cameras[camera_index]->GetFeatureByName("ExposureTimeAbs", feature);

	if (res != VmbErrorSuccess)
	{
		return false;
	}

	res = feature->SetValue((double)value);

	if (res != VmbErrorSuccess)
	{
		return false;
	}

	return true;

}

bool Vimba_Wrapper::Transfer_Frame(int camera_index)
{	
	/*
	while (frame_ready == false)//FrameObservers[camera_index]->is_frame_ready == false)
	{
		//printf("%d ", FrameObservers[camera_index]->is_frame_ready);
		QTest::qWait(1000);
	}
	*/

	FramePtr frame = FrameObservers[camera_index]->GetFrame();
		
	VmbUint32_t w, h, s;

	frame->GetWidth(current_image_width);
	frame->GetHeight(current_image_height);
	frame->GetImage(current_image_data);

    Mat image(current_image_height, current_image_width, CV_8UC1, current_image_data);

    internal_image_gray[camera_index] = image;
	
	VmbErrorType error = cameras[camera_index]->QueueFrame(frame);
	//printf("error %d\n\r", error);
	
	return true;
}
/*
bool Vimba_Wrapper::Save_Frame(QString filename)
{
	bool ok = imwrite(filename.toAscii().constData(), internal_image_rgb);

	if (ok)
	{
		return true;
	}
	else
	{
		return false;
	}
}
*/

bool Vimba_Wrapper::Load_Settings()
{
	QSettings set("camera settings/cameras.ini", QSettings::IniFormat);

	bool ok;

	cameras_to_open = set.value("Main/Count").toInt(&ok);
	if (!ok) return false;

    qDebug("Cameras to open: %d", cameras_to_open);

	for (int i=0; i<cameras_to_open; i++)
	{
		QString s = set.value(QString("Camera%1/ID").arg(i)).toString();
		id_list.append(s);
	}

	cameras = new CameraPtr[cameras_to_open];

	return true;
}

void Vimba_Wrapper::On_Callback_Finished(int index)
{
	frame_ready = true;

    emit Frame_Received(index);
}


}}
