/* VectorNav Programming Library v1.1.0.115
 * Copyright (c) 2016 VectorNav Technologies, LLC */
/** \file
 * {COMMON_HEADER}
 *
 * \section Description
 * This header file contains declarations for the math types used by the
 * VectorNav library.
 */
#ifndef _VNMATH_H_
#define _VNMATH_H_

#ifdef __cplusplus
extern "C" {
#endif

/** \brief Represents a 3x3 matrix with an underlying data type of
 *  <c>float</c>. */
typedef struct
{
	union
	{
		float e[3*3];	/**< The matrix's elements in column-major ordering. */

		struct
		{
			float e00;	/**< Element [0,0]. */
			float e10;	/**< Element [1,0]. */
			float e20;	/**< Element [2,0]. */
			float e01;	/**< Element [0,1]. */
			float e11;	/**< Element [1,1]. */
			float e21;	/**< Element [2,1]. */
			float e02;	/**< Element [0,2]. */
			float e12;	/**< Element [1,2]. */
			float e22;	/**< Element [2,2]. */
		};
	};

} mat3f;

/** \brief Represents a yaw, pitch, roll reading with underlying data type of
 *  <c>float</c>.
 *
 * The units used for expressing the yaw, pitch, roll is dependent on the
 * context this struct is used in.
 */
typedef struct
{
	union
	{
		/** The components. */
		float c[3];

		struct
		{
			float yaw;		/**< The yaw. */
			float pitch;	/**< The pitch. */
			float roll;		/**< The roll. */
		};

		struct
		{
			float x;	/**< The x component. */
			float y;	/**< The y component. */
			float z;	/**< The z component. */
		};

		struct
		{
			float c0;	/**< Component 0. */
			float c1;	/**< Component 1. */
			float c2;	/**< Component 2. */
		};
	};

} yprf;

/** \brief Represents a quaternion reading with underlying data type of 
 *  <c>float</c>. */
typedef struct
{
	union
	{
		/** The components. */
		float c[4];

		struct
		{
			float x;	/**< The x component. */
			float y;	/**< The y component. */
			float z;	/**< The z component. */
			float w;	/**< The w component. */
		};

		struct
		{
			float c0;	/**< Component 0. */
			float c1;	/**< Component 1. */
			float c2;	/**< Component 2. */
			float c3;	/**< Component 2. */
		};
	};

} quatf;

/** \brief Converts a mat3f to a string.
*
* \param[out] out The char buffer to output the result to.
* \param[in] m The mat3f to convert.
*/
void strFromMat3f(char* out, mat3f m);

#ifdef __cplusplus
}
#endif

#endif

