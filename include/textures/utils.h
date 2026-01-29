#ifndef UTILS_H
#define UTILS_H

#include <cmath>
#include <limits>
#include <random>


const double infinity = std::numeric_limits<double>::infinity();
const double pi = 3.14159265358979323846;


inline double degrees_to_radians(double degrees) {
    return degrees * pi / 180.0;
}

inline double radians_to_degrees(double radians) {
    return radians * 180.0 / pi;
}

inline double clamp(double x, double low, double high) {
    if (x < low) return low;
    if (x > high) return high;
    return x;
}


inline double random_double() {
    static std::uniform_real_distribution<double> distribution(0.0, 1.0);
    static std::mt19937 generator;
    return distribution(generator);
}

inline double random_double(double min, double max) {
    return min + (max - min) * random_double();
}

#endif
