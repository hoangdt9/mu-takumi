///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include <setjmp.h>
#include "ZzzTexture.h"
#include "./Utilities/Log/ErrorReport.h"
#include "WSclient.h"
#include "DSPlaySound.h"
#include "Jpeglib.h"
#include "ProtocolSend.h"

#if defined(__ANDROID__) || defined(MU_IOS)
#include <vector>
#include "turbojpeg.h"
#endif

CGlobalBitmap Bitmaps;

struct my_error_mgr 
{
	struct jpeg_error_mgr pub;	
	jmp_buf setjmp_buffer;	
};

typedef struct my_error_mgr * my_error_ptr;

METHODDEF(void) my_error_exit (j_common_ptr cinfo)
{
	my_error_ptr myerr = (my_error_ptr) cinfo->err;
	(*cinfo->err->output_message) (cinfo);
	longjmp(myerr->setjmp_buffer, 1);
}

bool WriteJpeg(char *filename,int Width,int Height,unsigned char *Buffer,int quality)
{
	struct jpeg_compress_struct cinfo;
	struct jpeg_error_mgr jerr;
	FILE * outfile;
	JSAMPROW row_pointer[1];	
	int row_stride;	
	
	cinfo.err = jpeg_std_error(&jerr);
	jpeg_create_compress(&cinfo);
	
	if ((outfile = fopen(filename, "wb")) == NULL)
	{
		//fprintf(stderr, "can't open %s\n", filename);
		//exit(1);
		return FALSE;
	}
	jpeg_stdio_dest(&cinfo, outfile);
	
	cinfo.image_width = Width; 
	cinfo.image_height = Height;
	cinfo.input_components = 3;		
	cinfo.in_color_space = JCS_RGB;
	jpeg_set_defaults(&cinfo);
	jpeg_set_quality(&cinfo, quality, TRUE);
	jpeg_start_compress(&cinfo, TRUE);
	row_stride = cinfo.image_width * 3;
	
	while (cinfo.next_scanline < cinfo.image_height)
	{
		row_pointer[0] = &Buffer[(Height-1-cinfo.next_scanline) * row_stride];
		(void) jpeg_write_scanlines(&cinfo, row_pointer, 1);
	}
	
	jpeg_finish_compress(&cinfo);
	fclose(outfile);
	jpeg_destroy_compress(&cinfo);
	return TRUE;
}

void SaveImage(int HeaderSize,char *Ext,char *filename,BYTE *PakBuffer,int Size)
{
	if(PakBuffer==NULL || Size==0)
	{
		char OpenFileName[256];
		strcpy(OpenFileName,"Data2\\");
		strcat(OpenFileName,filename);
		FILE *fp = fopen(OpenFileName,"rb");
		if(fp == NULL)
		{
			return;
		}
		fseek(fp,0,SEEK_END);
		Size = ftell(fp);
		fseek(fp,0,SEEK_SET);
		PakBuffer = new unsigned char [Size];
		fread(PakBuffer,1,Size,fp);
		fclose(fp);
	}

	char Header[24];
	memcpy(Header,PakBuffer,HeaderSize);

	char NewFileName[256];
	int iTextcnt = 0;
	for(int i=0;i<(int)strlen(filename);i++)
	{
		iTextcnt = i;
		NewFileName[i] = filename[i];
		if(filename[i]=='.') break;
	}
	NewFileName[iTextcnt+1] = NULL;
	strcat(NewFileName,Ext);
	char SaveFileName[256];
	strcpy(SaveFileName,"Data\\");
	strcat(SaveFileName,NewFileName);
	FILE *fp = fopen(SaveFileName,"wb");
	if(fp == NULL) return;
	fwrite(Header,1,HeaderSize,fp);
	fwrite(PakBuffer,1,Size,fp);
	fclose(fp);

	if(PakBuffer==NULL || Size==0)
	{
		SAFE_DELETE_ARRAY(PakBuffer);
	}
}

bool OpenJpegBuffer(char *filename,float *BufferFloat)
{
#if defined(__ANDROID__) || defined(MU_IOS)
	if(filename == NULL || BufferFloat == NULL)
	{
		return false;
	}

	char FileName[256] = { 0 };
	char NewFileName[256] = { 0 };
	int iTextcnt = 0;
	for(int i = 0; i < (int)strlen(filename) && i < 240; i++)
	{
		iTextcnt = i;
		NewFileName[i] = filename[i];
		if(filename[i] == '.') break;
	}
	NewFileName[iTextcnt + 1] = '\0';
	strcpy(FileName, "Data\\");
	strcat(FileName, NewFileName);
	strcat(FileName, "OZJ");

	FILE* infile = fopen(FileName, "rb");
	if(infile == NULL)
	{
		const size_t len = strlen(FileName);
		if(len >= 3)
		{
			FileName[len - 3] = 'o';
			FileName[len - 2] = 'z';
			FileName[len - 1] = 'j';
			infile = fopen(FileName, "rb");
		}
	}
	if(infile == NULL)
	{
		char Text[256];
		sprintf(Text, "%s - File not exist.", FileName);
		g_ErrorReport.Write(Text);
		g_ErrorReport.Write("\r\n");
		MessageBox(g_hWnd, Text, NULL, MB_OK);
		SendMessage(g_hWnd, WM_DESTROY, 0, 0);
		return false;
	}

	fseek(infile, 0, SEEK_END);
	const long fileSize = ftell(infile);
	if(fileSize <= 24)
	{
		fclose(infile);
		return false;
	}
	fseek(infile, 24, SEEK_SET);

	const size_t jpegSize = (size_t)(fileSize - 24);
	std::vector<unsigned char> jpegBuffer(jpegSize);
	if(fread(jpegBuffer.data(), 1, jpegBuffer.size(), infile) != jpegBuffer.size())
	{
		fclose(infile);
		return false;
	}
	fclose(infile);

	int jpegWidth = 0;
	int jpegHeight = 0;
	int jpegSubsamp = TJSAMP_444;
	int jpegColorspace = TJCS_RGB;

	tjhandle tjHandle = tjInitDecompress();
	if(tjHandle == NULL)
	{
		return false;
	}

	if(tjDecompressHeader3(tjHandle, jpegBuffer.data(), (unsigned long)jpegBuffer.size(), &jpegWidth, &jpegHeight, &jpegSubsamp, &jpegColorspace) != 0 ||
		jpegWidth <= 0 || jpegHeight <= 0)
	{
		tjDestroy(tjHandle);
		return false;
	}

	const size_t bufferSize = (size_t)jpegWidth * (size_t)jpegHeight * 3u;
	std::vector<unsigned char> buffer(bufferSize);
	if(tjDecompress2(tjHandle, jpegBuffer.data(), (unsigned long)jpegBuffer.size(), buffer.data(), jpegWidth, 0, jpegHeight, TJPF_RGB, TJFLAG_BOTTOMUP) != 0)
	{
		tjDestroy(tjHandle);
		return false;
	}
	tjDestroy(tjHandle);

	for(size_t i = 0; i < bufferSize; ++i)
	{
		BufferFloat[i] = (float)buffer[i] / 255.f;
	}

	if(strstr(FileName, "TerrainLight") != NULL || strstr(FileName, "World74") != NULL)
	{
		char szDebugOutput[512];
		sprintf(szDebugOutput, "OpenJpegBuffer turbo file=%s jpeg=%dx%d firstRGB=%.3f,%.3f,%.3f",
			FileName, jpegWidth, jpegHeight, BufferFloat[0], BufferFloat[1], BufferFloat[2]);
		OutputDebugStringA(szDebugOutput);
	}
	return true;
#else
	struct jpeg_decompress_struct cinfo;
	struct my_error_mgr jerr;
	FILE * infile;		
	JSAMPARRAY buffer;	
	int row_stride;	
	
	char FileName[256];

	char NewFileName[256];
	int iTextcnt = 0;
	for(int i=0;i<(int)strlen(filename);i++)
	{
		iTextcnt = i;
		NewFileName[i] = filename[i];
		if(filename[i]=='.') break;
	}
	NewFileName[iTextcnt+1] = NULL;
	strcpy(FileName,"Data\\");
    strcat(FileName,NewFileName);
	strcat(FileName,"OZJ");

	if((infile = fopen(FileName, "rb")) == NULL) 
	{
		char Text[256];
    	sprintf(Text,"%s - File not exist.",FileName);
		g_ErrorReport.Write( Text);
		g_ErrorReport.Write( "\r\n");
		MessageBox(g_hWnd,Text,NULL,MB_OK);
		SendMessage(g_hWnd,WM_DESTROY,0,0);
		return false;
	}

	fseek(infile,24,SEEK_SET);
	
	cinfo.err = jpeg_std_error(&jerr.pub);
	jerr.pub.error_exit = my_error_exit;
	if (setjmp(jerr.setjmp_buffer)) 
	{
		jpeg_destroy_decompress(&cinfo);
		fclose(infile);
		return false;
	}
	jpeg_create_decompress(&cinfo);
	
	jpeg_stdio_src(&cinfo, infile);
	
	(void) jpeg_read_header(&cinfo, TRUE);
	
	(void) jpeg_start_decompress(&cinfo);
	row_stride = cinfo.output_width * cinfo.output_components;
	buffer = (*cinfo.mem->alloc_sarray)((j_common_ptr) &cinfo, JPOOL_IMAGE, row_stride, 1);
	
	unsigned char *Buffer = (unsigned char*) new BYTE [cinfo.output_width*cinfo.output_height*cinfo.output_components];
	while (cinfo.output_scanline < cinfo.output_height) 
	{
		(void) jpeg_read_scanlines(&cinfo, buffer, 1);
		memcpy(Buffer+(cinfo.output_height-cinfo.output_scanline)*row_stride,buffer[0],row_stride);
	}
	int Index = 0;
	for(unsigned int y=0;y<cinfo.output_height;y++)
	{
		for(unsigned int x=0;x<cinfo.output_width;x++)
		{
			BufferFloat[Index  ] = (float)Buffer[Index  ]/255.f;
			BufferFloat[Index+1] = (float)Buffer[Index+1]/255.f;
			BufferFloat[Index+2] = (float)Buffer[Index+2]/255.f;
			Index += 3;
		}
	}
	SAFE_DELETE_ARRAY(Buffer);
	
	(void) jpeg_finish_decompress(&cinfo);
	jpeg_destroy_decompress(&cinfo);
	fclose(infile);
	return true;
#endif
}

#if defined(KJH_ADD_INGAMESHOP_UI_SYSTEM) || defined(__ANDROID__)
bool LoadBitmap(const char* szFileName, GLuint uiTextureIndex, GLuint uiFilter, GLuint uiWrapMode, bool bCheck, bool bFullPath)
#else // KJH_ADD_INGAMESHOP_UI_SYSTEM || __ANDROID__
bool LoadBitmap(const char* szFileName, GLuint uiTextureIndex, GLuint uiFilter, GLuint uiWrapMode, bool bCheck)
#endif // KJH_ADD_INGAMESHOP_UI_SYSTEM || __ANDROID__
{
	char szFullPath[256] = {0, };
#if defined(KJH_ADD_INGAMESHOP_UI_SYSTEM) || defined(__ANDROID__)
	if( bFullPath == true )
	{
		strcpy(szFullPath, szFileName);
	}
	else
	{
		strcpy(szFullPath,"Data\\");
		strcat(szFullPath, szFileName);
	}	
#else // KJH_ADD_INGAMESHOP_UI_SYSTEM || __ANDROID__
	strcpy(szFullPath,"Data\\");
	strcat(szFullPath, szFileName);
#endif // KJH_ADD_INGAMESHOP_UI_SYSTEM || __ANDROID__
	if(bCheck)
	{	
		if(false == Bitmaps.LoadImage(uiTextureIndex, szFullPath, uiFilter, uiWrapMode))
		{
			char szErrorMsg[256] = {0, };
			sprintf(szErrorMsg, "LoadBitmap Failed: %s", szFullPath);
#ifdef FOR_WORK
			PopUpErrorCheckMsgBox(szErrorMsg);
#else // FOR_WORK
			PopUpErrorCheckMsgBox(szErrorMsg, true);
#endif // FOR_WORK
			return false;
		}
		return true;
	}
	return Bitmaps.LoadImage(uiTextureIndex, szFullPath, uiFilter, uiWrapMode);
}
void DeleteBitmap(GLuint uiTextureIndex, bool bForce)
{
	Bitmaps.UnloadImage(uiTextureIndex, bForce);
}
void PopUpErrorCheckMsgBox(const char* szErrorMsg, bool bForceDestroy)
{
#if(OnOffPopupMess)
	char szMsg[1024] = {0, };
	strcpy(szMsg, szErrorMsg);

	if(bForceDestroy)
	{
		MessageBox(g_hWnd, szErrorMsg, "ErrorCheckBox", MB_OK|MB_ICONERROR);
	}
	else
	{
		int iResult = MessageBox(g_hWnd, szMsg, "ErrorCheckBox", MB_YESNO|MB_ICONERROR);
		if(IDYES == iResult)
		{
			return;
		}
	}

	#ifdef NEW_PROTOCOL_SYSTEM
		gProtocolSend.DisconnectServer();
	#endif

	SocketClient.Close();
	KillGLWindow();
	DestroySound();
	DestroyWindow();
	CloseMainExe();
	ExitProcess(0);
#else 
	if (bForceDestroy)
	{
		//SendMessage(NULL, WM_DESTROY, 0, 0);
	}
	std::ofstream outFile("KEN.txt", std::ios::app);
	if (!outFile.is_open())
	{
		outFile.open("KEN.txt");
	}

	if (outFile.is_open())
	{
		outFile << szErrorMsg << std::endl;
		outFile.close();
	}
	else
	{
		MessageBox(NULL, "Khong the mo hoac tao file KEN.txt de ghi.", NULL, MB_OK);
	}
#endif
}
