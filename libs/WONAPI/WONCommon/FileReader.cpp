#include "FileReader.h"
#include "LittleEndian.h"

using namespace WONAPI;

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void FileReader::Init()
{
	mFile = NULL;
	mMemDataP = NULL;
	mMemDataLen = 0;
	mMemDataPos = 0;
	mOwnMemData = false;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
FileReader::FileReader()
{
	Init();
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
FileReader::FileReader(const char *theFilePath)
{
	Init();
	Open(theFilePath);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
FileReader::FileReader(const void *theMemDataP, unsigned long theMemDataLen)
{
	Init();
	Open(theMemDataP,theMemDataLen);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
FileReader::~FileReader()
{
	if(mFile!=NULL)
		fclose(mFile);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
bool FileReader::Open(const char *theFilePath)
{
	Close();
	if(theFilePath[0]=='\0') // MS fopen.c asserts if theFilePath is empty
		return false;

	mFile = fopen(theFilePath,"rb");
	return mFile!=NULL;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void FileReader::Open(const void *theMemDataP, unsigned long theMemDataLen)
{
	Close();
	mMemDataP = (const char*)theMemDataP;
	mMemDataLen = theMemDataLen;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void FileReader::Close()
{
	if(mMemDataP!=NULL)
	{
		if(mOwnMemData)
			delete [] (char*)mMemDataP;
		
		mMemDataP = NULL;
		mMemDataLen = 0;
		mMemDataPos = 0;
		mOwnMemData = false;
	}

	if(mFile!=NULL)
	{
		fclose(mFile);
		mFile = NULL;
	}
}


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
unsigned long FileReader::pos()
{
	if(mMemDataP!=NULL)
		return mMemDataPos;
	else if(mFile!=NULL)
		return ftell(mFile);
	else
		return 0;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void FileReader::SetPos(unsigned long thePos)
{
	if(mMemDataP!=NULL)
	{
		if(thePos >= mMemDataLen)
			throw FileReaderException("Attempt to setpos past end of mem buffer.");

		mMemDataPos = thePos;
	}
	else if(mFile!=NULL)
	{
		if(fseek(mFile,thePos,SEEK_SET)!=0)
			throw FileReaderException("Attempt to setpos past end of file.");
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
int FileReader::ReadMaxBytes(void *theBuf, int theMaxBytes)
{
	if(mMemDataP!=NULL)
	{
		int aNumBytes = mMemDataLen - mMemDataPos;
		if(aNumBytes > theMaxBytes)
			aNumBytes = theMaxBytes;

		memcpy(theBuf, mMemDataP + mMemDataPos, aNumBytes);
		mMemDataPos += aNumBytes;
		return aNumBytes;
	}
	else
	{
		if(mFile==NULL)
			throw FileReaderException("NULL File.");

		int aNumBytes = (int)fread(theBuf,1,theMaxBytes,mFile);
		return aNumBytes;
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void FileReader::ReadBytes(void *theBuf, unsigned long theNumBytes)
{
	if(mMemDataP!=NULL)
	{
		if(mMemDataPos + theNumBytes > mMemDataLen)
			throw FileReaderException("Attempt to read past end of mem buffer.");

		memcpy(theBuf, mMemDataP + mMemDataPos, theNumBytes);
		mMemDataPos += theNumBytes;
	}
	else
	{
		if(mFile==NULL)
			throw FileReaderException("NULL File.");

		if(fread(theBuf,1,theNumBytes,mFile)!=theNumBytes)
			throw FileReaderException("Attempt to read past end of file.");
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void FileReader::SkipBytes(unsigned long theNumBytes)
{
	if(mMemDataP!=NULL)
	{
		if(mMemDataPos + theNumBytes > mMemDataLen)
			throw FileReaderException("Attempt to read past end of mem buffer.");

		mMemDataPos += theNumBytes;
	}
	else
	{
		if(mFile==NULL)
			throw FileReaderException("NULL File.");

		if(fseek(mFile,theNumBytes,SEEK_CUR)!=0)
			throw FileReaderException("Attempt to read past end of file.");
	}
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
void FileReader::ReadIntoMemory()
{
	if(mMemDataP!=NULL) // already in memory
		return;

	if(mFile==NULL) // no file
		return;

	// find file length
	int aPos = pos();
	fseek(mFile,0,SEEK_END);
	DWORD aNumBytes = pos();
	SetPos(0);

	char *aBytes = new char[aNumBytes];	
	ReadBytes(aBytes,aNumBytes);
	Open(aBytes,aNumBytes);
	mOwnMemData = true;
	SetPos(aPos);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
unsigned char FileReader::ReadByte()
{
	unsigned char aByte;
	ReadBytes(&aByte,1);
	return aByte;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
unsigned short FileReader::ReadShort()
{
	unsigned short aShort;
	ReadBytes(&aShort,2);
	return ShortFromLittleEndian(aShort);
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
unsigned long FileReader::ReadLong()
{
	unsigned long aLong;
	ReadBytes(&aLong,4);
	return LongFromLittleEndian(aLong);
}
