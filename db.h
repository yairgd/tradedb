/*
 * =====================================================================================
 *
 *       Filename:  . db.h
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  13/10/11 03:53:51
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Yair Gadelov (yg), yair.gadelov@gmail.com
 *        Company:  Israel
 *
 * =====================================================================================
 */

#ifndef DB_H
#define DB_H

#include <stdio.h> 
#include <stdlib.h>


#define MAX_OPEN_FILES 20
#define WORKING_FIELDS 20
#define OK  1
#define ERR 0
#define MAX_FIELDS 16

#define DB_LEN8   0
#define DB_LEN16  1
#define DB_LEN32  2
#define DB_LEN64 3

#define FIELD_NAME_LEN 16
#define HALF_BUFFER_SIZE 1024
#define BUFFER_SIZE 2*HALF_BUFFER_SIZE+1
#define FILE_ID unsigned int

typedef struct _Time {
 unsigned char hour,minutes,seconds,day,month;
    unsigned char nIntruments;
 unsigned short year,msSec;
} Time;

typedef struct _Field {
	
	// 4-upper bits - number of  bytes,4  mantisa 10^(a) a is 2 bit number
	char numFormat; 
	// first bytes sets the file lengths
	char name[FIELD_NAME_LEN];
	
	// field id
	unsigned int id;

	// set bufeer;
	float *buffer;

	// field length
	char length;
	
	// fiel mantisa
 	float mantisa;


} Field;

typedef struct _Header {
   Time initialTime ;
   unsigned int id;
   unsigned int totalRecords;
   unsigned char nFields;
}Header;

typedef struct _File {
   Header header;
   Field *fields[100];
  // void *row;
   int row_length; 
}File;

typedef struct _FileRecord {
	File file;
	FILE *fP;
	int filePointer;
//	float **buffer;

} FileRecord;

//unsigned int FileRecord_HashCoding(char  *str);
//int FileRecord_GetIndexByID(int id);
//int FileRecord_GetIndexByName(char *str);
//int FileRecord_GetAvaileableIndex();

void DB_Init();
char DB_Finalize();
//char DB_UpdateFilePointer(FILE_ID fileid,int fileP);
void DB_SetLastError(int _errID);
int  DB_GetLastError();
int DB_WriteColumn(int workingFieldIdx,float *buffer,int buffer_size,int bufferPlace);
int DB_ReadColumn(int workingFieldIdx,float *buffer,int buffer_size,int bufferPlace);

char DB_SetColumn(FILE_ID fileid,char *fieldName,int workingFieldIdx);
FILE_ID DB_Open(char *filename);
FILE_ID DB_Create(char *filename);
char DB_SetFilePointer(FILE_ID fileid,int fileP);
char DB_ReadBuffer(FILE_ID fileid);
char DB_WriteBuffer(FILE_ID fileid);
char DB_WriteHeader(unsigned int hashCode);
char DB_Close(int hashCode);
Header * DB_GetHeader(unsigned int fileID);
char DB_WriteFields(unsigned int fileID);
 char DBFile_Init(File *file);
 char DBFile_Free(File *file);
 void DBFile_SetZeros(File *file);
 Field *DBFile_GetFieldByName(File *file,char *fielName);

//static char DBFile_GetInfo(int hashCode, Header *fh);
//char DBFile_ReadRow(	File *file, int idx );
char DBFile_ReadRow(File *file,int i_g_buffer);

char DBFile_WriteRow(File *file,int idx);

//char DBFile_nSetPointer(int fileID,int idx);
//static char DBFile_WriteRecord(unsigned char s );
char DBFile_SetTime(int hashCode, Time time);

//char DBFile_Close(File *file);
char DBFileRecord_ReadHeader(FileRecord *fileRecord);//(int hashCode);
char DBFileRecord_ReadFields(FileRecord *fileRecord);
//char DBFileRecord_ReadBuffer(FileRecord *fileRecord,int fileP);
char DBFileRecord_ReadBuffer(FileRecord *fileRecord);

char DBFileRecord_SetPointer(FileRecord *fileRecord, int p);
char DBFileRecord_WriteBuffer(FileRecord *fileRecord);


char DBFileRecord_AddZeroRows(FileRecord *fileRecord,int nRows);

char DBField_Init(Field *field);

char DBFileRecord_AddZeroRows(FileRecord *file,int nRows);
//char DBFile_AddBuffer(File *file);
//char DBFile_SetHeader(fileID);

char DBFile_AddZeroRows(FileRecord *file,int nRows);
int DBFile_GetFirstEntry(File *file);

//int DBField_Init(Field *field);
Field* DBField_Create(char *,char,char);

//static char DBFileld_ReadValue(Field *field,int idx);
//    void DBField_Delete(Field *);
 Field * DBField_GetByName(unsigned int fileID, char *name);
char DBField_Free(Field *field);

Field * DBField_GetByIndex(unsigned int fileID,int idx);
Header * DBHeader_Create();
 char DBHeader_AddField(unsigned int fileID,Field *field);
void DBHeader_Free(Header *header);














#endif
