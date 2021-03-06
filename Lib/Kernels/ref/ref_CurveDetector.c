/*
File: ref_CurveDetector.c

Author: Shutihin Anton

Date: 28 May 2016
*/

#include "../ref.h"

//@brief function returns the coordinates of a neighboring pixel by number
int32_t Neighbors(const uint8_t i, const uint32_t point, const uint32_t width)
{
	// 0 1 2
	// 7 * 3
	// 6 5 4
	bool left_border = (point%width) == 0; //if pixel on the left border
	bool right_border = (point%width) == width - 1; //if pixel on the right border
	switch (i)
	{
	case 0:
		if (left_border) return -1;
		else return point - width - 1;
	case 1:
		return point - width;
	case 2:
		if (right_border) return -1;
		else return point - width + 1;
	case 3:
		if (right_border) return -1;
		else return point + 1;
	case 4:
		if (right_border) return -1;
		else return point + width + 1;
	case 5:
		return point + width;
	case 6:
		if (left_border) return -1;
		else return point + width - 1;
	case 7:
		if (left_border) return -1;
		else return point - 1;
	default:
		return -1;
	}
}

//@brief function returns the count of neighbors of pixel
uint8_t CountNeighbors(vx_image image, const uint32_t point)
{
	uint8_t* data = image->data;
	const uint32_t width = image->width;
	const uint32_t height = image->height;
	uint8_t nNeighbors = 0;
	int32_t currentPoint = 0;
	int32_t hlim = width*height;
	for (uint8_t ind = 0; ind < 8; ++ind)
	{
		currentPoint = Neighbors(ind, point, width);
		if ((currentPoint < 0) || (currentPoint >= hlim)) 
			continue; // if it's going beyond the image
		if (data[currentPoint] == 255)
		{
			++nNeighbors;
		}
	}
	return nNeighbors;
}

//@brief function removes the corners on the curves of the edge image
void ThinEdge(vx_image image)
{
	uint8_t* data = image->data;
	int32_t hlim = image->width*image->height;
	int32_t currentPoint1 = 0;
	int32_t currentPoint2 = 0;
	int32_t currentPoint3 = 0;
	uint8_t counter = 1;
	for (int32_t ind_w = 0; ind_w < hlim; ++ind_w)
	{
		if (CountNeighbors(image, ind_w) >= 2)
		{
			for (uint8_t i = 0; i < 4; ++i)
			{
				currentPoint1 = Neighbors(counter, ind_w, image->width);
				counter = (counter + 2) % 8;
				currentPoint2 = Neighbors(counter, ind_w, image->width);
				currentPoint3 = Neighbors((counter + 3) % 8, ind_w, image->width);
				if ((currentPoint1 < 0) || (currentPoint2 < 0) || (currentPoint1 >= hlim) || (currentPoint2 >= hlim))
					continue;
				if ((data[currentPoint1] == 255) && (data[currentPoint2] == 255))
				{
					if (currentPoint3 >= 0 && currentPoint3 < hlim)
					{
						if (data[currentPoint3] == 0)
						{
							data[ind_w] = 0;
							break;
						}
							
					}
					else
					{
						data[ind_w] = 0;
						break;
					}	
				}
			}
		}	
	}
}

/*@brief function returns the next pixel of the curve
next pixel must be non-zero, not equal to the previous one, not marked; otherwise, it returns the current pixel*/
uint32_t GetNextDirection(vx_image image, const uint32_t currentPoint, const int32_t prevPoint, uint8_t* Marked)
{
	uint8_t* data = image->data;
	int32_t hlim = image->width*image->height;
	int32_t nextPoint = 0;
	for (uint8_t ind = 0; ind < 8; ++ind)
	{
		nextPoint = Neighbors(ind, currentPoint, image->width);
		if (nextPoint >= 0 && nextPoint < hlim)
		{
			if (data[nextPoint] == 255 && nextPoint != prevPoint && Marked[nextPoint] == 0)
				return (uint32_t)nextPoint;	
		}
	}
	return currentPoint;
}

//@brief function returns the end of the curve
void FindEndCurve(vx_image image, uint32_t* endPoint, uint8_t* Marked)
{
	uint8_t nNeighbors = CountNeighbors(image, endPoint[0]);
	switch (nNeighbors)
	{
	case 0: 
	{
				endPoint[1] = endPoint[0];
				break;
	}
	default:
	{
			   uint32_t prevPoint = endPoint[1];
			   uint32_t currentPoint = endPoint[0];
			   uint32_t nextPoint = GetNextDirection(image, endPoint[0], endPoint[1], Marked);
			   uint32_t addPoint = image->width*image->height;
			   // addPoint - additional pixel to avoid looping
			   while (nextPoint != endPoint[0] && nextPoint != currentPoint)
			   {
				   prevPoint = currentPoint;
				   currentPoint = nextPoint;
				   nextPoint = GetNextDirection(image, currentPoint, prevPoint, Marked);
				   if (nextPoint == addPoint)
				   {
					   nextPoint = currentPoint;
				   }
				   if (CountNeighbors(image, nextPoint) > 2)
				   {
					   uint32_t addnPoint = 0;
					   for (uint8_t ind = 0; ind < 8; ++ind)
					   {
						   addnPoint = Neighbors(ind, nextPoint, image->width);
						   if (addnPoint == addPoint)
							   break;
						   else 
						   if (ind == 7) addPoint = nextPoint;

					   }
				   }
				   
			   }
			   endPoint[0] = nextPoint; //the end of the curve
			   endPoint[1] = prevPoint; //pixel is preceding the end
	}
	}
}

//@brief curve tracking function from the set point to the nearest end. Tracked curve marked on the output image.
void TraceCurve(vx_image image, uint32_t* Point, uint32_t* Curves, uint8_t* Marked)
{
	FindEndCurve(image, Point, Marked);
	for (uint8_t ind = 0; ind < 2; ++ind)
	{
		Curves[ind] = Point[ind];
	}
	uint32_t counter = 1;
	uint32_t nextPoint = Point[1];
	Marked[Curves[0]] = 255;
	while (nextPoint != Point[0])
	{
		Curves[counter++] = nextPoint;
		Marked[nextPoint] = 255;
		nextPoint = GetNextDirection(image, Point[1], Point[0], Marked);
		Point[0] = Point[1];
		Point[1] = nextPoint;
	}
}

//@brief main function
vx_status ref_CurveDetector(vx_image src_image, vx_image dst_image, uint32_t** Curves)
{
	const uint32_t src_width = src_image->width;
	const uint32_t src_height = src_image->height;
	const uint32_t dst_width = dst_image->width;
	const uint32_t dst_height = dst_image->height;
	if (src_width != dst_width || src_height != dst_height)
	{
		return VX_ERROR_INVALID_PARAMETERS;
	}

	uint8_t* src_data = src_image->data;
	uint8_t* dst_data = dst_image->data;
	for (uint32_t ind_w = 0; ind_w <= dst_width*dst_height; ++ind_w)
	{
		dst_data[ind_w] = 0;
	}

	ThinEdge(src_image);
	
	const uint32_t points = src_width*src_height*3/100;
	uint8_t nCurve = 0;
	uint32_t Point[2] = { 0 };
	for (uint32_t ind_w = 0; ind_w < src_width*src_height; ++ind_w)
	{
		if ((src_data[ind_w] == 255) && (dst_data[ind_w] == 0))
		{
			Point[0] = ind_w;
			Point[1] = ind_w;
			Curves[nCurve] = (uint32_t*)calloc(points * sizeof(uint32_t), sizeof(uint32_t));
			TraceCurve(src_image, Point, Curves[nCurve], dst_data);
			nCurve++;
		}
	}
	//clearing the output image
	for (uint32_t ind_w = 0; ind_w < dst_width*dst_height; ++ind_w)
		dst_data[ind_w] = 0;

	//displaying of curves on the output image
	for (uint32_t i = 0; i < nCurve; i++)
	{
		for (uint32_t j = 0; j < points; j++)
			dst_data[Curves[i][j]] = 255;
	}

	return VX_SUCCESS;
}
