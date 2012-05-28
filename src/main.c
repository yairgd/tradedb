/*
 * =====================================================================================
 *
 *       Filename:  . main.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  13/10/11 04:25:04
 *       Revision:  none
 *       Compiler:  gcc
 *:
 *         Author:  Yair Gadelov (yg), yair.gadelov@gmail.com
 *        Company:  Israel
 *
 * =====================================================================================
 */
#include "db.h"
//#include "dbfile.h"

#define BUF_SIZE 20000
//void writefloat(float v, FILE *f) {
//
 //fwrite((void*)(&v), sizeof(v), 1, f);
//}

//int readfloat(FILE *f) {
 //int v;
 //fread((void*)(&v), sizeof(v), 1, f);
 //return v;
//}
//

void CreateMBTDataFile(char *filename)
{

	Field *field;
	unsigned int fileID; 

	// create the file, set header,close file
	fileID = DB_Create(filename);

	// create header record
     	field = DBField_Create("EUR_USD",DB_LEN16,-1);
        DBHeader_AddField(fileID,field);
	field = DBField_Create("EUR_CAD",DB_LEN16,0);
        DBHeader_AddField(fileID,field);
	field = DBField_Create("EUR_JPY",DB_LEN16,1);
        DBHeader_AddField(fileID,field);

	// update file header		
	DB_WriteHeader(fileID);
	DB_WriteFields(fileID);

	// close file
	DB_Close (fileID);
}

void showFileInfo(char *filename)
{
	Field *field;
	Header *header;
	unsigned int fileID; 
	int i;
	fileID = DB_Open(filename);
	header = DB_GetHeader(fileID);
	//field = DBField_GetByName(fileID,"EUR_USD");
	printf("file name %s , total fields %d,total data %d \n",filename,header->nFields,header->totalRecords);
//	printf("file date: %d-%d-%d %d:%d:%d",header->initialTime.day
	for (i=0;i<header->nFields;i++)
	{
		field = DBField_GetByIndex(fileID,i);
		printf("%s mantisa:%f length:%d\n",field->name,field->mantisa,field->length);
	}
	DB_Close(fileID);

}

void addData()
{
	FILE_ID fileid;
	float buf1[BUF_SIZE];
	float buf2[BUF_SIZE];
	int i;

	for (i=0;i<BUF_SIZE;i++)
	{
		buf1[i]=i;
		buf2[i]=i*2;//ff
	}
	fileid = DB_Open("/home/openrisc/junk.mbt");
	
	DB_SetColumn(fileid,"EUR_USD",0);
	DB_SetColumn(fileid,"EUR_CAD",1);
////	DB_SetFilePointer(fileid,100);
//	DB_ReadBuffer(fileid);


	DB_WriteColumn(0,buf1,BUF_SIZE,0);
	DB_WriteColumn(1,buf2,BUF_SIZE,0);

////	DB_WriteBuffer(fileid);
	DB_Close(fileid);
	
}
#define BUF_SIZE1 120
#define BUF_SIZE2 2500

void readData()
{
	FILE_ID fileid;
	
	float buf1[BUF_SIZE1];
	float buf2[BUF_SIZE2];
	float buf3[BUF_SIZE1];

//	int i;
	fileid = DB_Open("/home/openrisc/junk.mbt");
	
	DB_SetColumn(fileid,"EUR_USD",2);
	DB_SetColumn(fileid,"EUR_CAD",3);
////	DB_SetFilePointer(fileid,100);
//	DB_ReadBuffer(fileid);
	DB_ReadColumn(2,buf1,BUF_SIZE1,2800);
	DB_ReadColumn(3,buf2,BUF_SIZE2,2900);

	DB_ReadColumn(3,buf3,BUF_SIZE1,1500);
	DB_Close(fileid);

}
int main()
{


    // FILE *f; 
    // int fileID,i; ede
   //  FieldType *fieldType;
    // FileHeader *fh;
//float v=1.24;

  //   f=fopen("/home/yair/testfloat","w");  
// writefloat(v, f) ;
 //fclose(f); 

  //   f=fopen("/home/yair/testfloat","r");  
//	printf("%d\n",readfloat(f));
//	fclose(f);
//return;

	// initate Data Base
	//FileClass fileClass;
     DB_Init();
     Time time;	
     time.hour=12;
     time.minutes=45;
     time.seconds=23;
     time.month=11;
     time.year=2011;
     time.msSec=233;
     CreateMBTDataFile("/home/openrisc/junk.mbt");
     showFileInfo("/home/openrisc/junk.mbt");
	addData();



	
//	fileClass = FileClass_Open("/home/yair/junk.mbt");
//	FileClass_ShowFileInfo(fileClass);

	
	readData();


//	File_Open(&fileClass,"/home/yair/junk.mbt");
//	File_SetColum(&fileClass,"EUR_USD",0);
//	File_SetColum(&fileClass,"EUR_CAD",1);




	DB_Finalize();



   //  DB_Init();

     //fh = DBHeader_Create();
	
     //fieldType = DBField_Create("EUR_USD",DB_LEN16,4);
     //DBHeader_AddField(fh,fieldType);
     //fieldType = DBField_Create("EUR_CAD",DB_LEN16,4);
     //DBHeader_AddField(fh,fieldType);

// DBFile_SetHeader(FileHeader *fh)

     
  //   fileID = DBFile_Open("/home/yair/junk");
    // DBFile_SetHeader(FileHeader *fh)

     //DBFile_SetTime(fileID, time);
     //DBFile_SetPointer(fileID, 0);
     //for (i = 0; i<10; i++)
     //{
    // }
     //DBFile_Close(fileID);


   //  fwrite(&fh,sizeof(struct _FileHeader),1,f);    
    // fclose (f);
     
        //f=fopen("/mnt/win_c/yair/junk","r");    
       // fread(&fh1,sizeof(struct _FileHeader),1,f);      
      //  printf("%d:%d:%d\n",fh1.hour,fh1.minutes,fh1.seconds);
return 0;
}


