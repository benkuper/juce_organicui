#pragma once

#include <cmath>

namespace AutomationKeySearch
{
	template <typename PositionAt>
	int previous(int count, double pos, bool trueIfEqual, PositionAt positionAt)
	{
		if (count == 0 || std::isnan(pos)) return -1;
		if (pos < positionAt(0) || pos == 0.0) return 0;
		int low = 0;
		int high = count;
		while (low < high)
		{
			const int mid = low + (high - low) / 2;
			const double p = positionAt(mid);
			if (p < pos || (trueIfEqual && p == pos)) low = mid + 1;
			else high = mid;
		}
		return low > 0 ? low - 1 : -1;
	}

	template <typename PositionAt>
	int next(int count, double pos, bool trueIfEqual, PositionAt positionAt)
	{
		if (count == 0 || std::isnan(pos)) return -1;
		if (pos < positionAt(0) || pos == 0.0) return 0;
		int low = 0;
		int high = count;
		while (low < high)
		{
			const int mid = low + (high - low) / 2;
			const double p = positionAt(mid);
			if (p < pos || (!trueIfEqual && p == pos)) low = mid + 1;
			else high = mid;
		}
		return low < count ? low : -1;
	}
}
