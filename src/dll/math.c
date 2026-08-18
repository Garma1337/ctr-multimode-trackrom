#include "math.h"

int Math_Clamp(int value, int min, int max)
{
	if (value < min) { return min; }
	if (value > max) { return max; }

	return value;
}

int Math_Wrap(int value, int count)
{
	if (count <= 0)
	{
		return 0;
	}

	value %= count;

	if (value < 0)
	{
		value += count;
	}

	return value;
}

int Math_Scale(int value, int total, int range)
{
	if (total <= 0)
	{
		return 0;
	}

	return Math_Clamp((value * range) / total, 0, range);
}

int Math_Percent(int value, int total)
{
	if (total <= 0)
	{
		return 0;
	}

	return (value * 100) / total;
}
