/*	WARNING: COPYRIGHT (C) 2017 XSENS TECHNOLOGIES OR SUBSIDIARIES WORLDWIDE. ALL RIGHTS RESERVED.
	THIS FILE AND THE SOURCE CODE IT CONTAINS (AND/OR THE BINARY CODE FILES FOUND IN THE SAME
	FOLDER THAT CONTAINS THIS FILE) AND ALL RELATED SOFTWARE (COLLECTIVELY, "CODE") ARE SUBJECT
	TO A RESTRICTED LICENSE AGREEMENT ("AGREEMENT") BETWEEN XSENS AS LICENSOR AND THE AUTHORIZED
	LICENSEE UNDER THE AGREEMENT. THE CODE MUST BE USED SOLELY WITH XSENS PRODUCTS INCORPORATED
	INTO LICENSEE PRODUCTS IN ACCORDANCE WITH THE AGREEMENT. ANY USE, MODIFICATION, COPYING OR
	DISTRIBUTION OF THE CODE IS STRICTLY PROHIBITED UNLESS EXPRESSLY AUTHORIZED BY THE AGREEMENT.
	IF YOU ARE NOT AN AUTHORIZED USER OF THE CODE IN ACCORDANCE WITH THE AGREEMENT, YOU MUST STOP
	USING OR VIEWING THE CODE NOW, REMOVE ANY COPIES OF THE CODE FROM YOUR COMPUTER AND NOTIFY
	XSENS IMMEDIATELY BY EMAIL TO INFO@XSENS.COM. ANY COPIES OR DERIVATIVES OF THE CODE (IN WHOLE
	OR IN PART) IN SOURCE CODE FORM THAT ARE PERMITTED BY THE AGREEMENT MUST RETAIN THE ABOVE
	COPYRIGHT NOTICE AND THIS PARAGRAPH IN ITS ENTIRETY, AS REQUIRED BY THE AGREEMENT.
*/

#ifndef XSPORTINFOARRAY_H
#define XSPORTINFOARRAY_H

#include "xsarray.h"
#include "xsportinfo.h"

#ifdef __cplusplus
extern "C" {
#endif

extern XsArrayDescriptor const XSTYPES_DLL_API g_xsPortInfoArrayDescriptor;

#ifndef __cplusplus
#define XSPORTINFOARRAY_INITIALIZER	XSARRAY_INITIALIZER(&g_xsPortInfoArrayDescriptor)
struct XsPortInfo;

XSARRAY_STRUCT(XsPortInfoArray, struct XsPortInfo);
typedef struct XsPortInfoArray XsPortInfoArray;

XSTYPES_DLL_API void XsPortInfoArray_construct(XsPortInfoArray* thisPtr, XsSize count, struct XsPortInfo const* src);
#else
} // extern "C"
#endif

#ifdef __cplusplus
struct XsPortInfoArray : public XsArrayImpl<XsPortInfo, g_xsPortInfoArrayDescriptor, XsPortInfoArray> {
	//! \brief Constructs an XsPortInfoArray
	inline explicit XsPortInfoArray(XsSize sz = 0, XsPortInfo const* src = 0)
		 : ArrayImpl(sz, src)
	{
	}

	//! \brief Constructs an XsPortInfoArray as a copy of \a other
	inline XsPortInfoArray(XsPortInfoArray const& other)
		 : ArrayImpl(other)
	{
	}

	//! \brief Constructs an XsPortInfoArray that references the data supplied in \a ref
	inline explicit XsPortInfoArray(XsPortInfo* ref, XsSize sz, XsDataFlags flags = XSDF_None)
		: ArrayImpl(ref, sz, flags)
	{
	}

#ifndef XSENS_NOITERATOR
	//! \brief Constructs an XsPortInfoArray with the array bound by the supplied iterators \a beginIt and \a endIt
	template <typename Iterator>
	inline XsPortInfoArray(Iterator beginIt, Iterator endIt)
		: ArrayImpl(beginIt, endIt)
	{
	}
#endif
};
#endif

#endif
