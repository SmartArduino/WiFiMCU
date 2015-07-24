////////////////////////////////////////////////////////////////////////////////
//                   Mountain View Silicon Tech. Inc.
//		Copyright 2011, Mountain View Silicon Tech. Inc., ShangHai, China
//                   All rights reserved.
//
//		Filename	:debug.h
//
//		Description	:
//					Define debug ordinary print & debug routine
//
//		Changelog	:
///////////////////////////////////////////////////////////////////////////////

#ifndef __DEBUG_H__
#define __DEBUG_H__

#ifdef __cplusplus
extern "C" {
#endif//__cplusplus
#include <stdio.h>

#if defined(FUNC_DEBUG_EN)
	#define	DBG(format, ...)		printf(format, ##__VA_ARGS__)
#else
	#define	DBG(format, ...)
#endif
	
#if (defined(FUNC_DEBUG_EN) && defined(FUNC_APP_DEBUG_EN))
	#define	APP_DBG(format, ...)	printf(format, ##__VA_ARGS__)
#else
	#define	APP_DBG(format, ...)
#endif

#if (defined(FUNC_DEBUG_EN) && defined(FUNC_FS_DEBUG_EN))
	#define	FS_DBG(format, ...)		printf(format, ##__VA_ARGS__)
#else
	#define	FS_DBG(format, ...)
#endif

#define ASSERT(x)	

#ifdef __cplusplus
}
#endif//__cplusplus

#endif //__DBG_H__ 

