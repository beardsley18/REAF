/* VectorNav Programming Library v1.1.0.115
 * Copyright (c) 2016 VectorNav Technologies, LLC */
#include "vn/math.h"

#include <stdio.h>

#if defined(_MSC_VER)
	/* Disable warnings regarding using sprintf_s since these
	 * function signatures do not provide us with information
	 * about the length of 'out'. */
	#pragma warning(push)
	#pragma warning(disable:4996)
#endif

void strFromMat3f(char* out, mat3f m)
{
	sprintf(out, "[(%f; %f; %f) (%f; %f; %f) (%f; %f; %f)]", m.e00, m.e01, m.e02, m.e10, m.e11, m.e12, m.e20, m.e21, m.e22);
}

#if defined(_MSC_VER)
	#pragma warning(pop)
#endif
