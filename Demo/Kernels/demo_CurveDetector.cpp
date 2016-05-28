//@file demo_CurveDetector.h
//@brief Contains demonstration of CurveDetector function in comparing with OpenCV
//@author Anton Shutikhin
//@date 28 May 2016

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
	static void applyParameters(void* data);

private:
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
	cv::namedWindow(m_openCVWindow, CV_WINDOW_NORMAL);
	cv::namedWindow(m_openVXWindow, CV_WINDOW_NORMAL);
	cv::namedWindow(m_diffWindow, CV_WINDOW_NORMAL);

	const std::string imgPath = "..\\Image\\test.png";
	m_srcImage = cv::imread(imgPath, CV_LOAD_IMAGE_GRAYSCALE);
	cv::imshow(m_originalWindow, m_srcImage);

	applyParameters(this);

	cv::waitKey(0);
}

///////////////////////////////////////////////////////////////////////////////
void demo_CurveDetector::applyParameters(void* data)
{
	auto demo = static_cast<demo_CurveDetector*>(data);
	cv::Canny(demo->m_srcImage, demo->m_srcImage, 100, 200);

	const cv::Size imgSize(demo->m_srcImage.cols, demo->m_srcImage.rows);
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

	uint32_t** Curve;
	Curve = static_cast<uint32_t**>(calloc(imgSize.width*imgSize.height * 3/100 * sizeof(uint32_t*), sizeof(uint32_t*)));
	ref_CurveDetector(&srcVXImage, &dstVXImage, Curve);

	cv::imshow(m_openCVWindow, demo->m_srcImage);
	const cv::Mat vxImage = cv::Mat(imgSize, CV_8UC1, outVXImage);
	cv::imshow(m_openVXWindow, vxImage);

	const cv::Mat diffImage(imgSize, CV_8UC1);
	cv::absdiff(vxImage, demo->m_srcImage, diffImage);
	cv::imshow(m_diffWindow, diffImage);
	
	free(Curve);
}

///////////////////////////////////////////////////////////////////////////////
IDemoCasePtr CreateCurveDetectorDemo()
{
	return std::make_unique<demo_CurveDetector>();
}
