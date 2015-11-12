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

#include "xscalibrateddata.h"

/*!	\struct XsCalibratedData
	\brief Container for combined calibrated measurement data from accelerometers, gyroscopes and
	magnetometers.
*/

/*! \addtogroup cinterface C Interface
	@{
*/

/*! \relates XsCalibratedData
	\brief Construct an %XsCalibratedData object
*/
void XsCalibratedData_construct(XsCalibratedData* thisPtr, const XsReal* acc, const XsReal* gyr, const XsReal* mag)
{
	XsVector3_construct(&thisPtr->m_acc, acc);
	XsVector3_construct(&thisPtr->m_gyr, gyr);
	XsVector3_construct(&thisPtr->m_mag, mag);
}

/*!	\relates XsCalibratedData
	\brief Destruct an %XsCalibratedData object
*/
void XsCalibratedData_destruct(XsCalibratedData* thisPtr)
{
	XsVector3_destruct(&thisPtr->m_acc);
	XsVector3_destruct(&thisPtr->m_gyr);
	XsVector3_destruct(&thisPtr->m_mag);
}

/*! @} */
