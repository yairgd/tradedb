clear  field;
name='test.mbt';
  field(1).name='A';
field(1).bit_length=1;
 field(1).mantisa=1;
 
   field(2).name='B';
field(2).bit_length=1;
 field(2).mantisa=-1;

 mex craeteFile.c  ..\src\db.c
 
craeteFile('test',field);