/* VectorNav Programming Library v1.1.0.115
 * Copyright (c) 2016 VectorNav Technologies, LLC */
#ifndef _VN_UTIL_EXPORT_H
#define _VN_UTIL_EXPORT_H

/* Not only does this have to be windows to use __declspec */
/* it also needs to actually be outputting a DLL */
#if defined _WINDOWS && defined _WINDLL
	#if proglib_c_EXPORTS
		#define DllExport __declspec(dllexport)
	#else
		#define DllExport __declspec(dllimport)
	#endif
#else
	#define DllExport
#endif

#endif
