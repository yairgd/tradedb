/*
 * =====================================================================================
 *
 *       Filename:  craeteFile.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  03/12/2012 08:11:21 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *       Author:  Yair Gadelov (yg), yair.gadelov@gmail.com
 *       Company:  Israel
 *
 * =====================================================================================
 */

#include "mex.h"
#include "../src/db.h"
#include <stdlib.h>
#include <stdio.h>

#define GET_FIELD(mx_array,field_name,val_out)  { \
if  ( mxArray *mx_field = mxGetField( (mx_array) , (0), (field_name) ) )   { \
double *val_ptr=mxGetPr (mx_field );\
(val_out)=*val_ptr;\
} \
else (val_out)=0;\
}

#define GET_STRING1(_m_array,out) \
{	\
	int n = mxGetN( (_m_array))+1; \
	(out) =(char*) mxMalloc(n); \
	mxGetString( (_m_array),(out),n);\
}

void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[]) {

	char *filename, *fieldName;
	int nFields, nStructs, i;
	mxArray *tmp;
	Field *field;
	unsigned int fileID;

	int mantisa, bit_length;
	if (nrhs != 2)
		mexErrMsgTxt("Usage: createFile(filename,fieldList)");

	GET_STRING1( prhs[0], filename);
	nStructs = mxGetNumberOfElements(prhs[1]);
	nFields = mxGetNumberOfFields(prhs[1]);

	//mexPrintf ( "file name:%s number structs: %d\n",filename,nStructs );

	// create the file, set header,close file
	fileID = DB_Create(filename);

	for (i = 0; i < nStructs; i++) {
		// get name
		tmp = mxGetField(prhs[1], i, "name");
		if (tmp == 0)
			mexErrMsgTxt("Error:  no name at filed #%d\n", i);
		GET_STRING1(tmp, fieldName);

		// get field length
		tmp = mxGetField(prhs[1], i, "bit_length");
		if (tmp == 0)
			mexErrMsgTxt("Error:  no bit_length at filed #%d\n", i);
		bit_length = (int) *mxGetPr(tmp);

		// get field length
		tmp = mxGetField(prhs[1], i, "mantisa");
		if (tmp == 0)
			mexErrMsgTxt("Error:  no mantisa at filed #%d\n", i);
		mantisa = (int) *mxGetPr(tmp);

		//field = DBField_Create(fieldName, DB_LEN16, mantisa);
	 //	DBHeader_AddField(fileID, field);

		//mexPrintf ( "field name: %s bit_length: %d mantisa:%d\n",fieldName,bit_length,mantisa );
		mxFree(fieldName);

	}

 	// update file header		
 //	DB_WriteHeader(fileID);
 //	DB_WriteFields(fileID);

	// close file
 	DB_Close(fileID);

	mxFree(filename);
	return;

}

