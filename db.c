
#include "db.h"
#include <string.h>
#include <math.h>


#define GET_BIT(data_bit,i) ( ((unsigned char *)(data_bit))[ (i)>>3 ] & 1<<( 7 -(i%8) ) )>0
#define RESET_BIT(data_bit,i)  (((unsigned char *)(data_bit))[ (i)>>3 ])&= (~1<<( 7 -(i%8)) )
#define SET_BIT(data_bit,i)  (((unsigned char *)(data_bit))[ (i)>>3 ])|= (1<<( 7 -(i%8)) )

 static FileRecord fileRecords[MAX_OPEN_FILES];
 static Field *workingFields[WORKING_FIELDS];
 static int errID;
 static void *g_buffer;

// char DBField_Free(Field *field);
// char DBFile_Init(File *file);
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
		if (fileRecords[i].file.header.id==id)
			return i;
	}
	return -1;
	
}
 int FileRecord_GetIndexByName(char *str)
{
	int i;
	for (i = 0;i<MAX_OPEN_FILES;i++)
	{
		if (fileRecords[i].file.header.id==FileRecord_HashCoding(str))
			return i;
	}
	return -1;
}

 int FileRecord_GetAvaileableIndex()
{
	int i;
	for (i = 0; i<MAX_OPEN_FILES;i++)
	{
		if (fileRecords[i].file.header.id==-1)
			return i;
	}
	return -1;
}


 int DBFile_GetFirstEntry(File *file)
{
	int tot_length = sizeof(Header);
	tot_length+= file->header.nFields * sizeof(Field);
	return tot_length;
}

 char DBFileRecord_SetPointer(FileRecord *fileRecord, int fileP)
{
	int nRows=0;
	int sz;

	// calcaulte number of rows in the current file
	fseek(fileRecord->fP, 0L, SEEK_END); 
	sz = ftell(fileRecord->fP);
	sz -= DBFile_GetFirstEntry(&fileRecord->file);
	sz /= fileRecord->file.header.nFields;

	// check if adding rows is neccesery
	nRows = fileP - sz; 
	if (nRows>=0)
	{
		fileRecord->file.header.totalRecords+=nRows;
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
	
	// set the file cursor
	fileCursor =  fileRecord->filePointer*fileRecord->file.row_length;
	int n;
	if (fileRecord->filePointer>HALF_BUFFER_SIZE)
	{
		fileCursor -= HALF_BUFFER_SIZE*fileRecord->file.row_length;
		n = 0;
	}
	else
	{
		fileCursor = 0;
		n = HALF_BUFFER_SIZE - fileRecord->filePointer ;
	}
	fileCursor += 	DBFile_GetFirstEntry(&fileRecord->file);

	// read buffer to memory
	fseek ( fileRecord->fP, fileCursor , SEEK_SET);
	fread (g_buffer+6*n,sizeof(char)*fileRecord->file.row_length,BUFFER_SIZE,fileRecord->fP);  

	// fetch data from file to curre
	for (i=0;i<BUFFER_SIZE;i++)
	{
		DBFile_ReadRow(	&fileRecord->file,i  );
	}
	DB_SetLastError(0);
	return OK;


}

 char DBFile_ReadRow(File *file,int i_g_buffer)
{
	int i;
	int tot_length=0;
	int field_length;
	void *row = (g_buffer+file->row_length*i_g_buffer);
	for (i=0;i<file->header.nFields;i++)
	{
		field_length = file->fields[i]->length;
		switch (field_length-1)
		{
			case DB_LEN8:
				file->fields[i]->buffer[i_g_buffer]=*((char  *)(row+tot_length));
				tot_length+=1;
			break;

			case DB_LEN16:
				file->fields[i]->buffer[i_g_buffer] = *((short  *)(row+tot_length));
				tot_length+=2;
			break;

			case DB_LEN32:
				file->fields[i]->buffer[i_g_buffer] = *((int  *)(row+tot_length));
				tot_length+=3;
			break;

			case DB_LEN64:		
				file->fields[i]->buffer[i_g_buffer] = *((long  *)(row+tot_length));
				tot_length+=4;
			break;
				
		}

	}
	DB_SetLastError(0);
	return OK;


}

int DB_WriteColumn(int workingFieldIdx,float *buffer,int buffer_size,int bufferPlace)
{
	int i;
	Field *field = workingFields[workingFieldIdx];
	for (i=0;i<buffer_size;i++)
//	for (i=0;i<buffer_size && bufferPlace+i+HALF_BUFFER_SIZE>0 && bufferPlace+i+HALF_BUFFER_SIZE<BUFFER_SIZE ;i++)
	{

		if (bufferPlace+i>HALF_BUFFER_SIZE)
		{
			// set file rpointer
		}
		if (bufferPlace+i<-HALF_BUFFER_SIZE)
		{
			// set file rpointer
		}


		field->buffer[bufferPlace+i+HALF_BUFFER_SIZE] = buffer[i];
	}
	return buffer_size-i;
//	if (i!=buffer_size)
//	{
//		printf("error line %d at file %s: buffer out of range",__LINE__,__FILE__);
//	}
//	DB_SetLastError(0);
//	return ERR;
}

 int DB_ReadColumn(int workingFieldIdx,float *buffer,int buffer_size,int bufferPlace)
{
	int i;
	Field *field = workingFields[workingFieldIdx];
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
	File *file;
	int i;
	
	if  ( (i = FileRecord_GetIndexByID(fileid))<0)
	{
		DB_SetLastError(0);
		return ERR;
	}
	
	file = &fileRecords[i].file;

	if  ( !(field = DBFile_GetFieldByName (file,fieldName)))
	{
		DB_SetLastError(0);
		return ERR;
	}	
	
	workingFields[workingFieldIdx]=field;
	

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
char DB_WriteBuffer(FILE_ID fileid)
{
	int i;
	if ((i=FileRecord_GetIndexByID(fileid)<0))
	{
		DB_SetLastError(0);
		return ERR;
	}
	return DBFileRecord_WriteBuffer(&fileRecords[i]);
	
}
 char DBFileRecord_WriteBuffer(FileRecord *fileRecord)
{

	int i;
	int fileCursor;
	int n;
	
	// set data in buffer
	for (i=0;i<BUFFER_SIZE;i++)
	{
		DBFile_WriteRow(&fileRecord->file, i );
	}


	// calculate file cursor
	fileCursor =  fileRecord->filePointer*fileRecord->file.row_length;
	if (fileRecord->filePointer>HALF_BUFFER_SIZE)
	{
		fileCursor -= HALF_BUFFER_SIZE*fileRecord->file.row_length;
		n = 0;
	}
	else
	{
		fileCursor = 0;
		n = HALF_BUFFER_SIZE - fileRecord->filePointer ;
	}
	fileCursor += 	DBFile_GetFirstEntry(&fileRecord->file);



	// write buffer to file
	fseek ( fileRecord->fP, fileCursor , SEEK_SET);
	n = fwrite (g_buffer+6*n,sizeof(char)*fileRecord->file.row_length,BUFFER_SIZE-n,fileRecord->fP);

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
		fileRecords[i].file.header.id=-1;
	}
	for (i = 0; i <WORKING_FIELDS;i++)
		workingFields[0]=0;
	
}
char DB_Finalize()
{
	free(g_buffer);
	DB_SetLastError(0);
	return OK;

}


char DB_Close(int hashCode)
{
	int i;//,j,k;
	if ( (i=FileRecord_GetIndexByID(hashCode))>-1)
	{
		fclose(fileRecords[i].fP);
		fileRecords[i].file.header.id = -1;
		DBFile_Free(&fileRecords[i].file);
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
	return &fileRecords[i].file.header;

}

char DBField_Free(Field *field)
{
	int i;
	// remove field from working fields list
	for (i = 0; i<WORKING_FIELDS;i++)
		if (workingFields[i]==field)
			workingFields[i]=NULL;

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
			if (fileRecords[fileIndex].file.fields[i]->id==FileRecord_HashCoding(name))
			{
				return fileRecords[fileIndex].file.fields[i];
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
		
		if (idx < fileRecords[fileIndex].file.header.nFields)
		{
			return fileRecords[fileIndex].file.fields[idx];
		}
		
	}
	return 0;
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
	    fileRecords[fileIndex].file.header.id = FileRecord_HashCoding(filename);
	    fileRecords[fileIndex].fP = f;
	    fileRecords[fileIndex].file.header.totalRecords = 0;

	    DBFileRecord_ReadHeader(&fileRecords[fileIndex]);

	    DBFileRecord_ReadFields(&fileRecords[fileIndex]);

	    DBFile_Init(&fileRecords[fileIndex].file);
	   	
	    DBFileRecord_SetPointer(&fileRecords[fileIndex],0);

	
	    DB_SetLastError(0);

	    return fileRecords[fileIndex].file.header.id ;
	}
	else
	{
	     // cannot open file 
	     DB_SetLastError(0);
	     return 0;
	}
	

}

 char DBFile_Init(File *file)
{
// set field buffer
	int i;
	int tot_length = 0;
	for (i = 0; i<file->header.nFields;i++)
	{
	   
		DBField_Init(file->fields[i]);
		switch (file->fields[i]->length-1)
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
	file->row_length = tot_length;
	DB_SetLastError(0);
	return OK;
}

 char DBFile_Free(File *file)
{

	int i;
	for (i = 0; i<file->header.nFields;i++)
	   {
		DBField_Free(file->fields[i]);
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
	    fileRecords[fileIndex].file.header.id = FileRecord_HashCoding(filename);
	    fileRecords[fileIndex].fP = f;
	    fileRecords[fileIndex].file.header.totalRecords = 0;
//	   	    DBFileRecord_SetPointer(0);
	    DB_SetLastError(0);
	    return fileRecords[fileIndex].file.header.id ;
	}
	else
	{
	     // cannot open file 
	     DB_SetLastError(0);
	     return 0;
	}
	

}



 Field * DBFile_GetFieldByName(File *file,char *fileName)
{
	int i;
	for (i=0;i<file->header.nFields;i++)
	{
		if (strcmp(fileName,file->fields[i]->name)==0)
			return file->fields[i];
		
	}
	return NULL;
}

// char DBFile_GetInfo(int hashCode, Header *header)
//{
//	return 0;
//}

char DBFile_SetTime(int hashCode, Time time)
{
	Header header;
	header.initialTime=time;
	return 0;
}


 char DBFileRecord_AddZeroRows(FileRecord *fileRecord,int nRows)
{
	int firstEntry,i;
	DBFile_SetZeros(&fileRecord->file);
	firstEntry = DBFile_GetFirstEntry(&fileRecord->file);
	fseek ( fileRecord->fP, firstEntry , SEEK_SET);
	for (i = 0 ; i<nRows; i++)		
		fwrite(&fileRecord->file.header,sizeof(char),fileRecord->file.row_length,fileRecord->fP);  
	return OK;
}

 void DBFile_SetZeros(File *file)
{
	int i;
	void *row = g_buffer;
	for (i=0;i<file->row_length;i++)
		((char *)row)[i]=0;
}

 char DBFile_WriteRow(File *file,int n)
{
	int i=0;
	int field_length ;//= (file->fields->numFormat&0xf0)>>4;
	int tot_length =0;
	void *row = (g_buffer+n*file->row_length);
	for (i=0;i<file->header.nFields;i++)
	{
		
		field_length = file->fields[i]->length;
		switch (field_length-1)
		{
			case DB_LEN8:
			 	*((char  *)(row+tot_length))=file->fields[i]->buffer[n];
				tot_length+=1;
			break;

			case DB_LEN16:
				*((short  *)(row+tot_length))=file->fields[i]->buffer[n];
				tot_length+=2;
			break;

			case DB_LEN32:
				*((int  *)(row+tot_length))=file->fields[i]->buffer[n];

				tot_length+=3;
			break;

			case DB_LEN64:
				*((long  *)(row+tot_length))=file->fields[i]->buffer[n];
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
	
	n = fileRecords[fileIndex].file.header.nFields;

	
	fileRecords[fileIndex].file.header.nFields++;
	fileRecords[fileIndex].file.fields[n] = field;



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
	fwrite(&fileRecords[fileIndex].file.header,sizeof(struct _Header),1,fileRecords[fileIndex].fP);  
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
	fread (&fileRecords[fileIndex].file.header,sizeof(struct _Header),1,fileRecords[fileIndex].fP);  
	return OK;

}

 char DBFileRecord_ReadHeader(FileRecord *fileRecord)

{
	fseek ( fileRecord->fP, 0 , SEEK_SET);
	fread (&fileRecord->file.header,sizeof(struct _Header),1,fileRecord->fP);  
	return OK;

}

 char DBFileRecord_ReadFields(FileRecord *fileRecord)
{
	int i;
	// set file pointer to field place
	fseek ( fileRecord->fP, sizeof(Header) , SEEK_SET);

	// red fields
	for (i=0;i<fileRecord->file.header.nFields;i++)
	{
		// allocate place
		fileRecord->file.fields[i] = (Field*)malloc(sizeof(Field));
		// read from file
		fread(fileRecord->file.fields[i],sizeof(Field),1,fileRecord->fP); 
		// set hash key 
		fileRecord->file.fields[i]->id = FileRecord_HashCoding(fileRecord->file.fields[i]->name);
	
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
	for (i=0;i<fileRecords[fileIndex].file.header.nFields;i++)
	{
		// allocate place
		fileRecords[fileIndex].file.fields[i] = (Field*)malloc(sizeof(Field*));
		// read from file
		fread(fileRecords[fileIndex].file.fields[i],sizeof(Field),1,fileRecords[fileIndex].fP); 
		// set hash key 
		fileRecords[fileIndex].file.fields[i]->id = FileRecord_HashCoding(fileRecords[fileIndex].file.fields[i]->name);
	
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
	for (i=0;i<fileRecords[0].file.header.nFields;i++)
	{
		fwrite(fileRecords[fileIndex].file.fields[i],sizeof(Field),1,fileRecords[fileIndex].fP);  
		
	}
	return OK;

}

