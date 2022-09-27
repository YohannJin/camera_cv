#include <iostream>
#include <pcl/io/pcd_io.h>
#include <pcl/point_types.h>
#include <pcl/filters/voxel_grid.h>
#include <pcl/visualization/cloud_viewer.h>
#include <pcl/visualization/pcl_visualizer.h>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/calib3d/calib3d.hpp>
#include "stdlib.h"
#include <fstream>
using namespace std;
using namespace cv;

//need to preconfigure the h file directory 
#include <GalaxyIncludes.h>
 
int main()
{
	IGXFactory::GetInstance().Init();
	//
	GxIAPICPP::gxdeviceinfo_vector vectorDeviceInfo;
	IGXFactory::GetInstance().UpdateDeviceList(1000, vectorDeviceInfo);
	if (vectorDeviceInfo.size() > 0)
	{
		//open device
		CGXDevicePointer ObjDevicePtr = IGXFactory::GetInstance().OpenDeviceBySN(
			vectorDeviceInfo[0].GetSN(),
			GX_ACCESS_EXCLUSIVE);
		CGXStreamPointer ObjStreamPtr = ObjDevicePtr->OpenStream(0);
		//start camera stream
		ObjStreamPtr->StartGrab();
		//ask the device to get frame
		CGXFeatureControlPointer objFeatureControlPtr = ObjDevicePtr->GetRemoteFeatureControl();
		objFeatureControlPtr->GetCommandFeature("AcquisitionStart")->Execute();

        //并不需要用到此循环其实。。。它回调采集内部就是已经做成循环了
		for (int i = 0; i < 5; i++)
		{
			//sample single frame
			CImageDataPointer objImageDataPtr;
			objImageDataPtr = ObjStreamPtr->GetImage(500);//500ms timeout
			if (objImageDataPtr->GetStatus() == GX_FRAME_STATUS_SUCCESS)
			{
				//got a frame successfully, ready for image processing
				cout << "got a single frame!" << endl;
				cout << "ImageInfo: " << objImageDataPtr->GetStatus() << endl;
				cout << "ImageInfo: " << objImageDataPtr->GetWidth() << endl;
				cout << "ImageInfo: " << objImageDataPtr->GetHeight() << endl;
				cout << "ImageInfo: " << objImageDataPtr->GetPayloadSize() << endl;

				void* pRaw8Buffer = NULL;
				//for mono8 raw image
				pRaw8Buffer = objImageDataPtr->ConvertToRaw8(GX_BIT_0_7);
				Mat curr(objImageDataPtr->GetHeight(), objImageDataPtr->GetWidth(), CV_8UC1);
				memcpy(curr.data, pRaw8Buffer, (objImageDataPtr->GetHeight())*(objImageDataPtr->GetWidth()));
				char st[30];
				sprintf_s(st, 30, "%s%d%s", "./photo/", i, ".bmp");
				imwrite(st, curr);
				waitKey(15);
			}
		}
		//stop sampling
		objFeatureControlPtr->GetCommandFeature("AcquisitionStop")->Execute();
		ObjStreamPtr->StopGrab();
		//close stream
		ObjStreamPtr->Close();
	}
 
	IGXFactory::GetInstance().Uninit();
	system("pause");
	return 0;
}