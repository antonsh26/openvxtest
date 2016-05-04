//@file demo_CurveDetector.h
//@brief Contains demonstration of CurveDetector function in comparing with OpenCV
//@author Anton Shutikhin
//@date 23 April 2016

#include "../stdafx.h"

#include <opencv2/opencv.hpp>

extern "C"
{
#include "Lib/Kernels/ref.h"
#include "Lib/Common/types.h"
}

#include "../DemoEngine.h"

///////////////////////////////////////////////////////////////////////////////
//@brief Demonstration of CurveDetector function
class demo_CurveDetector : public IDemoCase
{
public:
	///@brief default ctor
	demo_CurveDetector()
		: m_curveDetector(127)
	{
		// nothing to do
	}

	///@see IDemoCase::ReplyName
	virtual std::string ReplyName() const override
	{
		return "CurveDetector";
	}

private:
	///@see IDemoCase::execute
	virtual void execute() override;

	///@brief provide interactive demo
	static void applyParameters(int pos, void* data);

private:
	int m_curveDetector;
	cv::Mat m_srcImage;
};

///////////////////////////////////////////////////////////////////////////////
namespace
{
	const std::string m_openVXWindow = "openVX";
	const std::string m_openCVWindow = "openCV";
	const std::string m_originalWindow = "original";
	const std::string m_diffWindow = m_openVXWindow + "-" + m_openCVWindow;
}

///////////////////////////////////////////////////////////////////////////////
void demo_CurveDetector::execute()
{
	cv::namedWindow(m_originalWindow, CV_WINDOW_NORMAL);
	cv::namedWindow(m_openVXWindow, CV_WINDOW_NORMAL);
	//cv::namedWindow(m_openCVWindow, CV_WINDOW_NORMAL);
	cv::namedWindow(m_diffWindow, CV_WINDOW_NORMAL);

	const std::string imgPath = "..\\Image\\curve.png";
	m_srcImage = cv::imread(imgPath, CV_LOAD_IMAGE_GRAYSCALE);
	cv::imshow(m_originalWindow, m_srcImage);

	//cv::createTrackbar("CurveDetector:", m_originalWindow, &m_curveDetector, 255, applyParameters, static_cast<void*>(this));
	applyParameters(m_curveDetector, this);

	cv::waitKey(0);
}

///////////////////////////////////////////////////////////////////////////////
void demo_CurveDetector::applyParameters(int, void* data)
{
	auto demo = static_cast<demo_CurveDetector*>(data);

	const cv::Size imgSize(demo->m_srcImage.cols, demo->m_srcImage.rows);
	///@{ OPENCV
	cv::Mat cvImage = demo->m_srcImage;
	//cv::Canny(demo->m_srcImage, cvImage, 50, 200);
	//cv::imshow(m_openCVWindow, cvImage);
	///@}

	///@{ OPENVX

	_vx_image srcVXImage = {
		demo->m_srcImage.data,
		imgSize.width,
		imgSize.height,
		VX_DF_IMAGE_U8,
		VX_COLOR_SPACE_DEFAULT
	};

	uint8_t* outVXImage = static_cast<uint8_t*>(calloc(imgSize.width* imgSize.height, sizeof(uint8_t)));

	_vx_image dstVXImage = {
		outVXImage,
		imgSize.width,
		imgSize.height,
		VX_DF_IMAGE_U8,
		VX_COLOR_SPACE_DEFAULT
	};


	ref_CurveDetector(&srcVXImage, &dstVXImage);
	
	const cv::Mat vxImage = cv::Mat(imgSize, CV_8UC1, outVXImage);
	cv::imshow(m_openVXWindow, vxImage);
	///@}
	
	// Show difference of OpenVX and OpenCV
	const cv::Mat diffImage(imgSize, CV_8UC1);
	cv::absdiff(vxImage, cvImage, diffImage);
	cv::imshow(m_diffWindow, diffImage);
}

///////////////////////////////////////////////////////////////////////////////
IDemoCasePtr CreateCurveDetectorDemo()
{
	return std::make_unique<demo_CurveDetector>();
}
