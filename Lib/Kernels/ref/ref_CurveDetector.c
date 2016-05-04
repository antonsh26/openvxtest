/*
File: ref_CurveDetector.c

Author: Шутихин Антон

Date: 01 Мая 2016
*/

#include "../ref.h"

int32_t Neighbors(const uint8_t i, const uint32_t point, const uint32_t width) //передает индекс соседней точки по параметру от 0 до 7
{
	// 0 1 2
	// 7 * 3
	// 6 5 4
	bool left = (point%width) == 0; // точка на левой границе
	bool right = (point%width) == width - 1; // точка на правой границе
	// у них некоторые соседние точки выходят за пределы => левые и правые соответственно
	switch (i)
	{
	case 0:
		if (left) return -1;
		else return point - width - 1;
	case 1:
		return point - width;
	case 2:
		if (right) return -1;
		else return point - width + 1;
	case 3:
		if (right) return -1;
		else return point + 1;
	case 4:
		if (right) return -1;
		else return point + width + 1;
	case 5:
		return point + width;
	case 6:
		if (left) return -1;
		else return point + width - 1;
	case 7:
		if (left) return -1;
		else return point - 1;
	default:
		return -1;
	}
}

uint8_t CountNeighbors(vx_image dst_image, const uint32_t point)//находит число соседних точек с данной
{
	uint8_t* dst_data = dst_image->data;
	const uint32_t dst_width = dst_image->width;
	const uint32_t dst_height = dst_image->height;

	uint8_t nNeighbors = 0;
	int32_t currentPoint = 0;
	int32_t hlim = dst_width*dst_height;
	for (uint8_t ind = 0; ind < 8; ++ind)
	{
		currentPoint = Neighbors(ind, point, dst_width);
		if ((currentPoint < 0) || (currentPoint > hlim))
			continue;
		if (dst_data[currentPoint] == 255)
		{
			++nNeighbors;
		}
	}
	return nNeighbors;
}

void ThinEdge(vx_image dst_image) //удаляет точки, являющиеся углами
{
	uint8_t* dst_data = dst_image->data;
	uint32_t hlim = dst_image->width*dst_image->height;
	int32_t currentPoint1 = 0;
	int32_t currentPoint2 = 0;
	int32_t currentPoint3 = 0;
	uint8_t counter = 1;

	for (uint32_t ind_w = 0; ind_w <= hlim; ++ind_w)
	{
		for (uint8_t i = 0; i < 4; ++i)
		{
			currentPoint1 = Neighbors(counter, ind_w, dst_image->width);
			counter = (counter + 2) % 8;
			currentPoint2 = Neighbors(counter, ind_w, dst_image->width);
			currentPoint3 = Neighbors((counter + 3) % 8, ind_w, dst_image->width);
			if ((currentPoint1 < 0) || (currentPoint2 < 0) || (currentPoint1 >(int32_t)hlim) || (currentPoint2 >(int32_t)hlim))
				continue;
			if ((dst_data[currentPoint1] == 255) && (dst_data[currentPoint2] == 255) && (dst_data[currentPoint3] == 0))
			{
				dst_data[ind_w] = 0;
				break;
			}

		}
	}
}

uint32_t GetNextDirection(vx_image dst_image, const uint32_t currentPoint, const int32_t prevPoint) //выбирает следующую точку кривой
{
	uint8_t* dst_data = dst_image->data;
	uint32_t hlim = dst_image->width*dst_image->height;
	int32_t nextPoint = 0;
	for (uint8_t ind = 0; ind < 8; ++ind)
	{
		nextPoint = Neighbors(ind, currentPoint, dst_image->width);
		if ((nextPoint >= 0) && (nextPoint < (int32_t)hlim))
		{
			if ((dst_data[nextPoint] == 255) && (nextPoint != prevPoint) )
				return (uint32_t)nextPoint;

		}
	}
	return currentPoint;
}


void FindEndCurve(vx_image dst_image, uint32_t* endPoint, bool* Marked) //находит конечную точку кривой
{
	uint32_t startPoint = endPoint[0]; //стартовая точка
	uint8_t nNeighbors = CountNeighbors(dst_image, startPoint); // число соседей стартовой точки
	switch (nNeighbors)
	{
	case 0: //нет соседей - точка сама конец кривой
	{
			  endPoint[0] = startPoint;
			  endPoint[1] = startPoint;
			  break;
	}
	default: 
	{
			  uint32_t prevPoint = startPoint;
			  uint32_t currentPoint = startPoint;
			  uint32_t nextPoint = GetNextDirection(dst_image, startPoint, -1); //следующая точка
			  while ((nextPoint != startPoint) && (nextPoint != currentPoint) && (Marked[nextPoint] == false)) //если она отличается от текущей и не отмечена
			  {
				  prevPoint = currentPoint;
				  currentPoint = nextPoint;
				  nextPoint = GetNextDirection(dst_image, currentPoint, prevPoint); //переход к следующей точке
			  }
			  endPoint[0] = nextPoint; //конечная точка кривой
			  endPoint[1] = currentPoint; //предшествующая ей
			  break;
	}
	}
}

void TraceCurve(vx_image dst_image, uint32_t* Point, uint32_t* Curve, bool* Marked)
{
	// bool* Marked - указывает на массив отмеченных/не отмеченных точек
	// uint32_t* Curve - указывает на массив точек кривой, выходной параметр
	// uint32_t* Point - указывает на массив из двух точек - конечные точки кривой, идем в другом направлении => теперь они начальные
	for (uint8_t ind = 0; ind < 2; ++ind)
		Curve[ind] = Point[ind]; // 2 начальные точки
	uint32_t counter = 2;
	uint32_t startPoint[2] = {0}; // инициализируется массив из двух точек
	startPoint[0] = Point[0]; // начальная точка
	FindEndCurve(dst_image, startPoint, Marked); // результат: startPoint[0] - конец кривой, startPoint[1] - предыдущая точка
	uint32_t nextPoint = startPoint[0] + 1;
	while (nextPoint != startPoint[0])
	{
		nextPoint = GetNextDirection(dst_image, Point[1], Point[0]); // следующая точка
		Curve[counter++] = nextPoint; // точка записывается в массив точек кривой
		Point[0] = Point[1];
		Point[1] = nextPoint; // сохраняем следующую точку
		Marked[nextPoint] = true; // точка отмечается как использованная
	}
}



vx_status ref_CurveDetector(vx_image src_image, vx_image dst_image)
{

	const uint32_t src_width = src_image->width;
	const uint32_t src_height = src_image->height;
	const uint32_t dst_width = dst_image->width;
	const uint32_t dst_height = dst_image->height;

	if (src_width != dst_width || src_height != dst_height)
	{
		return VX_ERROR_INVALID_PARAMETERS;
	}

	uint8_t* dst_data = dst_image->data;
	const uint8_t* src_data = src_image->data;

	for (uint32_t ind_w = 0; ind_w <= src_width*src_height; ++ind_w)
	{
		dst_data[ind_w] = 0;
	}
	ThinEdge(src_image);
	uint32_t Point[2] = { 0 };
	uint32_t Curve[100000] = { 0 };
	bool Marked[500000] = { false };
	for (uint32_t ind_w = 0; ind_w <= src_width*src_height; ++ind_w)
	{
		if ( (src_data[ind_w] == 255)  && (Marked[ind_w] == false) )
		{
			Point[0] = ind_w;
			Point[1] = 0;
			FindEndCurve(src_image, Point, Marked);
			TraceCurve(src_image, Point, Curve, Marked);

			for (uint32_t d = 0; d < 1000; ++d)
			{
				if (Curve[d] != 0)
					dst_data[Curve[d]] = 255;
				Curve[d] = 0;
			}
		}
	}
	return VX_SUCCESS;
}
