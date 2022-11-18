#include <cmath>    // IWYU pragma: keep
#include <ratio>    // for ratio
#include <utility>  // for move

#include "ftxui/component/animation.hpp"

namespace ftxui::animation::easing {

namespace {
constexpr float kPi = 3.14159265358979323846F;
constexpr float kPi2 = kPi / 2.F;
}  // namespace

// Easing function have been taken out of:
// https://github.com/warrenm/AHEasing/blob/master/AHEasing/easing.c
//
// Corresponding license:
//  Copyright (c) 2011, Auerhaus Development, LLC
//
//  This program is free software. It comes without any warranty, to
//  the extent permitted by applicable law. You can redistribute it
//  and/or modify it under the terms of the Do What The Fuck You Want
//  To Public License, Version 2, as published by Sam Hocevar. See
//  http://sam.zoy.org/wtfpl/COPYING for more details.

// Modeled after the line y = x
[[gnu::const]] float Linear(float p) {
  return p;
}

// Modeled after the parabola y = x^2
[[gnu::const]] float QuadraticIn(float p) {
  return p * p;
}

// Modeled after the parabola y = -x^2 + 2x
[[gnu::const]] float QuadraticOut(float p) {
  return -(p * (p - 2.f));
}

// Modeled after the piecewise quadratic
// y = (1/2)((2x)^2)             ; [0, 0.5)
// y = -(1/2)((2x-1)*(2x-3) - 1) ; [0.5, 1]
[[gnu::const]] float QuadraticInOut(float p) {
  if (p < 0.5f) {
    return 2.f * p * p;
  }
  return (-2.f * p * p) + (4.f * p) - 1.f;
}

// Modeled after the cubic y = x^3
[[gnu::const]] float CubicIn(float p) {
  return p * p * p;
}

// Modeled after the cubic y = (x - 1)^3 + 1
[[gnu::const]] float CubicOut(float p) {
  const float f = (p - 1.f);
  return (f * f * f) + 1.f;
}

// Modeled after the piecewise cubic
// y = (1/2)((2x)^3)       ; [0, 0.5)
// y = (1/2)((2x-2)^3 + 2) ; [0.5, 1]
[[gnu::const]] float CubicInOut(float p) {
  if (p < 0.5f) {
    return 4.f * p * p * p;
  }
  const float f = ((2.f * p) - 2.f);
  return (0.5f * f * f * f) + 1.f;
}

// Modeled after the quartic x^4
[[gnu::const]] float QuarticIn(float p) {
  return p * p * p * p;
}

// Modeled after the quartic y = 1 - (x - 1)^4
[[gnu::const]] float QuarticOut(float p) {
  const float f = (p - 1.f);
  return (f * f * f * (1.f - p)) + 1.f;
}

// Modeled after the piecewise quartic
// y = (1/2)((2x)^4)        ; [0, 0.5)
// y = -(1/2)((2x-2)^4 - 2) ; [0.5, 1]
[[gnu::const]] float QuarticInOut(float p) {
  if (p < 0.5f) {
    return 8.f * p * p * p * p;
  }
  const float f = (p - 1.f);
  return (-8.f * f * f * f * f) + 1.f;
}

// Modeled after the quintic y = x^5
[[gnu::const]] float QuinticIn(float p) {
  return p * p * p * p * p;
}

// Modeled after the quintic y = (x - 1)^5 + 1
[[gnu::const]] float QuinticOut(float p) {
  const float f = (p - 1.f);
  return (f * f * f * f * f) + 1.f;
}

// Modeled after the piecewise quintic
// y = (1/2)((2x)^5)       ; [0, 0.5)
// y = (1/2)((2x-2)^5 + 2) ; [0.5, 1]
[[gnu::const]] float QuinticInOut(float p) {
  if (p < 0.5f) {
    return 16.f * p * p * p * p * p;
  }
  const float f = ((2.f * p) - 2.f);
  return (0.5f * f * f * f * f * f) + 1.f;
}

// Modeled after quarter-cycle of sine wave
[[gnu::const]] float SineIn(float p) {
  return std::sin((p - 1.f) * kPi2) + 1.f;
}

// Modeled after quarter-cycle of sine wave (different phase)
[[gnu::const]] float SineOut(float p) {
  return std::sin(p * kPi2);
}

// Modeled after half sine wave
[[gnu::const]] float SineInOut(float p) {
  return 0.5f * (1.f - std::cos(p * kPi));
}

// Modeled after shifted quadrant IV of unit circle
float CircularIn(float p) {
  return 1 - std::sqrt(1.f - (p * p));
}

// Modeled after shifted quadrant II of unit circle
float CircularOut(float p) {
  return std::sqrt((2.f - p) * p);
}

// Modeled after the piecewise circular function
// y = (1/2)(1 - sqrt(1 - 4x^2))           ; [0, 0.5)
// y = (1/2)(sqrt(-(2x - 3)*(2x - 1)) + 1) ; [0.5, 1]
float CircularInOut(float p) {
  if (p < 0.5f) {
    return 0.5f * (1.f - std::sqrt(1.f - (4.f * (p * p))));
  }
  return 0.5f * (std::sqrt(-((2.f * p) - 3.f) * ((2.f * p) - 1.f)) + 1.f);
}

// Modeled after the exponential function y = 2^(10(x - 1))
float ExponentialIn(float p) {
  return (p == 0.f) ? p : std::pow(2.f, 10.f * (p - 1.f));
}

// Modeled after the exponential function y = -2^(-10x) + 1
float ExponentialOut(float p) {
  return (p == 1.f) ? p : (1.f - std::pow(2.f, -10.f * p));
}

// Modeled after the piecewise exponential
// y = (1/2)2^(10(2x - 1))         ; [0,0.5)
// y = -(1/2)*2^(-10(2x - 1))) + 1 ; [0.5,1]
float ExponentialInOut(float p) {
  if (p == 0.F || p == 1.F) {
    return p;
  }

  if (p < 0.5f) {
    return 0.5f * std::pow(2.f, (20.f * p) - 10.f);
  }
  return -0.5f * std::pow(2.f, (-20.f * p) + 10.f) + 1.f;
}

// Modeled after the damped sine wave y = sin(13pi/2*x)*pow(2, 10 * (x - 1))
float ElasticIn(float p) {
  return std::sin(13.f * kPi2 * p) * std::pow(2.f, 10.f * (p - 1.f));
}

// Modeled after the damped sine wave y = sin(-13pi/2*(x + 1))*pow(2, -10x) +
// 1
float ElasticOut(float p) {
  return std::sin(-13.f * kPi2 * (p + 1.f)) * std::pow(2.f, -10.f * p) + 1.f;
}

// Modeled after the piecewise exponentially-damped sine wave:
// y = (1/2)*sin(13pi/2*(2*x))*pow(2, 10 * ((2*x) - 1))      ; [0,0.5)
// y = (1/2)*(sin(-13pi/2*((2x-1)+1))*pow(2,-10(2*x-1)) + 2) ; [0.5, 1]
float ElasticInOut(float p) {
  if (p < 0.5f) {
    return 0.5f * std::sin(13.f * kPi2 * (2.f * p)) *
           std::pow(2.f, 10.f * ((2.f * p) - 1.f));
  }
  return 0.5f * (std::sin(-13.f * kPi2 * (((2.f * p) - 1.f) + 1.f)) *
                    std::pow(2.f, -10.f * ((2.f * p) - 1.f)) +
                2.f);
}

// Modeled after the overshooting cubic y = x^3-x*sin(x*pi)
[[gnu::const]] float BackIn(float p) {
  return (p * p * p) - (p * std::sin(p * kPi));
}

// Modeled after overshooting cubic y = 1-((1-x)^3-(1-x)*sin((1-x)*pi))
[[gnu::const]] float BackOut(float p) {
  const float f = (1.f - p);
  return 1.f - ((f * f * f) - (f * std::sin(f * kPi)));
}

// Modeled after the piecewise overshooting cubic function:
// y = (1/2)*((2x)^3-(2x)*sin(2*x*pi))           ; [0, 0.5)
// y = (1/2)*(1-((1-x)^3-(1-x)*sin((1-x)*pi))+1) ; [0.5, 1]
[[gnu::const]] float BackInOut(float p) {
  if (p < 0.5f) {
    const float f = 2.f * p;
    return 0.5f * (f * f * f - f * std::sin(f * kPi));
  }
  const float f = (1 - (2 * p - 1));
  return 0.5f * (1 - (f * f * f - f * std::sin(f * kPi))) + 0.5f;
}

[[gnu::const]] float BounceOut(float p) {
  if (p < 4.f / 11.f) {
    return (121.f * p * p) / 16.f;
  } else if (p < 8.f / 11.f) {
    return (363.f / 40.f * p * p) - (99.f / 10.f * p) + 17.f / 5.f;
  } else if (p < 9.f / 10.f) {
    return (4356.f / 361.f * p * p) - (35442.f / 1805.f * p) + 16061.f / 1805.f;
  }
  return (54.f / 5.f * p * p) - (513.f / 25.f * p) + 268.f / 25.f;
}

[[gnu::const]] float BounceIn(float p) {
  return 1.f - BounceOut(1.f - p);
}

[[gnu::const]] float BounceInOut(float p) {
  if (p < 0.5f) {
    return 0.5f * BounceIn(p * 2.f);
  }
  return 0.5f * BounceOut(p * 2.f - 1.f) + 0.5f;
}

}  // namespace ftxui::animation::easing
