#include "BitMatrix.h"


void Faerie::BitMatrix::Transpose()
{
	if (Width != Height)
	{
		return;
	}

	for (int32 i = 0; i < Height; i++)
	{
		for (int32 j = i + 1; j < Width; j++)
		{
			const bool Temp = Get(i, j);
			Set(i, j, Get(j, i));
			Set(j, i, Temp);
		}
	}
}

void Faerie::BitMatrix::Reverse()
{
	for (int32 Row = 0; Row < Height; Row++)
	{
		for (int32 Col = 0; Col < Width / 2; Col++)
		{
			const bool Temp = Get(Col, Row);
			Set(Col, Row, Get(Width - 1 - Col, Row));
			Set(Width - 1 - Col, Row, Temp);
		}
	}
}
