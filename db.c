
#include "db.h"
#include <string.h>
#include <math.h>


#define GET_BIT(data_bit,i) ( ((unsigned char *)(data_bit))[ (i)>>3 ] & 1<<( 7 -(i%8) ) )>0
#define RESET_BIT(data_bit,i)  (((unsigned char *)(data_bit))[ (i)>>3 ])&= (~1<<( 7 -(i%8)) )
#define SET_BIT(data_bit,i)  (((unsigned char *)(data_bit))[ (i)>>3 ])|= (1<<( 7 -(i%8)) )

 static FileRecord fileRecords[MAX_OPEN_FILES];
// static Field *workingFields[WORKING_FIELDS];
 static WorkingFields workingFields[WORKING_FIELDS];
 static int errID;
 static void *g_buffer;

// char DBField_Free(Field *field);
// char DBFile_Init(File *file);
//
WorkingFields *GetWorkingField(int idx)
{
	if (idx<WORKING_FIELDS)
		return &workingFields[idx];
	else
		return 0;
}
 unsigned int FileRecord_HashCoding(char  *str)
{
   // RSHsh  http://wizardsoweb.com/rs-hashing-algorithm-and-program/2011/05/
   // http://icodesnip.com/snippet/c/hashing-function-for-c
   unsigned int b    = 378551;
   unsigned int a    = 63689;
   unsigned int hash = 0;
   int i;

   for(i = 0; str[i]!=0; i++)
   {
      hash = hash * a + str[i];
      a    = a * b;
   }
   return hash;
   
}

 int FileRecord_GetIndexByID(unsigned int id)
{
	int i;
	for (i = 0;i<MAX_OPEN_FILES;i++)
	{
		if (fileRecords[i].table.header.id==id)
			return i;
	}
	return -1;
	
}
 int FileRecord_GetIndexByName(char *str)
{
	int i;
	for (i = 0;i<MAX_OPEN_FILES;i++)
	{
		if (fileRecords[i].table.header.id==FileRecord_HashCoding(str))
			return i;
	}
	return -1;
}

 int FileRecord_GetAvaileableIndex()
{
	int i;
	for (i = 0; i<MAX_OPEN_FILES;i++)
	{
		if (fileRecords[i].table.header.id==-1)
			return i;
	}
	return -1;
}


 int DBTable_GetFirstEntry(Table *table)
{
	int tot_length = sizeof(Header);
	tot_length+= table->header.nFields * sizeof(Field);
	return tot_length;
}

FileRecord *DBFileRecord_GetRecord(FILE_ID fileid)
{
	int i;
	if ((i=FileRecord_GetIndexByID(fileid)<0))
	{
		DB_SetLastError(0);
		return NULL;
	}
	return &fileRecords[i];
	
}

 char DBFileRecord_SetPointer(FileRecord *fileRecord, int fileP)
{
	int nRows=0;
	int sz;

	// calcaulte number of rows in the current file
	fseek(fileRecord->fP, 0L, SEEK_END); 
	sz = ftell(fileRecord->fP);
	sz -= DBTable_GetFirstEntry(&fileRecord->table);
	sz /= fileRecord->table.header.nFields;

	// check if adding rows is neccesery
	nRows = fileP - sz; 
	if (nRows>=0)
	{
		fileRecord->table.header.totalRecords+=nRows;
		DBFileRecord_AddZeroRows(fileRecord,nRows);
	}
	
	// update file pointer
	fileRecord->filePointer = fileP;


	DB_SetLastError(0);
	return OK;
	
}

 char DBFileRecord_ReadBuffer(FileRecord *fileRecord)
{
	int i;
	int fileCursor;
	int n=0;
	// set the file cursor
	fileCursor =  fileRecord->filePointer*fileRecord->table.row_length;
	// add the header in to account
	fileCursor += 	DBTable_GetFirstEntry(&fileRecord->table);

	// read buffer to memory
	fseek ( fileRecord->fP, fileCursor , SEEK_SET);
	fread (g_buffer+(fileRecord->table.row_length)*n,sizeof(char)*fileRecord->table.row_length,BUFFER_SIZE,fileRecord->fP);

	// fetch data from file to curre
	for (i=0;i<BUFFER_SIZE;i++)
	{
		DBTable_ReadRow(&fileRecord->table,i  );
	}
	DB_SetLastError(0);
	return OK;


}






 char DBTable_ReadRow(Table *table,int i_g_buffer)
{
	int i;
	int tot_length=0;
	int field_length;
	void *row = (g_buffer+table->row_length*i_g_buffer);
	for (i=0;i<table->header.nFields;i++)
	{
		field_length = table->fields[i]->length;
		switch (field_length-1)
		{
			case DB_LEN8:
				table->fields[i]->buffer[i_g_buffer]=*((char  *)(row+tot_length));
				tot_length+=1;
			break;

			case DB_LEN16:
				table->fields[i]->buffer[i_g_buffer] = *((short  *)(row+tot_length));
				tot_length+=2;
			break;

			case DB_LEN32:
				table->fields[i]->buffer[i_g_buffer] = *((int  *)(row+tot_length));
				tot_length+=3;
			break;

			case DB_LEN64:		
				table->fields[i]->buffer[i_g_buffer] = *((long  *)(row+tot_length));
				tot_length+=4;
			break;
				
		}

	}
	DB_SetLastError(0);
	return OK;


}

int DB_WriteColumn_old(int workingFieldIdx,float *buffer,int buffer_size,int bufferPlace)
{
	int i;
	Field *field = workingFields[workingFieldIdx].field;
//	FileRecord *fileRecord =  workingFields[workingFieldIdx].fileRecord;
//	int ref = 0;
	for (i=0;i<buffer_size;i++)
//	for (i=0;i<buffer_size && bufferPlace+i+HALF_BUFFER_SIZE>0 && bufferPlace+i+HALF_BUFFER_SIZE<BUFFER_SIZE ;i++)
	{

		if (bufferPlace+i>HALF_BUFFER_SIZE)
		{
			// set file rpointer
		//	DBFileRecord_WriteBuffer(fileRecord);
		//	DBFileRecord_SetPointer(fileRecord,fileRecord->filePointer+BUFFER_SIZE);
		//	ref+=BUFFER_SIZE;
		//	DB_SetFilePointer(fileid,
			return i;
		}
		if (bufferPlace+i<-HALF_BUFFER_SIZE)
		{
			// set file rpointer
			return i;

		}

		field->buffer[bufferPlace+i+HALF_BUFFER_SIZE] = buffer[i];
	}
	return i;
//	if (i!=buffer_size)
//	{
//		printf("error line %d at file %s: buffer out of range",__LINE__,__FILE__);
//	}
//	DB_SetLastError(0);
//	return ERR;
}

int DB_WriteColumn(int workingFieldIdx,float *buffer,int buffer_length,int filePointer)
{
	int bufferPlace ;
	int nWrite;
//	int idx = 0;
	int n=0;
	WorkingFields *workField;
	// get working field
	workField = &workingFields[workingFieldIdx];
	while (1)
	{

		if ( (filePointer - workField->fileRecord->filePointer>HALF_BUFFER_SIZE)
		|| (filePointer - workField->fileRecord->filePointer<-HALF_BUFFER_SIZE))
		{
			// flash current buffer
			DBFileRecord_WriteBuffer(workField->fileRecord);
			// set new file pointer 
			DBFileRecord_SetPointer(workField->fileRecord,filePointer);
			// read new buffer
			DBFileRecord_ReadBuffer(workField->fileRecord);
		
		}

		// set buffer place
		bufferPlace = filePointer - workField->fileRecord->filePointer-HALF_BUFFER_SIZE;

		
		// write to buffer
		nWrite = DBField_WriteBuffer(workField->field,buffer+n,buffer_length-n,bufferPlace);

		n+=nWrite;

		if (n>=buffer_length)
			break;

		filePointer+=BUFFER_SIZE;

	}

}

int DB_ReadColumn(int workingFieldIdx,float *buffer,int buffer_length,int filePointer)
{
	int bufferPlace ;
	int nWrite;
	int n=0;
	WorkingFields *workField;
	// get working field
	workField = &workingFields[workingFieldIdx];
	while (1)
	{

		if ( (filePointer - workField->fileRecord->filePointer>HALF_BUFFER_SIZE)
		|| (filePointer - workField->fileRecord->filePointer<-HALF_BUFFER_SIZE))
		{
			// flash current buffer
			DBFileRecord_WriteBuffer(workField->fileRecord);
			// set new file pointer 
			DBFileRecord_SetPointer(workField->fileRecord,filePointer);
			// read new buffer
			DBFileRecord_ReadBuffer(workField->fileRecord);
		
		}

		// set buffer place
		bufferPlace = filePointer - workField->fileRecord->filePointer-HALF_BUFFER_SIZE;

		
		// write to buffer
		nWrite = DBField_ReadBuffer(workField->field,buffer+n,buffer_length-n,bufferPlace);

		n+=nWrite;
		if (n>=buffer_length)
			break;

		filePointer+=BUFFER_SIZE;

	}

}


 int DB_ReadColumn_old(int workingFieldIdx,float *buffer,int buffer_size,int bufferPlace)
{
	int i;
	Field *field = workingFields[workingFieldIdx].field;
//	for (i=0;i<buffer_size && bufferPlace+i+HALF_BUFFER_SIZE>0 && bufferPlace+i+HALF_BUFFER_SIZE<BUFFER_SIZE ;i++)
//		buffer[i] = field->buffer[bufferPlace+i+HALF_BUFFER_SIZE] ;
//
	for (i=0;i<buffer_size;i++)
	{

		if (bufferPlace+i>HALF_BUFFER_SIZE)
		{
			// set file rpointer
		}
		if (bufferPlace+i<-HALF_BUFFER_SIZE)
		{
			// set file rpointer
		}

		buffer[i] = field->buffer[bufferPlace+i+HALF_BUFFER_SIZE] ;

		//field->buffer[bufferPlace+i+HALF_BUFFER_SIZE] = buffer[i];
	}

	return buffer_size-i;
//	if (i!=buffer_size)
//	{
//		printf("error line %d at file %s: buffer out of range",__LINE__,__FILE__);
//	}
//	DB_SetLastError(0);
//	return ERR;
}


char DB_SetColumn(FILE_ID fileid,char *fieldName,int workingFieldIdx)
{
	Field *field;
	Table *table;
	int i;
	
	if  ( (i = FileRecord_GetIndexByID(fileid))<0)
	{
		DB_SetLastError(0);
		return ERR;
	}
	
	table = &fileRecords[i].table;

	if  ( !(field = DBTable_GetFieldByName (table,fieldName)))
	{
		DB_SetLastError(0);
		return ERR;
	}	
	
	workingFields[workingFieldIdx].field=field;
	workingFields[workingFieldIdx].fileRecord = &fileRecords[i];

	

	DB_SetLastError(0);
	return OK;
}

char DB_SetFilePointer(FILE_ID fileid,int fileP)
{
	int i;
	if ((i=FileRecord_GetIndexByID(fileid)<0))
	{
		DB_SetLastError(0);
		return ERR;
	}
	return DBFileRecord_SetPointer(&fileRecords[i],fileP);
	
}



char DB_ReadBuffer(FILE_ID fileid)
{
	int i;
	if ((i=FileRecord_GetIndexByID(fileid)<0))
	{
		DB_SetLastError(0);
		return ERR;
	}
	return DBFileRecord_ReadBuffer(&fileRecords[i]);
	
}
/*
char DB_WriteBuffer(FILE_ID fileid)
{
	int i;
	if ((i=FileRecord_GetIndexByID(fileid)<0))
	{
		DB_SetLastError(0);
		return ERR;
	}
	return DBFileRecord_WriteBuffer(&fileRecords[i]);
	
}*/

 char DBFileRecord_WriteBuffer(FileRecord *fileRecord)
{

	int i;
	int fileCursor;
	int n=0;
	
	// set data in buffer
	for (i=0;i<BUFFER_SIZE;i++)
	{
		DBTable_WriteRow(&fileRecord->table, i );
	}

	// calculate file cursor
	fileCursor =  fileRecord->filePointer*fileRecord->table.row_length;

	// take heder into account
	fileCursor += 	DBTable_GetFirstEntry(&fileRecord->table);

	// write buffer to file
	fseek ( fileRecord->fP, fileCursor , SEEK_SET);
	n = fwrite (g_buffer+(fileRecord->table.row_length)*n,sizeof(char)*fileRecord->table.row_length,BUFFER_SIZE-n,fileRecord->fP);

	DB_SetLastError(0);
	return OK;

}

void DB_SetLastError(int _errID)
{
	errID = _errID;
}
int DB_GetLastError()
{
	return errID;
}
void DB_Init()
{
	int i;
	g_buffer = malloc(1e6);
	for ( i=0;i<MAX_OPEN_FILES;i++)
	{
		fileRecords[i].table.header.id=-1;
	}
	for (i = 0; i <WORKING_FIELDS;i++)
		workingFields[0].field=0;
	
}
char DB_Finalize()
{
	free(g_buffer);
	DB_SetLastError(0);
	return OK;

}


char DB_Close(unsigned int hashCode)
{
	int i;//,j,k;
	if ( (i=FileRecord_GetIndexByID(hashCode))>-1)
	{
		// flash current buffer
		DBFileRecord_WriteBuffer(&fileRecords[i]);

		// close file
		fclose(fileRecords[i].fP);
		fileRecords[i].table.header.id = -1;
		DBTable_Free(&fileRecords[i].table);
		DB_SetLastError(0);
		return OK;
	}

	DB_SetLastError(0);
	return ERR;
		
	
}
Header * DB_GetHeader(unsigned int hashCode)
{
	int i;
	if ( (i=FileRecord_GetIndexByID(hashCode))==OK)
	{
		fclose(fileRecords[i].fP);
		fileRecords[i].filePointer = -1;
		DB_SetLastError(0);
		return 0;
	}

	DB_SetLastError(0);
	return &fileRecords[i].table.header;

}

char DBField_Free(Field *field)
{
	int i;
	// remove field from working fields list
	for (i = 0; i<WORKING_FIELDS;i++)
		if (workingFields[i].field==field)
			workingFields[i].field=NULL;

	// free buffer and fields
	if (field->buffer)
		free (field->buffer);
	free (field);
	return OK;

}
Field * DBField_GetByName(unsigned int fileID,char *name)
{
	int i,fileIndex;
	if  ( (fileIndex = FileRecord_GetIndexByID (fileID))>-1)
	{
	
		for (i=0;i<MAX_FIELDS;i++)
		{
			if (fileRecords[fileIndex].table.fields[i]->id==FileRecord_HashCoding(name))
			{
				return fileRecords[fileIndex].table.fields[i];
			}
		}
	}
	return 0;
}
Field * DBField_GetByIndex(unsigned int fileID,int idx)
{
	int fileIndex;
	if  ( (fileIndex = FileRecord_GetIndexByID (fileID))>-1)
	{
		
		if (idx < fileRecords[fileIndex].table.header.nFields)
		{
			return fileRecords[fileIndex].table.fields[idx];
		}
		
	}
	return 0;
}

int DBField_WriteBuffer(Field *field,float *buffer,int buffer_size,int bufferPlace)
{
	int i;
	for (i=0;i<buffer_size;i++)
	{

		if (bufferPlace+i>HALF_BUFFER_SIZE)
		{
			return i;
		}
		if (bufferPlace+i<-HALF_BUFFER_SIZE)
		{
			// set file rpointer
			return i;

		}

		field->buffer[bufferPlace+i+HALF_BUFFER_SIZE] = buffer[i];
	}
	return i;
}

int DBField_ReadBuffer(Field *field,float *buffer,int buffer_size,int bufferPlace)
{
	int i;
	for (i=0;i<buffer_size;i++)
	{

		if (bufferPlace+i>HALF_BUFFER_SIZE)
		{
			// set file rpointer
			return i;
		}
		if (bufferPlace+i<-HALF_BUFFER_SIZE)
		{
			// set file rpointer
			return i;
		}

		buffer[i] = (field->buffer[bufferPlace+i+HALF_BUFFER_SIZE]);///pow(10,field->mantisa);
	}

	return i;
}



unsigned int DB_Open(char *filename)
{

	FILE *f;  
	int fileIndex;
	fileIndex =  FileRecord_GetIndexByName(filename);
	if (fileIndex>0)
	{
		// file exist, return handle
		DB_SetLastError(0);
		return fileIndex;
	}
	if ( (fileIndex = FileRecord_GetAvaileableIndex())<0)
	{
		// cannot open file, no memory
		DB_SetLastError(0);
		return 0;
	}
 
	f=fopen(filename,"rb+");  
	if (f)
	{
	    fileRecords[fileIndex].table.header.id = FileRecord_HashCoding(filename);
	    fileRecords[fileIndex].fP = f;
	    fileRecords[fileIndex].table.header.totalRecords = 0;

	    DBFileRecord_ReadHeader(&fileRecords[fileIndex]);

	    DBFileRecord_ReadFields(&fileRecords[fileIndex]);

	    DBTable_Init(&fileRecords[fileIndex].table);
	   	
	    DBFileRecord_SetPointer(&fileRecords[fileIndex],0);

	    DBFileRecord_ReadBuffer(&fileRecords[fileIndex]);

	
	    DB_SetLastError(0);

	    return fileRecords[fileIndex].table.header.id ;
	}
	else
	{
	     // cannot open file 
	     DB_SetLastError(0);
	     return 0;
	}
	

}

 char DBTable_Init(Table *table)
{
// set field buffer
	int i;
	int tot_length = 0;
	for (i = 0; i<table->header.nFields;i++)
	{
	   
		DBField_Init(table->fields[i]);
		switch (table->fields[i]->length-1)
		{
			case DB_LEN8:
				tot_length+=1;
			break;
			case DB_LEN16:
				tot_length+=2;
			break;
			case DB_LEN32:
				tot_length+=3;
			break;
			case DB_LEN64:
				tot_length+=4;
			break;


		}
	}
	table->row_length = tot_length;
	DB_SetLastError(0);
	return OK;
}

 char DBTable_Free(Table *table)
{

	int i;
	for (i = 0; i<table->header.nFields;i++)
	   {
		DBField_Free(table->fields[i]);
  	   }
	DB_SetLastError(0);
	return OK;

}
 char DBField_Init(Field *field)
{
	field->buffer =  (float*)malloc(sizeof(float)*BUFFER_SIZE);

	field->mantisa = (field->numFormat&0x20) ? (field->numFormat&0x1f)-32 : (field->numFormat&0x1f) ;
	field->length = ((field->numFormat&0xC0)>>6) + 1;

	DB_SetLastError(0);
	return OK;
}

unsigned int DB_Create(char *filename)
{

	FILE *f;  
	int fileIndex;
	fileIndex =  FileRecord_GetIndexByName(filename);
	if (fileIndex>0)
	{
		// file exist, return handle
		DB_SetLastError(0);
		return fileIndex;
	}
	if ( (fileIndex = FileRecord_GetAvaileableIndex())<0)
	{
		// cannot open file, no memory
		DB_SetLastError(0);
		return 0;
	}
 
	f=fopen(filename,"wb+");  
	if (f)
	{
	    fileRecords[fileIndex].table.header.id = FileRecord_HashCoding(filename);
	    fileRecords[fileIndex].fP = f;
	    fileRecords[fileIndex].table.header.totalRecords = 0;
//	   	    DBFileRecord_SetPointer(0);
	    DB_SetLastError(0);
	    return fileRecords[fileIndex].table.header.id ;
	}
	else
	{
	     // cannot open file 
	     DB_SetLastError(0);
	     return 0;
	}
	

}



 Field * DBTable_GetFieldByName(Table *table,char *fileName)
{
	int i;
	for (i=0;i<table->header.nFields;i++)
	{
		if (strcmp(fileName,table->fields[i]->name)==0)
			return table->fields[i];
		
	}
	return NULL;
}

// char DBFile_GetInfo(int hashCode, Header *header)
//{
//	return 0;
//}

/*char DBFile_SetTime(int hashCode, Time time)
{
	Header header;
	header.initialTime=time;
	return 0;
}
*/

 char DBFileRecord_AddZeroRows(FileRecord *fileRecord,int nRows)
{
	int firstEntry,i;
	DBTable_SetZeros(&fileRecord->table);
	firstEntry = DBTable_GetFirstEntry(&fileRecord->table);
	fseek ( fileRecord->fP, firstEntry , SEEK_SET);
	for (i = 0 ; i<nRows; i++)		
		fwrite(&fileRecord->table.header,sizeof(char),fileRecord->table.row_length,fileRecord->fP);  
	return OK;
}

void DBTable_SetZeros(Table *table)
{
	int i;
	void *row = g_buffer;
	for (i=0;i<table->row_length;i++)
		((char *)row)[i]=0;
}

char DBTable_WriteRow(Table *table,int n)
{
	int i=0;
	int field_length ;//= (file->fields->numFormat&0xf0)>>4;
	int tot_length =0;
	void *row = (g_buffer+n*table->row_length);
	for (i=0;i<table->header.nFields;i++)
	{
		
		field_length = table->fields[i]->length;
		switch (field_length-1)
		{
			case DB_LEN8:
			 	*((char  *)(row+tot_length))=table->fields[i]->buffer[n];
				tot_length+=1;
			break;

			case DB_LEN16:
				*((short  *)(row+tot_length))=table->fields[i]->buffer[n];
				tot_length+=2;
			break;

			case DB_LEN32:
				*((int  *)(row+tot_length))=table->fields[i]->buffer[n];

				tot_length+=3;
			break;

			case DB_LEN64:
				*((long  *)(row+tot_length))=table->fields[i]->buffer[n];
				tot_length+=4;
			break;
				
		}

	}
	DB_SetLastError(0);
	return OK;

}



Field * DBField_Create(char *fieldname,char size,char mantisa)
{
	Field *field;
	int j;
	// allocate place for structure
	field = (Field*)malloc(sizeof(Field));
	if (field==NULL)
		return NULL;
	// set field name
	//field->name=(char*)malloc(strlen(fieldname)+1);
	//field->name[0] = strlen(fieldname);/
	//strcpy( field->name+1,fieldname);
		
	for (j=0;j<FIELD_NAME_LEN;j++)
	{
		field->name[j]=0;
	}

	strcpy( field->name,fieldname);
	field->id = FileRecord_HashCoding(field->name);
	
	field->buffer = 0;	
		

	// set format type
	field->numFormat = ((size&0x0f)<<6)+ ((mantisa&0x3f));
//	field->numFormat = (size&0x0f)*16+ (mantisa&0x0f);


	return field;

}


Header * DBHeader_Create()
{
	Header *fh;
	fh = (Header*)malloc(sizeof(Header));
	fh->totalRecords=0;
	fh->nFields=0;
	return fh;
}

void DBHeader_Free(Header *header)
{
	free(header);
}

char DBHeader_AddField(unsigned int fileID,Field *field)
{
	int n;
	int fileIndex = FileRecord_GetIndexByID (fileID);
	if (fileIndex<0)
	{
		// invalid faile handle
		DB_SetLastError(0);
		return ERR;
	}
	
	n = fileRecords[fileIndex].table.header.nFields;

	
	fileRecords[fileIndex].table.header.nFields++;
	fileRecords[fileIndex].table.fields[n] = field;



//	header->fieldType[header->nFields]=field;

	return OK;
}

char DB_WriteHeader(unsigned int fileID)
{
	int fileIndex = FileRecord_GetIndexByID (fileID);
	if (fileIndex<0)
	{
		// invalid faile handle
		DB_SetLastError(0);
		return ERR;
	}
	fwrite(&fileRecords[fileIndex].table.header,sizeof(struct _Header),1,fileRecords[fileIndex].fP);  
	return OK;

}


char DBFile_ReadHeader_old1(int fileID)
{
	int fileIndex = FileRecord_GetIndexByID (fileID);
	if (fileIndex<0)
	{
		// invalid faile handle
		DB_SetLastError(0);
		return ERR;
	}
	fseek ( fileRecords[fileIndex].fP, 0 , SEEK_SET);
	fread (&fileRecords[fileIndex].table.header,sizeof(struct _Header),1,fileRecords[fileIndex].fP);  
	return OK;

}

 char DBFileRecord_ReadHeader(FileRecord *fileRecord)

{
	fseek ( fileRecord->fP, 0 , SEEK_SET);
	fread (&fileRecord->table.header,sizeof(struct _Header),1,fileRecord->fP);  
	return OK;

}

 char DBFileRecord_ReadFields(FileRecord *fileRecord)
{
	int i;
	// set file pointer to field place
	fseek ( fileRecord->fP, sizeof(Header) , SEEK_SET);

	// red fields
	for (i=0;i<fileRecord->table.header.nFields;i++)
	{
		// allocate place
		fileRecord->table.fields[i] = (Field*)malloc(sizeof(Field));
		// read from file
		fread(fileRecord->table.fields[i],sizeof(Field),1,fileRecord->fP); 
		// set hash key 
		fileRecord->table.fields[i]->id = FileRecord_HashCoding(fileRecord->table.fields[i]->name);
	
	}
	return OK;

}

 char DBFile_ReadFields_old1(unsigned int fileID)
{
	int i;
	int fileIndex = FileRecord_GetIndexByID (fileID);
	if (fileIndex<0)
	{
		// invalid faile handle
		DB_SetLastError(0);
		return ERR;
	}
	// set file pointer to field place
	fseek ( fileRecords[fileIndex].fP, sizeof(Header) , SEEK_SET);

	// red fields
	for (i=0;i<fileRecords[fileIndex].table.header.nFields;i++)
	{
		// allocate place
		fileRecords[fileIndex].table.fields[i] = (Field*)malloc(sizeof(Field*));
		// read from file
		fread(fileRecords[fileIndex].table.fields[i],sizeof(Field),1,fileRecords[fileIndex].fP); 
		// set hash key 
		fileRecords[fileIndex].table.fields[i]->id = FileRecord_HashCoding(fileRecords[fileIndex].table.fields[i]->name);
	
	}
	return OK;

}


char DB_WriteFields(unsigned int fileID)
{
	int i;
	int fileIndex = FileRecord_GetIndexByID (fileID);
	if (fileIndex<0)
	{
		// invalid faile handle
		DB_SetLastError(0);
		return ERR;
	}
	for (i=0;i<fileRecords[0].table.header.nFields;i++)
	{
		fwrite(fileRecords[fileIndex].table.fields[i],sizeof(Field),1,fileRecords[fileIndex].fP);  
		
	}
	return OK;

}

