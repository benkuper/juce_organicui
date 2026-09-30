#pragma once

#include <cmath>

namespace EasingMath
{
	inline double linear(double start, double end, double weight)
	{
		return start + (end - start) * weight;
	}

	inline double sine(double weight, double start, double end, double duration,
		double frequency, double amplitude)
	{
		const double base = linear(start, end, weight);
		if (frequency == 0.0) return base;
		constexpr double twoPi = 6.283185307179586476925286766559;
		const double endPhase = duration * twoPi / frequency;
		// Remove the endpoint offset without changing integer-cycle sine curves.
		return base + amplitude * (std::sin(weight * endPhase) - weight * std::sin(endPhase));
	}

	inline double elastic(double weight, double start, double end, double duration, double frequency)
	{
		if (duration <= 0.0) return start;
		const double c4 = 10.0 * (1.0 - frequency / duration);
		const auto raw = [c4](double w)
		{
			return std::pow(2.0, -10.0 * w) * std::sin((w * 10.0 - 0.75) * c4) + 1.0;
		};
		const double rawStart = raw(0.0);
		// Remove only the endpoint error; dividing by the endpoint span becomes
		// unstable for some otherwise valid frequencies.
		const double corrected = raw(weight) - linear(rawStart, raw(1.0), weight) + weight;
		return linear(start, end, corrected);
	}

	template <typename T>
	inline T bounceOut(T x)
	{
		constexpr T d1 = static_cast<T>(7.5625);
		constexpr T n1 = static_cast<T>(2.75);

		if (x < static_cast<T>(1) / n1)
			return d1 * x * x;
		if (x < static_cast<T>(2) / n1)
		{
			const T shifted = x - static_cast<T>(1.5) / n1;
			return d1 * shifted * shifted + static_cast<T>(0.75);
		}
		if (x < static_cast<T>(2.5) / n1)
		{
			const T shifted = x - static_cast<T>(2.25) / n1;
			return d1 * shifted * shifted + static_cast<T>(0.9375);
		}
		const T shifted = x - static_cast<T>(2.625) / n1;
		return d1 * shifted * shifted + static_cast<T>(0.984375);
	}
}
