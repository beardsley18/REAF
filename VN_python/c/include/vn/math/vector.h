/* VectorNav Programming Library v1.1.0.115
 * Copyright (c) 2016 VectorNav Technologies, LLC */
/** \file
* \section Description
* This header file contains declarations for the math types used by the
* VectorNav library. */
#ifndef _VN_MATH_VECTOR_H_
#define _VN_MATH_VECTOR_H_

#ifdef __cplusplus
extern "C" {
#endif

/** \brief Represents a 3 component vector with an underlying data type of
*  <c>float</c>. */
typedef struct
{
	union
	{
		/** The components. */
		float c[3];

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
	
} vec3f;

/** \brief Represents a 3 component vector with an underlying data type of
*  <c>double</c>. */
typedef struct
{
	union
	{
		/** The components. */
		double c[3];

		struct
		{
			double x;	/**< The x component. */
			double y;	/**< The y component. */
			double z;	/**< The z component. */
		};

		struct
		{
			double c0;	/**< Component 0. */
			double c1;	/**< Component 1. */
			double c2;	/**< Component 2. */
		};
	};
	
} vec3d;

/** \brief Represents a 4 component vector with an underlying data type of
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
	
} vec4f;

//vec3f vec3f_init(float x, float y, float z);

/** \brief Adds two vec3f together.
*
* \param[in] lhs The lhs vec3f.
* \param[in] rhs The rhs vec3f.
* \return The resulting vec3f from adding lhs and rhs together. */
vec3f add_v3f_v3f(vec3f lhs, vec3f rhs);

/** \brief Adds two vec3d together.
*
* \param[in] lhs The lhs vec3d.
* \param[in] rhs The rhs vec3d.
* \return The resulting vec3d from adding lhs and rhs together. */
vec3d add_v3d_v3d(vec3d lhs, vec3d rhs);

/** \brief Adds two vec4f together.
*
* \param[in] lhs The lhs vec4f.
* \param[in] rhs The rhs vec4f.
* \return The resulting vec4f from adding lhs and rhs together. */
vec4f add_v4f_v4f(vec4f lhs, vec4f rhs);

/** \brief Subtracts a vec3f from another vec3f.
*
* \param[in] lhs The lhs vec3f.
* \param[in] rhs The rhs vec3f.
* \return The resulting vec3f from subtracting rhs from lhs. */
vec3f sub_v3f_v3f(vec3f lhs, vec3f rhs);

/** \brief Subtracts a vec3d from another vec3d.
*
* \param[in] lhs The lhs vec3d.
* \param[in] rhs The rhs vec3d.
* \return The resulting vec3d from subtracting rhs from lhs. */
vec3d sub_v3d_v3d(vec3d lhs, vec3d rhs);

/** \brief Subtracts a vec4f from another vec4f.
*
* \param[in] lhs The lhs vec4f.
* \param[in] rhs The rhs vec4f.
* \return The resulting vec4f from subtracting rhs from lhs. */
vec4f sub_v4f_v4f(vec4f lhs, vec4f rhs);

/** \brief Converts a vec3f to a string.
 *
 * \param[out] out The char buffer to output the result to.
 * \param[in] v The vec3f to convert. */
void str_vec3f(char* out, vec3f v);

/** \brief Converts a vec3d to a string.
*
* \param[out] out The char buffer to output the result to.
* \param[in] v The vec3d to convert. */
void str_vec3d(char* out, vec3d v);

/** \brief Converts a vec4f to a string.
*
* \param[out] out The char buffer to output the result to.
* \param[in] v The vec4f to convert. */
void str_vec4f(char* out, vec4f v);

#ifdef __cplusplus
}
#endif

#endif
