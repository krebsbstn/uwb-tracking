#pragma once

/**
 * @file datatypes.h
 * @brief Defines custom data types used in the UWB device code.
 */

#define NUM_LANDMARKS 5

/* Define datatype for UWB addresses */
typedef long long uwb_addr;

/* Define datatype for Coordinates */
struct coordinate
{
    double x; /**< X-coordinate value */
    double y; /**< Y-coordinate value */
    double z; /**< Z-coordinate value */
};
