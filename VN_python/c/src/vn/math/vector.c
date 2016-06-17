/* VectorNav Programming Library v1.1.0.115
 * Copyright (c) 2016 VectorNav Technologies, LLC */
#include "vn/math/vector.h"

#include <stdio.h>

vec3f add_v3f_v3f(vec3f lhs, vec3f rhs)
{
	vec3f r;

	r.x = lhs.x + rhs.x;
	r.y = lhs.y + rhs.y;
	r.z = lhs.z + rhs.z;

	return r;
}

vec3d add_v3d_v3d(vec3d lhs, vec3d rhs)
{
	vec3d r;

	r.x = lhs.x + rhs.x;
	r.y = lhs.y + rhs.y;
	r.z = lhs.z + rhs.z;

	return r;
}

vec4f add_v4f_v4f(vec4f lhs, vec4f rhs)
{
	vec4f r;

	r.x = lhs.x + rhs.x;
	r.y = lhs.y + rhs.y;
	r.z = lhs.z + rhs.z;
	r.w = lhs.w + rhs.w;

	return r;
}

vec3f sub_v3f_v3f(vec3f lhs, vec3f rhs)
{
	vec3f r;

	r.x = lhs.x - rhs.x;
	r.y = lhs.y - rhs.y;
	r.z = lhs.z - rhs.z;

	return r;
}

vec3d sub_v3d_v3d(vec3d lhs, vec3d rhs)
{
	vec3d r;

	r.x = lhs.x - rhs.x;
	r.y = lhs.y - rhs.y;
	r.z = lhs.z - rhs.z;

	return r;
}

vec4f sub_v4f_v4f(vec4f lhs, vec4f rhs)
{
	vec4f r;

	r.x = lhs.x - rhs.x;
	r.y = lhs.y - rhs.y;
	r.z = lhs.z - rhs.z;
	r.w = lhs.w - rhs.w;

	return r;
}

#if defined(_MSC_VER)
	/* Disable warnings regarding using sprintf_s since these
	 * function signatures do not provide us with information
	 * about the length of 'out'. */
	#pragma warning(push)
	#pragma warning(disable:4996)
#endif

void str_vec3f(char* out, vec3f v)
{
	sprintf(out, "(%f; %f; %f)", v.x, v.y, v.z);
}

void str_vec3d(char* out, vec3d v)
{
	sprintf(out, "(%f; %f; %f)", v.x, v.y, v.z);
}

void str_vec4f(char* out, vec4f v)
{
	sprintf(out, "(%f; %f; %f; %f)", v.x, v.y, v.z, v.w);
}

#if defined(_MSC_VER)
	#pragma warning(pop)
#endif
