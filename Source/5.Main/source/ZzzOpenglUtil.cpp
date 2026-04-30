///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ZzzOpenglUtil.h"
#include "ZzzTexture.h"
#include "ZzzBMD.h"
#include "ZzzInfomation.h"
#include "zzzObject.h"
#include "zzzcharacter.h"
#include "Zzzinfomation.h"
#include "NewUISystem.h"
#if !defined(__ANDROID__) && !defined(MU_IOS)
#include "wglext.h"
#endif

extern float g_fScreenRate_x;
extern float g_fScreenRate_y;
extern int DisplayWinCDepthBox;
extern int DisplayWin;
extern int DisplayHeight;
extern int DisplayWinMid;
extern int DisplayHeightExt;
extern int DisplayWinExt;
extern int DisplayWinReal;

int     OpenglWindowX;     
int     OpenglWindowY;     
int     OpenglWindowWidth; 
int     OpenglWindowHeight;
bool    CameraTopViewEnable = false;

float    Camera3DPosY = 0;
float    Camera3DPosZ = 0;
bool    Camera3DEnable = false;
bool    Camera3DSet = false;
bool    Camera3DSetMove = false;
float Zoom3D = 0.f;

float   CameraViewNear      = 20.f;
float   CameraViewFar       = 2000.f;
float   CameraFOV           = 55.f;
vec3_t  CameraPosition;
vec3_t  CameraAngle;
float   CameraMatrix[3][4];
vec3_t  MousePosition;
vec3_t  MouseTarget;
float   g_fCameraCustomDistance = 0.f;
bool    FogEnable   = false;
GLfloat FogDensity  = 0.0004f;
GLfloat FogColor[4] = {30/256.f,20/256.f,10/256.f,};

bool _isVSyncAvailable = false;
bool _isVSyncEnabled = true;
#if !defined(__ANDROID__) && !defined(MU_IOS)
PFNWGLSWAPINTERVALEXTPROC       wglSwapIntervalEXT = nullptr;
#endif
//PFNWGLGETSWAPINTERVALEXTPROC    wglGetSwapIntervalEXT = nullptr;

unsigned int WindowWidth  = 1024;
unsigned int WindowHeight = 768;
int          MouseX       = WindowWidth/2;
int          MouseY       = WindowHeight/2;
int          BackMouseX   = MouseX;
int          BackMouseY   = MouseY;
bool         MouseLButton;
bool 		 MouseLButtonPop;
bool 		 MouseLButtonPush;
bool         MouseRButton;
bool 		 MouseRButtonPop;
bool 		 MouseRButtonPush;
bool 	   	 MouseLButtonDBClick;
bool         MouseMButton;
bool         MouseMButtonPop;
bool         MouseMButtonPush;
int          MouseWheel;
DWORD          BlockMouseWheel =0;
DWORD		 MouseRButtonPress = 0;
//===Test
int		mMAX_JOIN = 0;
int		mMAX_POINTS = 0;
int		mMAX_POINTERS = 0;
int		mMAX_EFFECTS = 0;
int		mMAX_SKILL_EFFECTS = 0;
int		mMAX_PARTICLES = 0;
//bool    showShoppingMall = false;

bool  g_RenderEff = true;

// Các biến VBO được tạo một lần
GLuint vertexVBO, texCoordVBO;
GLuint vboRenderBitMap[2]; // vboRenderBitMap cho đỉnh và tọa độ texture
bool vboRenderBitMapInitialized = false;
// Khởi tạo VBO cho vertex và texture coordinates (chỉ cần gọi một lần trong khởi tạo)
void InitVBO()
{
	// Tạo VBO cho các vertex và texture coordinates
	glGenBuffers(1, &vertexVBO);
	glGenBuffers(1, &texCoordVBO);

	// Gán kích thước cho VBO
	glBindBuffer(GL_ARRAY_BUFFER, vertexVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vec3_t) * 4, NULL, GL_DYNAMIC_DRAW); // Dữ liệu sẽ được cập nhật sau
	glBindBuffer(GL_ARRAY_BUFFER, texCoordVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 4 * 2, NULL, GL_DYNAMIC_DRAW); // Dữ liệu sẽ được cập nhật sau

	glGenBuffers(2, vboRenderBitMap);

	// Cấu hình vboRenderBitMap cho đỉnh
	glBindBuffer(GL_ARRAY_BUFFER, vboRenderBitMap[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 8, nullptr, GL_STATIC_DRAW);

	// Cấu hình vboRenderBitMap cho tọa độ texture
	glBindBuffer(GL_ARRAY_BUFFER, vboRenderBitMap[1]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 8, nullptr, GL_STATIC_DRAW);

	vboRenderBitMapInitialized = true;

}
void OpenExploper(char *Name,char *para)
{
	ShellExecute( NULL, "open", Name, para, "", SW_SHOW);
}

bool CheckID_HistoryDay ( char* Name, WORD day )
{
    typedef struct  __day_history__
    {
        char ID[MAX_ID_SIZE+1];
        WORD date;
    }dayHistory;

    FILE* fp;
    dayHistory days[100];
    int   count = 0;
    WORD  num = 0;
    bool  sameName = false;
    bool  update = true;

    if ( ( fp=fopen( "dconfig.ini", "rb" ) )!=NULL )
    {
        fread ( &num, sizeof( WORD ), 1, fp );
        
        if ( num>100 )
        {
            num = 0;
        }
        else
        {
            for ( int i=0; i<num; ++i )
            {
                fread ( days[i].ID, sizeof( char ), MAX_ID_SIZE+1, fp );
                fread ( &days[i].date, sizeof( WORD ), 1, fp );

                if ( !strcmp( days[i].ID, Name ) )
                {
                    sameName = true;
                    if ( days[i].date==day )
                    {
                        update = false;
                        break;
                    }
                    days[i].date = day;
                }
                count++;
            }
        }
        fclose ( fp );
    }

    if ( update )
    {
        if ( !sameName )
        {
            memcpy ( days[num].ID, Name, (MAX_ID_SIZE+1)*sizeof( char ) );
            days[num].date = day;

            num++;
        }

        fp = fopen ( "dconfig.ini", "wb" );

        fwrite ( &num, sizeof( WORD ), 1, fp );
        for ( int i=0; i<num; ++i )
        {
            fwrite ( days[i].ID, sizeof( char ), MAX_ID_SIZE+1, fp );
            fwrite ( &days[i].date, sizeof( WORD ), 1, fp );
        }

        fclose ( fp );
    }

//    showShoppingMall = update;

    return  update;
}

bool GrabEnable = false;
char GrabFileName[MAX_PATH];
int  GrabScreen = 0;
bool GrabFirst = false;

void SaveScreen()
{
	GrabFirst = true;

	auto Buffer = new unsigned char[WindowWidth * WindowHeight * 4];

	glReadPixels(0, 0, WindowWidth, WindowHeight, GL_RGBA, GL_UNSIGNED_BYTE, &Buffer[0]);

	auto BufferNew = new unsigned char[WindowWidth * WindowHeight * 3];

	int counter = 0;
	for (int i = 0; i < WindowWidth * WindowHeight * 4; i += 4)
	{
		BufferNew[counter + 0] = Buffer[i + 0];
		BufferNew[counter + 1] = Buffer[i + 1];
		BufferNew[counter + 2] = Buffer[i + 2];

		counter += 3;
	}
	char GrabFullPath[MAX_PATH]; // Masio
	strcpy(GrabFullPath, "Screenshots\\"); // Masio
	CreateDirectoryA(GrabFullPath,0);
	strcat(GrabFullPath, GrabFileName); // Masio
	WriteJpeg(GrabFullPath, WindowWidth, WindowHeight, &BufferNew[0], 100);

	SAFE_DELETE_ARRAY(Buffer);
	SAFE_DELETE_ARRAY(BufferNew);

	GrabScreen++;
	GrabScreen %= 10000;
}

float PerspectiveX;
float PerspectiveY;
int   ScreenCenterX;
int   ScreenCenterY;
int   ScreenCenterYFlip;

void GetOpenGLMatrix(float Matrix[3][4])
{
	float OpenGLMatrix[16];
	glGetFloatv(GL_MODELVIEW_MATRIX,OpenGLMatrix);
	for(int i=0;i<3;i++)
	{
		for(int j=0;j<4;j++)
		{
			Matrix[i][j] = OpenGLMatrix[j*4+i];
		}
	}
}

void gluPerspective2(float Fov,float Aspect,float ZNear,float ZFar)
{
    gluPerspective(Fov,Aspect,ZNear,ZFar);

	ScreenCenterX      = OpenglWindowX + OpenglWindowWidth/2;
	ScreenCenterY      = OpenglWindowY + OpenglWindowHeight/2;
	ScreenCenterYFlip  = WindowWidth - ScreenCenterY;

	float AspectY = (float)(WindowHeight)/(float)(OpenglWindowHeight);
	PerspectiveX = tanf(Fov*0.5f*3.141592f/180.f)/(float)(OpenglWindowWidth /2)*Aspect;
	PerspectiveY = tanf(Fov*0.5f*3.141592f/180.f)/(float)(OpenglWindowHeight/2)*AspectY;
	//PerspectiveX = (float)ScreenCenterX/tanf(Fov*0.5f*3.141592f/180.f)*Aspect;
	//PerspectiveY = (float)ScreenCenterY/tanf(Fov*0.5f*3.141592f/180.f);
}
//=== Test WIDE
DWORD dword_ConvertX(float a1)
{
#if(WIDE_SCREEN)
	return (DWORD)(g_fScreenRate_x * a1);
#else
	return a1 * WindowWidth / 640;
#endif
	
}
DWORD dword_ConvertY(float a1)
{
#if(WIDE_SCREEN)
	return (DWORD)(g_fScreenRate_y * a1);
#else
	return a1 * WindowHeight / 480;;
#endif
	
}
void CreateScreenVector ( int sx, int sy, vec3_t Target, bool bFixView )
{
#if(WIDE_SCREEN)	
	sx = dword_ConvertX(sx); //fix WIDE
	sy = dword_ConvertY(sy); //fix WIDE
#else
	sx = sx * WindowWidth / 640;
	sy = sy * WindowHeight / 480;
#endif
	vec3_t p1,p2;
    if ( bFixView )
    {
	    p1[0] =  (float)(sx-ScreenCenterX) * CameraViewFar * PerspectiveX;
	    p1[1] = -(float)(sy-ScreenCenterY) * CameraViewFar * PerspectiveY;
	    p1[2] = -CameraViewFar;
    }
    else
    {
	    p1[0] =  (float)(sx-ScreenCenterX) * RENDER_ITEMVIEW_FAR * PerspectiveX;
	    p1[1] = -(float)(sy-ScreenCenterY) * RENDER_ITEMVIEW_FAR * PerspectiveY;
	    p1[2] = -RENDER_ITEMVIEW_FAR;
    }

	p2[0] = -CameraMatrix[0][3];
	p2[1] = -CameraMatrix[1][3];
	p2[2] = -CameraMatrix[2][3];
	VectorIRotate(p2,CameraMatrix,MousePosition);
	VectorIRotate(p1,CameraMatrix,p2);
	VectorAdd(MousePosition,p2,Target);
}

void Projection(vec3_t Position,int *sx,int *sy)
{
	vec3_t TrasformPosition;
	VectorTransform(Position,CameraMatrix,TrasformPosition);
	*sx = -(int)(TrasformPosition[0] / PerspectiveX / TrasformPosition[2]) + ScreenCenterX;
	*sy =  (int)(TrasformPosition[1] / PerspectiveY / TrasformPosition[2]) + ScreenCenterY;
	//==Test WIDE
#if(WIDE_SCREEN)	
	*sx = *sx* DisplayWin /(int)WindowWidth;
	*sy = *sy* DisplayHeight /(int)WindowHeight;
#else
	*sx = *sx * 640 / (int)WindowWidth;
	*sy = *sy * 480 / (int)WindowHeight;
#endif


}

void TransformPosition(vec3_t Position,vec3_t WorldPosition,int *x,int *y)
{
	vec3_t Temp;
	VectorSubtract(Position,CameraPosition,Temp);
	VectorRotate(Temp,CameraMatrix,WorldPosition);

	*x = (int)(WorldPosition[0]/PerspectiveX/-WorldPosition[2]) + (ScreenCenterX);
	*y = (int)(WorldPosition[1]/PerspectiveY/-WorldPosition[2]) + (ScreenCenterYFlip);
	//*y = (int)(WorldPosition[1]/PerspectiveY/-WorldPosition[2]) + (WindowHeight/2);
}

bool TestDepthBuffer(vec3_t Position)
{
	vec3_t WorldPosition;
	int x,y;
    TransformPosition(Position,WorldPosition,&x,&y);
	if(x<OpenglWindowX ||
		y<OpenglWindowY ||
		x>=(int)OpenglWindowX+OpenglWindowWidth ||
		y>=(int)OpenglWindowY+OpenglWindowHeight) return false;

	GLfloat key[3];
    glReadPixels(x,y,1,1,GL_DEPTH_COMPONENT,GL_FLOAT,key);

	float z = 1.f - CameraViewNear/-WorldPosition[2] + CameraViewNear/CameraViewFar;
	if(key[0] >= z) return true;
	return false;
}

///////////////////////////////////////////////////////////////////////////////
// opengl render util
///////////////////////////////////////////////////////////////////////////////

int  CachTexture = -1;
bool TextureEnable;
bool DepthTestEnable;
bool CullFaceEnable;
bool DepthMaskEnable;
bool AlphaTestEnable;
int  AlphaBlendType;

void BindTexture(int tex)
{
	if(CachTexture != tex)
	{
      	CachTexture = tex;
		if (tex >= 0)
		{
			BITMAP_t *b = &Bitmaps[tex];
			glBindTexture(GL_TEXTURE_2D,b->TextureNumber);
		}
		else
		{
			glBindTexture(GL_TEXTURE_2D, -1 * tex);
		}
	}
}

bool TextureStream = false;

extern  int test;
void BindTextureStream(int tex)
{
	if(CachTexture != tex)
	{
		CachTexture = tex;
		if(TextureStream)
			glEnd();
		BITMAP_t *b = &Bitmaps[tex];
		glBindTexture(GL_TEXTURE_2D,b->TextureNumber);

        glBegin(GL_TRIANGLES);
		TextureStream = true;
	}
}

void EndTextureStream()
{
	if(TextureStream)
     	glEnd();
	TextureStream = false;
}

void EnableDepthTest()
{
    if(!DepthTestEnable) 
	{
		DepthTestEnable = true;
        glEnable(GL_DEPTH_TEST);
	}
}

void DisableDepthTest()
{
    if(DepthTestEnable) 
	{
		DepthTestEnable = false;
        glDisable(GL_DEPTH_TEST);
	}
}

void EnableDepthMask()
{
    if(!DepthMaskEnable) 
	{
		DepthMaskEnable = true;
     	glDepthMask(true);
	}
}

void DisableDepthMask()
{
    if(DepthMaskEnable) 
	{
		DepthMaskEnable = false;
     	glDepthMask(false);
	}
}

void EnableCullFace()
{
    if(!CullFaceEnable) 
	{
		CullFaceEnable = true;
        glEnable(GL_CULL_FACE);
	}
}

void DisableCullFace()
{
    if(CullFaceEnable) 
	{
		CullFaceEnable = false;
        glDisable(GL_CULL_FACE);
	}
}

void DisableTexture( bool AlphaTest )
{
    EnableDepthMask();
    if ( AlphaTest==true )
    {
        if(!AlphaTestEnable) 
	    {
		    AlphaTestEnable = true;
	        glEnable(GL_ALPHA_TEST);
	    }
    }
    else
    {
        if(AlphaTestEnable) 
	    {
		    AlphaTestEnable = false;
	        glDisable(GL_ALPHA_TEST);
	    }
    }
    if(TextureEnable) 
	{
		TextureEnable = false;
		glDisable(GL_TEXTURE_2D);
	}
}

void DisableAlphaBlend()
{
    if(AlphaBlendType != 0) 
	{
		AlphaBlendType = 0;
		glDisable(GL_BLEND);
	}
    EnableCullFace();
    EnableDepthMask();
    if(AlphaTestEnable) 
	{
		AlphaTestEnable = false;
	    glDisable(GL_ALPHA_TEST);
	}
    if(!TextureEnable) 
	{
		TextureEnable = true;
		glEnable(GL_TEXTURE_2D);
	}
	if(FogEnable)
		glEnable(GL_FOG);
}

void EnableAlphaTest(bool DepthMask)
{
    if(AlphaBlendType != 2)
	{
		AlphaBlendType = 2;
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
	}
    DisableCullFace();
	if(DepthMask)
        EnableDepthMask();
    if(!AlphaTestEnable) 
	{
		AlphaTestEnable = true;
	    glEnable(GL_ALPHA_TEST);
	}
    if(!TextureEnable) 
	{
		TextureEnable = true;
		glEnable(GL_TEXTURE_2D);
	}
	if(FogEnable)
		glEnable(GL_FOG);
}

void EnableAlphaBlend()
{
    if(AlphaBlendType != 3)
	{
		AlphaBlendType = 3;
		glEnable(GL_BLEND);
		glBlendFunc(GL_ONE,GL_ONE);
	}
    DisableCullFace();
    DisableDepthMask();
    if(AlphaTestEnable) 
	{
		AlphaTestEnable = false;
	    glDisable(GL_ALPHA_TEST);
	}
    if(!TextureEnable) 
	{
		TextureEnable = true;
		glEnable(GL_TEXTURE_2D);
	}
	if(FogEnable)
		glDisable(GL_FOG);
}

void EnableAlphaBlendMinus()
{
    if(AlphaBlendType != 4)
	{
		AlphaBlendType = 4;
		glEnable(GL_BLEND);
		glBlendFunc(GL_ZERO,GL_ONE_MINUS_SRC_COLOR);
	}
    DisableCullFace();
    DisableDepthMask();
    if(AlphaTestEnable) 
	{
		AlphaTestEnable = false;
	    glDisable(GL_ALPHA_TEST);
	}
    if(!TextureEnable) 
	{
		TextureEnable = true;
		glEnable(GL_TEXTURE_2D);
	}
	if(FogEnable)
		glEnable(GL_FOG);
}

void EnableAlphaBlend2()
{
    if(AlphaBlendType != 5)
	{
		AlphaBlendType = 5;
		glEnable(GL_BLEND);
		glBlendFunc(GL_ONE_MINUS_SRC_COLOR,GL_ONE);
	}
    DisableCullFace();
    DisableDepthMask();
    if(AlphaTestEnable) 
	{
		AlphaTestEnable = false;
	    glDisable(GL_ALPHA_TEST);
	}
    if(!TextureEnable) 
	{
		TextureEnable = true;
		glEnable(GL_TEXTURE_2D);
	}
	if(FogEnable)
		glEnable(GL_FOG);
}

void EnableAlphaBlend3()
{
    if(AlphaBlendType != 6)
	{
		AlphaBlendType = 6;
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
	}
    DisableCullFace();
    DisableDepthMask();
    if(AlphaTestEnable) 
	{
		AlphaTestEnable = false;
	    glDisable(GL_ALPHA_TEST);
	}
    if(!TextureEnable) 
	{
		TextureEnable = true;
		glEnable(GL_TEXTURE_2D);
	}
	if(FogEnable)
		glEnable(GL_FOG);
}

void EnableAlphaBlend4()
{
    if(AlphaBlendType != 7)
	{
		AlphaBlendType = 7;
		glEnable(GL_BLEND);
		glBlendFunc(GL_ONE,GL_ONE_MINUS_SRC_COLOR);
	}
    DisableCullFace();
    DisableDepthMask();
    if(AlphaTestEnable) 
	{
		AlphaTestEnable = false;
	    glDisable(GL_ALPHA_TEST);
	}
    if(!TextureEnable) 
	{
		TextureEnable = true;
		glEnable(GL_TEXTURE_2D);
	}
	if(FogEnable)
		glEnable(GL_FOG);
}

void EnableLightMap()
{
    if(AlphaBlendType != 1)
	{
		AlphaBlendType = 1;
		glEnable(GL_BLEND);
		glBlendFunc(GL_ZERO,GL_SRC_COLOR);
	}
    EnableCullFace();
    EnableDepthMask();
    if(AlphaTestEnable) 
	{
		AlphaTestEnable = false;
	    glDisable(GL_ALPHA_TEST);
	}
    if(!TextureEnable) 
	{
		TextureEnable = true;
		glEnable(GL_TEXTURE_2D);
	}
	if(FogEnable)
		glEnable(GL_FOG);
}

void glViewport2(int x,int y,int Width,int Height)
{
   	OpenglWindowX      = x;
   	OpenglWindowY      = y;
   	OpenglWindowWidth  = Width;
   	OpenglWindowHeight = Height;
    glViewport(x,WindowHeight-(y+Height),Width,Height);
}


float ConvertX(float x)
{
	//=== Test WIDE
	//return x*(float)WindowWidth/640.f;
#if(WIDE_SCREEN)	
	return (float)((double)x * g_fScreenRate_x);
#else
	return x * (float)WindowWidth / 640.f;
#endif
}

float ConvertY(float y)
{
	//return y*(float)WindowHeight/480.f;
#if(WIDE_SCREEN)	
	return (float)((double)y * g_fScreenRate_y);
#else
	return y * (float)WindowHeight / 480.f;
#endif
}

void BeginOpengl(int x,int y,int Width,int Height )
{
#if(WIDE_SCREEN)	
	x      = x*WindowWidth/ (WindowWidth / g_fScreenRate_x);
	y      = y*WindowHeight/ (WindowHeight / g_fScreenRate_y);
	Width  = Width*WindowWidth/ (WindowWidth / g_fScreenRate_x);
	Height = Height*WindowHeight/ (WindowHeight / g_fScreenRate_y);
#else
	x = x * WindowWidth / 640;
	y = y * WindowHeight / 480;
	Width = Width * WindowWidth / 640;
	Height = Height * WindowHeight / 480;
#endif
    glMatrixMode(GL_PROJECTION);
	glPushMatrix();
    glLoadIdentity();
    glViewport2(x,y,Width,Height);

	gluPerspective2(CameraFOV,((float)Width/(float)Height),CameraViewNear,CameraViewFar*1.4f);
    
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
    glLoadIdentity();
    glRotatef(CameraAngle[1],0.f,1.f,0.f);
    if(CameraTopViewEnable == false)
		glRotatef(CameraAngle[0],1.f,0.f,0.f);
    glRotatef(CameraAngle[2],0.f,0.f,1.f);
    glTranslatef(-CameraPosition[0],-CameraPosition[1],-CameraPosition[2]);

	glEnable(GL_SCISSOR_TEST);
	glScissor(x, WindowHeight - (y + Height), Width, Height);
	glClear(GL_DEPTH_BUFFER_BIT);
	glDisable(GL_SCISSOR_TEST);

    glDisable(GL_ALPHA_TEST);
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
   	glDepthMask(true);
    AlphaTestEnable = false;
	TextureEnable   = true;
	DepthTestEnable = true;
	CullFaceEnable  = true;
	DepthMaskEnable = true;
    glDepthFunc(GL_LEQUAL);
	glAlphaFunc(GL_GREATER,0.25f);
	if(FogEnable) 
	{
		glEnable(GL_FOG);
		glFogi(GL_FOG_MODE, GL_LINEAR);
		glFogf(GL_FOG_DENSITY, FogDensity);
		glFogfv(GL_FOG_COLOR, FogColor);
	}
	else
	{
		glDisable(GL_FOG);
	}

    GetOpenGLMatrix(CameraMatrix);
}

void EndOpengl()
{
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
}

void UpdateMousePositionn()
{
	vec3_t vPos;

	glLoadIdentity();
	glTranslatef(-CameraPosition[0],-CameraPosition[1],-CameraPosition[2]);
	GetOpenGLMatrix(CameraMatrix);

	Vector(-CameraMatrix[0][3], -CameraMatrix[1][3], -CameraMatrix[2][3], vPos);
	VectorIRotate(vPos, CameraMatrix, MousePosition);
}

#if (LDS_ADD_MULTISAMPLEANTIALIASING == 0)
#if !defined(__ANDROID__) && !defined(MU_IOS)
BOOL IsGLExtensionSupported(const char* extension)
{
	const size_t extlen = strlen(extension);
	const char* supported = NULL;

	// Try To Use wglGetExtensionStringARB On Current DC, If Possible
	//PROC wglGetExtString = wglGetProcAddress("wglGetExtensionsStringARB");
	auto wglGetExtString = wglGetProcAddress("wglGetExtensionsStringARB");

	if (wglGetExtString)
		supported = ((char* (__stdcall*)(HDC))wglGetExtString)(wglGetCurrentDC());

	// If That Failed, Try Standard Opengl Extensions String
	if (supported == NULL)
		supported = (char*)glGetString(GL_EXTENSIONS);

	// If That Failed Too, Must Be No Extensions Supported
	if (supported == NULL)
		return FALSE;

	// Begin Examination At Start Of String, Increment By 1 On False Match
	for (const char* p = supported; ; p++)
	{
		// Advance p Up To The Next Possible Match
		p = strstr(p, extension);

		if (p == NULL)
			return FALSE;															// No Match

		if ((p == supported || p[-1] == ' ') && (p[extlen] == '\0' || p[extlen] == ' '))
			return TRUE;															// Match
	}
}
bool WGLExtensionSupported(const char* extension_name)
{
	// this is pointer to function which returns pointer to string with list of all wgl extensions
	PFNWGLGETEXTENSIONSSTRINGEXTPROC _wglGetExtensionsStringEXT = reinterpret_cast<PFNWGLGETEXTENSIONSSTRINGEXTPROC>(wglGetProcAddress("wglGetExtensionsStringEXT"));

	if (strstr(_wglGetExtensionsStringEXT(), extension_name) == nullptr)
	{
		// string was not found
		return false;
	}

	// extension is supported
	return true;
}

#if !defined(__ANDROID__) && !defined(MU_IOS)
void InitVSync()
{
	_isVSyncAvailable = WGLExtensionSupported("WGL_EXT_swap_control");
	if (_isVSyncAvailable)
	{
		// Extension is supported, init pointers.
		wglSwapIntervalEXT = (PFNWGLSWAPINTERVALEXTPROC)wglGetProcAddress("wglSwapIntervalEXT");

		// this is another function from WGL_EXT_swap_control extension
		// wglGetSwapIntervalEXT = (PFNWGLGETSWAPINTERVALEXTPROC)wglGetProcAddress("wglGetSwapIntervalEXT");
	}

	if (wglSwapIntervalEXT == nullptr)
	{
		_isVSyncAvailable = false;
	}
}

bool IsVSyncAvailable()
{
	return _isVSyncAvailable;
}

bool IsVSyncEnabled()
{
	return _isVSyncEnabled;
}

void EnableVSync()
{
	if (!_isVSyncAvailable)
	{
		return;
	}

	wglSwapIntervalEXT(1);
	_isVSyncEnabled = true;
}

void DisableVSync()
{
	if (!_isVSyncAvailable)
	{
		return;
	}

	wglSwapIntervalEXT(0);
	_isVSyncEnabled = false;
}

#endif
#if !defined(__ANDROID__) && !defined(MU_IOS)
BOOL InitGLMultisample(HINSTANCE hInstance, HWND hWnd, PIXELFORMATDESCRIPTOR pfd, int iRequestMSAAValue, int& OutiPixelFormat)
{
	BOOL bIsGLMultisampleSupported = FALSE;

#if defined(_DEBUG)
	CheckGLError(__FILE__, __LINE__);
#endif // defined(_DEBUG)

	// See If The String Exists In WGL!
	if (!IsGLExtensionSupported("WGL_ARB_multisample"))
	{
		bIsGLMultisampleSupported = FALSE;
		return FALSE;
	}

#if defined(_DEBUG)
	CheckGLError(__FILE__, __LINE__);
#endif // defined(_DEBUG)
	// Get Our Pixel Format
	PFNWGLCHOOSEPIXELFORMATARBPROC wglChoosePixelFormatARB = (PFNWGLCHOOSEPIXELFORMATARBPROC)wglGetProcAddress("wglChoosePixelFormatARB");
	if (!wglChoosePixelFormatARB)
	{
		bIsGLMultisampleSupported = FALSE;
		return FALSE;
	}

#if defined(_DEBUG)
	CheckGLError(__FILE__, __LINE__);
#endif // defined(_DEBUG)

	// Get Our Current Device Context
	HDC hDC = GetDC(hWnd);

	int		valid;
	UINT	numFormats;
	float	fAttributes[] = { 0,0 };

	// These Attributes Are The Bits We Want To Test For In Our Sample
	// Everything Is Pretty Standard, The Only One We Want To 
	// Really Focus On Is The SAMPLE BUFFERS ARB And WGL SAMPLES
	// These Two Are Going To Do The Main Testing For Whether Or Not
	// We Support Multisampling On This Hardware.
	int iAttributes[] =
	{
		WGL_DRAW_TO_WINDOW_ARB,GL_TRUE,
			WGL_SUPPORT_OPENGL_ARB,GL_TRUE,
			WGL_ACCELERATION_ARB,WGL_FULL_ACCELERATION_ARB,
			WGL_COLOR_BITS_ARB,24,
			WGL_ALPHA_BITS_ARB,8,
			WGL_DEPTH_BITS_ARB,16,
			WGL_STENCIL_BITS_ARB,0,
			WGL_DOUBLE_BUFFER_ARB,GL_TRUE,
			WGL_SAMPLE_BUFFERS_ARB,GL_TRUE,
			WGL_SAMPLES_ARB, iRequestMSAAValue,					// xN MultiSampling (N=4,2,1)
			0,0
	};

#if defined(_DEBUG)
	CheckGLError(__FILE__, __LINE__);
#endif // defined(_DEBUG)

	// First We Check To See If We Can Get A Pixel Format For 4 Samples
	valid = wglChoosePixelFormatARB(hDC, iAttributes, fAttributes, 1, &OutiPixelFormat, &numFormats);

#if defined(_DEBUG)
	CheckGLError(__FILE__, __LINE__);
#endif // defined(_DEBUG)

	// If We Returned True, And Our Format Count Is Greater Than 1
	if (valid && numFormats >= 1)
	{
		bIsGLMultisampleSupported = TRUE;
		return bIsGLMultisampleSupported;
	}

#if defined(_DEBUG)
	CheckGLError(__FILE__, __LINE__);
#endif // defined(_DEBUG)

	// Our Pixel Format With 4 Samples Failed, Test For 2 Samples
	iAttributes[19] = 2;
	valid = wglChoosePixelFormatARB(hDC, iAttributes, fAttributes, 1, &OutiPixelFormat, &numFormats);
	if (valid && numFormats >= 1)
	{
		bIsGLMultisampleSupported = TRUE;
		return bIsGLMultisampleSupported;
	}

#if defined(_DEBUG)
	CheckGLError(__FILE__, __LINE__);
#endif // defined(_DEBUG)

	// Return The Valid Format
	return  bIsGLMultisampleSupported;
}
//
//void SetEnableMultisample()
//{
//	if( TRUE == g_bSupportedMSAA )
//	{
//		glEnable(GL_MULTISAMPLE_ARB);							// Enable Multisampling
//	}
//
//#if defined(_DEBUG)
//	CheckGLError( __FILE__, __LINE__ );
//#endif // defined(_DEBUG)
//}
//
//void SetDisableMultisample()
//{
//	if( TRUE == g_bSupportedMSAA )
//	{
//		glDisable(GL_MULTISAMPLE_ARB);							// Enable Multisampling
//	}
//
//#if defined(_DEBUG)
//	CheckGLError( __FILE__, __LINE__ );
//#endif // defined(_DEBUG)
//}

#endif
#endif
#endif // LDS_ADD_MULTISAMPLEANTIALIASING

///////////////////////////////////////////////////////////////////////////////
// render util
///////////////////////////////////////////////////////////////////////////////

inline void TEXCOORD(float *c,float u,float v)
{
	c[0] = u;
	c[1] = v;
}

void RenderBox(float Matrix[3][4])
{
	vec3_t BoundingBoxMin;
	vec3_t BoundingBoxMax;
	Vector(-10.f,-30.f,-10.f,BoundingBoxMin);
	Vector(10.f,0.f,10.f,BoundingBoxMax);

	vec3_t BoundingVertices[8];
	Vector(BoundingBoxMax[0],BoundingBoxMax[1],BoundingBoxMax[2],BoundingVertices[0]);
	Vector(BoundingBoxMax[0],BoundingBoxMax[1],BoundingBoxMin[2],BoundingVertices[1]);
	Vector(BoundingBoxMax[0],BoundingBoxMin[1],BoundingBoxMax[2],BoundingVertices[2]);
	Vector(BoundingBoxMax[0],BoundingBoxMin[1],BoundingBoxMin[2],BoundingVertices[3]);
	Vector(BoundingBoxMin[0],BoundingBoxMax[1],BoundingBoxMax[2],BoundingVertices[4]);
	Vector(BoundingBoxMin[0],BoundingBoxMax[1],BoundingBoxMin[2],BoundingVertices[5]);
	Vector(BoundingBoxMin[0],BoundingBoxMin[1],BoundingBoxMax[2],BoundingVertices[6]);
	Vector(BoundingBoxMin[0],BoundingBoxMin[1],BoundingBoxMin[2],BoundingVertices[7]);
	
	vec3_t TransformVertices[8];
	for(int j=0;j<8;j++)
	{
   		VectorTransform(BoundingVertices[j],Matrix,TransformVertices[j]);
	}
	
	glBegin(GL_QUADS);
	//glBegin(GL_LINES);
	glColor3f(0.2f,0.2f,0.2f);
	glTexCoord2f( 1.0F, 1.0F); glVertex3fv(TransformVertices[7]);
	glTexCoord2f( 1.0F, 0.0F); glVertex3fv(TransformVertices[6]);
	glTexCoord2f( 0.0F, 0.0F); glVertex3fv(TransformVertices[4]);
	glTexCoord2f( 0.0F, 1.0F); glVertex3fv(TransformVertices[5]);
	
	glColor3f(0.2f,0.2f,0.2f);
	glTexCoord2f( 0.0F, 1.0F); glVertex3fv(TransformVertices[0]);
	glTexCoord2f( 1.0F, 1.0F); glVertex3fv(TransformVertices[2]);
	glTexCoord2f( 1.0F, 0.0F); glVertex3fv(TransformVertices[3]);
	glTexCoord2f( 0.0F, 0.0F); glVertex3fv(TransformVertices[1]);
	
	glColor3f(0.6f,0.6f,0.6f);
	glTexCoord2f( 1.0F, 1.0F); glVertex3fv(TransformVertices[7]);
	glTexCoord2f( 1.0F, 0.0F); glVertex3fv(TransformVertices[3]);
	glTexCoord2f( 0.0F, 0.0F); glVertex3fv(TransformVertices[2]);
	glTexCoord2f( 0.0F, 1.0F); glVertex3fv(TransformVertices[6]);
	
	glColor3f(0.6f,0.6f,0.6f);
	glTexCoord2f( 0.0F, 1.0F); glVertex3fv(TransformVertices[0]);
	glTexCoord2f( 1.0F, 1.0F); glVertex3fv(TransformVertices[1]);
	glTexCoord2f( 1.0F, 0.0F); glVertex3fv(TransformVertices[5]);
	glTexCoord2f( 0.0F, 0.0F); glVertex3fv(TransformVertices[4]);
	
	glColor3f(0.4f,0.4f,0.4f);
	glTexCoord2f( 1.0F, 1.0F); glVertex3fv(TransformVertices[7]);
	glTexCoord2f( 1.0F, 0.0F); glVertex3fv(TransformVertices[5]);
	glTexCoord2f( 0.0F, 0.0F); glVertex3fv(TransformVertices[1]);
	glTexCoord2f( 0.0F, 1.0F); glVertex3fv(TransformVertices[3]);
	
	glColor3f(0.4f,0.4f,0.4f);
	glTexCoord2f( 0.0F, 1.0F); glVertex3fv(TransformVertices[0]);
	glTexCoord2f( 1.0F, 1.0F); glVertex3fv(TransformVertices[4]);
	glTexCoord2f( 1.0F, 0.0F); glVertex3fv(TransformVertices[6]);
	glTexCoord2f( 0.0F, 0.0F); glVertex3fv(TransformVertices[2]);
	glEnd();
}

void RenderPlane3D(float Width,float Height,float Matrix[3][4])
{
	vec3_t BoundingVertices[4];
	Vector(-Width,-Width, Height,BoundingVertices[3]);
	Vector( Width, Width, Height,BoundingVertices[2]);
	Vector( Width, Width,-Height,BoundingVertices[1]);
	Vector(-Width,-Width,-Height,BoundingVertices[0]);
	
	vec3_t TransformVertices[4];
	for(int j=0;j<4;j++)
	{
   		VectorTransform(BoundingVertices[j],Matrix,TransformVertices[j]);
	}
	
	glBegin(GL_QUADS);
	glTexCoord2f( 0.f, 1.f); glVertex3fv(TransformVertices[0]);
	glTexCoord2f( 1.f, 1.f); glVertex3fv(TransformVertices[1]);
	glTexCoord2f( 1.f, 0.f); glVertex3fv(TransformVertices[2]);
	glTexCoord2f( 0.f, 0.f); glVertex3fv(TransformVertices[3]);
	glEnd();
}

void BeginSprite()
{
	glPushMatrix();
	glLoadIdentity();
}

void EndSprite()
{
	glPopMatrix();
}


void RenderSprite(int Texture, vec3_t Position, float Width, float Height, vec3_t Light, float Rotation, float u, float v, float uWidth, float vHeight)
{
	BindTexture(Texture);

	vec3_t p2;
	VectorTransform(Position, CameraMatrix, p2);
	//VectorCopy(Position,p2);
	float x = p2[0];
	float y = p2[1];
	float z = p2[2];

	Width *= 0.5f;
	Height *= 0.5f;

	vec3_t p[4];
	if (Rotation == 0)
	{
		Vector(x - Width, y - Height, z, p[0]);
		Vector(x + Width, y - Height, z, p[1]);
		Vector(x + Width, y + Height, z, p[2]);
		Vector(x - Width, y + Height, z, p[3]);
	}
	else
	{
		vec3_t p2[4];
		Vector(-Width, -Height, z, p2[0]);
		Vector(Width, -Height, z, p2[1]);
		Vector(Width, Height, z, p2[2]);
		Vector(-Width, Height, z, p2[3]);
		vec3_t Angle;
		Vector(0.f, 0.f, Rotation, Angle);
		float Matrix[3][4];
		AngleMatrix(Angle, Matrix);
		for (int i = 0; i < 4; i++)
		{
			VectorRotate(p2[i], Matrix, p[i]);
			p[i][0] += x;
			p[i][1] += y;
		}
	}

	float c[4][2];
	TEXCOORD(c[3], u, v);
	TEXCOORD(c[2], u + uWidth, v);
	TEXCOORD(c[1], u + uWidth, v + vHeight);
	TEXCOORD(c[0], u, v + vHeight);

	glBegin(GL_QUADS);
	if (Bitmaps[Texture].Components == 3)
		glColor3fv(Light);
	else
	{
		if (Texture == BITMAP_BLOOD + 1 || Texture == BITMAP_FONT_HIT)
			glColor4f(Light[0], Light[1], Light[2], 1.f);
		else
			glColor4f(Light[0], Light[1], Light[2], Light[0]);
	}
	for (int i = 0; i < 4; i++)
	{
		glTexCoord2f(c[i][0], c[i][1]);
		glVertex3fv(p[i]);
	}
	glEnd();
}




//void RenderSprite(int Texture, vec3_t Position, float Width, float Height, vec3_t Light, float Rotation, float u, float v, float uWidth, float vHeight)
//{
//	if (vertexVBO == 0 || texCoordVBO == 0) {
//		return;
//	}
//
//	BindTexture(Texture);
//
//	vec3_t p2;
//	VectorTransform(Position, CameraMatrix, p2);
//
//	float x = p2[0];
//	float y = p2[1];
//	float z = p2[2];
//
//	Width *= 0.5f;
//	Height *= 0.5f;
//
//	vec3_t p[4];
//
//	// Tính toán các điểm góc
//	if (Rotation == 0) {
//		p[0][0] = x - Width; p[0][1] = y - Height; p[0][2] = z;
//		p[1][0] = x + Width; p[1][1] = y - Height; p[1][2] = z;
//		p[2][0] = x + Width; p[2][1] = y + Height; p[2][2] = z;
//		p[3][0] = x - Width; p[3][1] = y + Height; p[3][2] = z;
//	}
//	else {
//		vec3_t p2[4] = {
//			{-Width, -Height, z},
//			{ Width, -Height, z},
//			{ Width, Height, z},
//			{-Width, Height, z}
//		};
//		vec3_t Angle = { 0.f, 0.f, Rotation };
//		float Matrix[3][4];
//		AngleMatrix(Angle, Matrix);
//
//		for (int i = 0; i < 4; i++) {
//			VectorRotate(p2[i], Matrix, p[i]);
//			p[i][0] += x;
//			p[i][1] += y;
//		}
//	}
//
//	// Tọa độ texture
//	float c[4][2] = {
//		{u,        v + vHeight},
//		{u + uWidth, v + vHeight},
//		{u + uWidth, v},
//		{u,        v}
//	};
//
//	// Cập nhật dữ liệu vào VBO cho vertex positions
//	glBindBuffer(GL_ARRAY_BUFFER, vertexVBO);
//	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(p), p); // Cập nhật dữ liệu vertex
//
//	// Cập nhật dữ liệu vào VBO cho texture coordinates
//	glBindBuffer(GL_ARRAY_BUFFER, texCoordVBO);
//	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(c), c); // Cập nhật dữ liệu texture coordinates
//
//	// Sử dụng VBO cho vertex positions
//	glEnableClientState(GL_VERTEX_ARRAY);
//	glBindBuffer(GL_ARRAY_BUFFER, vertexVBO);
//	glVertexPointer(3, GL_FLOAT, 0, 0); // Sử dụng VBO cho vertex
//
//	// Sử dụng VBO cho texture coordinates
//	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
//	glBindBuffer(GL_ARRAY_BUFFER, texCoordVBO);
//	glTexCoordPointer(2, GL_FLOAT, 0, 0); // Sử dụng VBO cho texture coords
//
//	// Thiết lập màu cho sprite
//	if (Bitmaps[Texture].Components == 3) {
//		glColor3fv(Light);
//	}
//	else {
//		glColor4f(Light[0], Light[1], Light[2], (Texture == BITMAP_BLOOD + 1 || Texture == BITMAP_FONT_HIT) ? 1.f : Light[0]);
//	}
//
//	// Vẽ sprite từ VBO
//	glDrawArrays(GL_QUADS, 0, 4);
//
//	// Dọn dẹp trạng thái OpenGL
//	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
//	glDisableClientState(GL_VERTEX_ARRAY);
//
//	// Hủy liên kết VBO
//	glBindBuffer(GL_ARRAY_BUFFER, 0);
//}

void RenderSpriteUV(int Texture,vec3_t Position,float Width,float Height,float (*UV)[2],vec3_t Light[4],float Alpha)
{
    BindTexture(Texture);

	vec3_t p2;
	VectorTransform(Position,CameraMatrix,p2);
	float x = p2[0];
	float y = p2[1];
	float z = p2[2];
	
	Width  *= 0.5f;
	Height *= 0.5f;
	vec3_t p[4];
	Vector(x-Width, y-Height, z, p[0]);
	Vector(x+Width, y-Height, z, p[1]);
	Vector(x+Width, y+Height, z, p[2]);
	Vector(x-Width, y+Height, z, p[3]);

	glBegin(GL_QUADS);
	for(int i=0;i<4;i++)
	{
      	glColor4f(Light[i][0],Light[i][1],Light[i][2],Alpha);
		glTexCoord2f(UV[i][0],UV[i][1]);
		glVertex3fv(p[i]);
	}
	glEnd();
}

void RenderNumber(vec3_t Position, QWORD Num,vec3_t Color,float Alpha,float Scale)
{
	const float Rad = 0.01745329f;
	float sinTh = sin(Rad * (CameraAngle[2]));
	float cosTh = cos(Rad * (CameraAngle[2]));
	vec3_t p;
	VectorCopy(Position,p);
	vec3_t Light[4];
	VectorCopy(Color,Light[0]);
	VectorCopy(Color,Light[1]);
	VectorCopy(Color,Light[2]);
	VectorCopy(Color,Light[3]);
    if(Num == -1)
	{
		float UV[4][2];
		TEXCOORD(UV[0],0.f       ,32.f/32.f);
		TEXCOORD(UV[1],32.f/256.f,32.f/32.f);
		TEXCOORD(UV[2],32.f/256.f,17.f/32.f);
		TEXCOORD(UV[3],0.f       ,17.f/32.f);
		RenderSpriteUV(BITMAP_FONT+1,p,45,20,UV,Light,Alpha);
	}
	else if(Num == -2)
	{
		RenderSprite(BITMAP_FONT_HIT,p,32*Scale,20*Scale, Light[0], 0.f, 0.f, 0.f, 27.f/32.f, 15.f/16.f);	
	}
	else
	{
		unsigned long long qword = (QWORD)Num; //
		std::string str = std::to_string(qword); //
		char Text[32];
		//itoa(Num,Text,10);
		sprintf(Text, "%s", strdup(str.c_str()));
		p[0] -= strlen(Text)*5.f;
		unsigned int Length = strlen(Text);
		p[0] -= Length*Scale*0.125f;
		p[1] -= Length*Scale*0.125f;
		for(unsigned int i=0;i<Length;i++)
		{
			float UV[4][2];
			float u = (float)(Text[i]-48)*16.f/256.f;
			TEXCOORD(UV[0],u           ,16.f/32.f);
			TEXCOORD(UV[1],u+16.f/256.f,16.f/32.f);
			TEXCOORD(UV[2],u+16.f/256.f,0.f);
			TEXCOORD(UV[3],u           ,0.f);
			RenderSpriteUV(BITMAP_FONT+1,p,Scale,Scale,UV,Light,Alpha);
			//p[0] += Scale*0.5f;
			//p[1] += Scale*0.5f;
			p[0] += Scale / 0.7071067f * cosTh / 2;
			p[1] -= Scale / 0.7071067f * sinTh / 2;
		}
	}
}

float RenderNumber2D(float x,float y,int Num,float Width,float Height)
{
	char Text[32];
	itoa(Num,Text,10);
	int Length = (int)strlen(Text);
	x -= Width*Length/2;
	for(int i=0;i<Length;i++)
	{
		float u = (float)(Text[i]-48)*16.f/256.f;
      	//glColor3fv(Color);
		RenderBitmap(BITMAP_FONT+1,x,y,Width,Height,u,0.f,16.f/256.f,16.f/32.f);
		x += Width*0.7f;
	}
	return x;
}

void BeginBitmap()
{
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
    glLoadIdentity();

    glViewport(0,0,WindowWidth,WindowHeight);
    gluPerspective(CameraFOV,(WindowWidth)/((float)WindowHeight),CameraViewNear,CameraViewFar);
    
	glLoadIdentity();
    gluOrtho2D(0,WindowWidth,0,WindowHeight);

    glMatrixMode(GL_MODELVIEW);
	glPushMatrix();

    glLoadIdentity();
	DisableDepthTest();
    DisableAlphaBlend();
    if(!TextureEnable) 
	{
		TextureEnable = true;
		glEnable(GL_TEXTURE_2D);
	}
	glColor4f(1.f,1.f,1.f,1.f);
}

void EndBitmap()
{
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
}

void RenderColor(float x,float y,float Width,float Height,float Alpha,int Flag)
{
    DisableTexture();

	x = ConvertX(x);
	y = ConvertY(y);
	Width  = ConvertX(Width);
	Height = ConvertY(Height);

	float p[4][2];
	y = WindowHeight - y;

	p[0][0] = x      ;p[0][1] = y;
	p[1][0] = x      ;p[1][1] = y-Height;
	p[2][0] = x+Width;p[2][1] = y-Height;
	p[3][0] = x+Width;p[3][1] = y;

	glBegin(GL_TRIANGLE_FAN);
	for(int i=0;i<4;i++)
	{
		if(Alpha > 0.f)
		{
			if(Flag == 0)
				glColor4f(1.f,1.f,1.f,Alpha);
			else
			if(Flag == 1)
				glColor4f(0.f,0.f,0.f,Alpha);
		}
		glVertex2f(p[i][0],p[i][1]);
		if(Alpha > 0.f)
		{
			glColor4f(1.f,1.f,1.f,1.f);
		}
	}
	glEnd();
}
void EndRenderColor()
{
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	glEnable(GL_TEXTURE_2D);
}

void RenderColorBitmap(int Texture,float x,float y,float Width,float Height,float u,float v,float uWidth, float vHeight, unsigned int color)
{
	x = ConvertX(x);
	y = ConvertY(y);

	Width = ConvertX(Width);
	Height = ConvertY(Height);

    BindTexture(Texture);

	float p[4][2];

	y = WindowHeight - y;

	p[0][0] = x      ;p[0][1] = y;
	p[1][0] = x      ;p[1][1] = y-Height;
	p[2][0] = x+Width;p[2][1] = y-Height;
	p[3][0] = x+Width;p[3][1] = y;

	float c[4][2];
	TEXCOORD(c[0],u       ,v        );
	TEXCOORD(c[3],u+uWidth,v        );
	TEXCOORD(c[2],u+uWidth,v+vHeight);
	TEXCOORD(c[1],u       ,v+vHeight);

	glBegin(GL_TRIANGLE_FAN);

	for(int i=0;i<4;i++)
	{
		glColor4ub( static_cast<GLubyte>((color&0xff)),         //Rad
			        static_cast<GLubyte>((color>>8)&0xff),      //Green
					static_cast<GLubyte>((color>>16)&0xff),     //Blue
				    static_cast<GLubyte>((color>>24)&0xff) );   //Alpha

		glTexCoord2f(c[i][0],c[i][1]);
		glVertex2f(p[i][0],p[i][1]);

		glColor4f(1.f,1.f,1.f,1.f);
	}
	glEnd();
}

void RenderBitmap(int Texture,float x,float y,float Width,float Height,float u,float v,float uWidth,float vHeight,bool Scale,bool StartScale,float Alpha)
{
#if defined(__ANDROID__) || defined(MU_IOS)
	static bool s_loggedLoginLogoTexture[2] = { false, false };
	const int logoSlot = (Texture == BITMAP_LOG_IN + 16) ? 0 : ((Texture == BITMAP_LOG_IN + 17) ? 1 : -1);
	if(logoSlot >= 0 && !s_loggedLoginLogoTexture[logoSlot])
	{
		BITMAP_t *pBitmap = Bitmaps.FindTexture(Texture);
		char szDebugOutput[512];
		if(pBitmap != NULL)
		{
			sprintf(szDebugOutput, "RenderBitmap logo texture=%d texNo=%u file=%s size=%.0fx%.0f comp=%d ref=%d colorAlpha=%.3f alphaArg=%.3f",
				Texture, pBitmap->TextureNumber, pBitmap->FileName, pBitmap->Width, pBitmap->Height, pBitmap->Components, pBitmap->Ref, Alpha, Alpha);
		}
		else
		{
			sprintf(szDebugOutput, "RenderBitmap logo texture=%d missing bitmap alphaArg=%.3f", Texture, Alpha);
		}
		OutputDebugStringA(szDebugOutput);
		s_loggedLoginLogoTexture[logoSlot] = true;
	}
#endif
	if(StartScale)
	{
		x = ConvertX(x);
		y = ConvertY(y);
	}
	if(Scale)
	{
		Width = ConvertX(Width);
		Height = ConvertY(Height);
	}

    BindTexture(Texture);

#if defined(__ANDROID__) || defined(MU_IOS)
	if(Texture == BITMAP_LOG_IN + 17)
	{
		BITMAP_t *pBitmap = Bitmaps.FindTexture(Texture);
		if(pBitmap == NULL || pBitmap->TextureNumber == 0 || pBitmap->Width <= 1.0f || pBitmap->Height <= 1.0f)
		{
			return;
		}
	}
#endif

	float p[4][2];

	y = WindowHeight - y;
	
	p[0][0] = x      ;p[0][1] = y;
	p[1][0] = x      ;p[1][1] = y-Height;
	p[2][0] = x+Width;p[2][1] = y-Height;
	p[3][0] = x+Width;p[3][1] = y;

	float c[4][2];
	TEXCOORD(c[0],u       ,v        );
	TEXCOORD(c[3],u+uWidth,v        );
	TEXCOORD(c[2],u+uWidth,v+vHeight);
	TEXCOORD(c[1],u       ,v+vHeight);

	glBegin(GL_TRIANGLE_FAN);
	for(int i=0;i<4;i++)
	{
		if(Alpha > 0.f)
		{
			glColor4f(1.f,1.f,1.f,Alpha);
		}
		glTexCoord2f(c[i][0],c[i][1]);
		glVertex2f(p[i][0],p[i][1]);
		if(Alpha > 0.f)
		{
			glColor4f(1.f,1.f,1.f,1.f);
		}
	}
	glEnd();
}

//void RenderBitmap(int Texture, float x, float y, float Width, float Height, float u, float v, float uWidth, float vHeight, bool Scale, bool StartScale, float Alpha)
//{
//	if (!vboRenderBitMapInitialized)
//	{
//		return; // Khởi tạo vboRenderBitMap nếu chưa được khởi tạo
//	}
//
//	if (StartScale)
//	{
//		x = ConvertX(x);
//		y = ConvertY(y);
//	}
//	if (Scale)
//	{
//		Width = ConvertX(Width);
//		Height = ConvertY(Height);
//	}
//
//	BindTexture(Texture);
//
//	// Chuyển đổi vị trí y
//	y = WindowHeight - y;
//
//	// Tạo mảng đỉnh và tọa độ texture
//	float p[4][2] = {
//		{x, y},
//		{x, y - Height},
//		{x + Width, y - Height},
//		{x + Width, y}
//	};
//
//	float c[4][2];
//	TEXCOORD(c[0], u, v);
//	TEXCOORD(c[3], u + uWidth, v);
//	TEXCOORD(c[2], u + uWidth, v + vHeight);
//	TEXCOORD(c[1], u, v + vHeight);
//
//	// Cập nhật dữ liệu vào vboRenderBitMap
//	glBindBuffer(GL_ARRAY_BUFFER, vboRenderBitMap[0]);
//	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(p), p);
//
//	glBindBuffer(GL_ARRAY_BUFFER, vboRenderBitMap[1]);
//	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(c), c);
//
//	// Bắt đầu vẽ
//	glBindBuffer(GL_ARRAY_BUFFER, vboRenderBitMap[0]);
//	glEnableClientState(GL_VERTEX_ARRAY);
//	glVertexPointer(2, GL_FLOAT, 0, nullptr); // Dùng nullptr vì chúng ta đã bind buffer
//
//	glBindBuffer(GL_ARRAY_BUFFER, vboRenderBitMap[1]);
//	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
//	glTexCoordPointer(2, GL_FLOAT, 0, nullptr);
//
//	// Thiết lập alpha
//	if (Alpha > 0.f)
//	{
//		glColor4f(1.f, 1.f, 1.f, Alpha);
//	}
//
//	// Vẽ hình chữ nhật
//	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
//
//	// Khôi phục màu
//	if (Alpha > 0.f)
//	{
//		glColor4f(1.f, 1.f, 1.f, 1.f);
//	}
//
//	glDisableClientState(GL_VERTEX_ARRAY);
//	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
//}

void GetDrawCircle(int ID, float X, float Y, float W, float CurrenX, float CurrenY, float SetScale, int ScaleSize, int ScalePosicion, float Alpha)
{
	float v14; // ST1C_4@1
	float v15; // ST18_4@1
	GLuint vertexcount; // ST1C_4@5
	float v33; // ST1C_4@5
	GLfloat angle; // ST1C_4@5
	GLfloat texturecoord[2]; // [sp+2Ch] [bp-40h]@5
	GLfloat VertexCoord[4]; // [sp+4Ch] [bp-20h]@5

	SetScale = SetScale / 2;
	//--
	W = W / 2;
	X = X + W;
	Y = Y + W;

	if (ScalePosicion)
	{
		X = ConvertX(X);
		Y = ConvertY(Y);
	}

	if (ScaleSize)
	{
		W = ConvertX(W);
	}

	v33 = (double)WindowHeight - Y;

	vertexcount = 20;

	BindTexture(ID);

	glBegin(GL_TRIANGLE_FAN);

	angle = 2.0 * 3.1415926 / vertexcount;
	//--
	if (Alpha > 0.0)
		glColor4f(1.0, 1.0, 1.0, Alpha);
	texturecoord[0] = CurrenX;
	texturecoord[1] = CurrenY;
	glTexCoord2fv(texturecoord);
	//--
	VertexCoord[0] = X;
	VertexCoord[1] = v33;
	VertexCoord[2] = 0.0;
	VertexCoord[3] = 1.0;
	glVertex4fv(VertexCoord);

	for (int i = 0; i < vertexcount; i++)
	{
		texturecoord[0] = (std::cos(angle * i) * SetScale) + CurrenX;
		texturecoord[1] = (std::sin(angle * i) * SetScale) + CurrenY;
		glTexCoord2fv(texturecoord);
		VertexCoord[0] = (std::cos(angle * i) * W) + X;
		VertexCoord[1] = (std::sin(angle * i) * -W) + v33;
		VertexCoord[2] = 0.0;
		VertexCoord[3] = 1.0;
		glVertex4fv(VertexCoord);
	}
	//--
	texturecoord[0] = (1.0 * SetScale) + CurrenX;
	texturecoord[1] = (0.0 * SetScale) + CurrenY;
	glTexCoord2fv(texturecoord);
	//--
	VertexCoord[0] = (1.0 * W) + X;
	VertexCoord[1] = (0.0 * -W) + v33;
	VertexCoord[2] = 0.0;
	VertexCoord[3] = 1.0;
	glVertex4fv(VertexCoord);
	//--
	if (Alpha > 0.0)
		glColor4f(1.0, 1.0, 1.0, 1.0);
	//--
	glEnd();
}

void RenderBitmapCircle(int Texture, float x, float y, float Radius, float u, float v, float uWidth, float vHeight, bool Scale, bool StartScale, float Alpha)
{
	GLuint vertexcount; // ST1C_4@5
	float var_radius2; // ST1C_4@5
	GLfloat var_angle; // ST1C_4@5
	GLfloat TextureCoord[2]; // [sp+2Ch] [bp-40h]@5
	GLfloat VertexCoord[4]; // [sp+4Ch] [bp-20h]@5

	x = x + Radius;
	y = y + Radius;

	if (Scale)
	{
		x = ConvertX(x);
		y = ConvertY(y);
	}

	if (StartScale)
	{
		var_radius2 = ConvertY(Radius);
		Radius = ConvertX(Radius);
	}

	y = (double)(unsigned int)WindowHeight - y;

	vertexcount = 20;
	var_angle = 2.0 * Q_PI / vertexcount;

	BindTexture(Texture);

	glBegin(GL_TRIANGLE_FAN);

	if (Alpha > 0.0)
		glColor4f(1.0, 1.0, 1.0, Alpha);
	TextureCoord[0] = u;
	TextureCoord[1] = v;
	glTexCoord2fv(TextureCoord);
	//--
	VertexCoord[0] = x;
	VertexCoord[1] = y;
	VertexCoord[2] = 0.0;
	VertexCoord[3] = 1.0;
	glVertex4fv(VertexCoord);

	for (int i = 0; i < vertexcount; i++)
	{
		TextureCoord[0] = (std::cos(var_angle * i) * uWidth) + u;
		TextureCoord[1] = (std::sin(var_angle * i) * vHeight) + v;
		glTexCoord2fv(TextureCoord);
		VertexCoord[0] = (std::cos(var_angle * i) * Radius) + x;
		VertexCoord[1] = (std::sin(var_angle * i) * -var_radius2) + y;
		VertexCoord[2] = 0.0;
		VertexCoord[3] = 1.0;
		glVertex4fv(VertexCoord);
	}
	//--
	TextureCoord[0] = uWidth + u;
	TextureCoord[1] = v;
	glTexCoord2fv(TextureCoord);

	VertexCoord[0] = Radius + x;
	VertexCoord[1] = y;
	VertexCoord[2] = 0.0;
	VertexCoord[3] = 1.0;
	glVertex4fv(VertexCoord);

	if (Alpha > 0.0)
		glColor4f(1.0, 1.0, 1.0, 1.0);

	glEnd();
}

void RenderBitmapRotate(int Texture,float x,float y,float Width,float Height,float Rotate,float u,float v,float uWidth,float vHeight)
{
	x = ConvertX(x);
	y = ConvertY(y);
	Width = ConvertX(Width);
	Height = ConvertY(Height);
	//x -= Width *0.5f;
	//y -= Height*0.5f;
    BindTexture(Texture);

	vec3_t p[4],p2[4];

	y = WindowHeight - y;

	Vector(-Width*0.5f, Height*0.5f,0.f,p[0]);
	Vector(-Width*0.5f,-Height*0.5f,0.f,p[1]);
	Vector( Width*0.5f,-Height*0.5f,0.f,p[2]);
	Vector( Width*0.5f, Height*0.5f,0.f,p[3]);

	vec3_t Angle;
	Vector(0.f,0.f,Rotate,Angle);
	float Matrix[3][4];
	AngleMatrix(Angle,Matrix);

	float c[4][2];
	TEXCOORD(c[0],u       ,v        );
	TEXCOORD(c[3],u+uWidth,v        );
	TEXCOORD(c[2],u+uWidth,v+vHeight);
	TEXCOORD(c[1],u       ,v+vHeight);

	glBegin(GL_TRIANGLE_FAN);
	for(int i=0;i<4;i++)
	{
		glTexCoord2f(c[i][0],c[i][1]);
     	VectorRotate(p[i],Matrix,p2[i]);
		glVertex2f(p2[i][0]+x,p2[i][1]+y);
	}
	glEnd();
}

void RenderBitRotate(int Texture,float x,float y,float Width,float Height,float Rotate)
{
	x = ConvertX(x);
	y = ConvertY(y);
	Width = ConvertX(Width);
	Height = ConvertY(Height);

    BindTexture(Texture);

	vec3_t p[4],p2[4];

	y = Height - y;

	float cx = (Width/2.f) - (Width - x);
	float cy = (Height/2.f) - (Height - y);

	float ax = (-Width*0.5f) + cx;
	float bx = (Width*0.5f) + cx;
	float ay = (-Height*0.5f) + cy;
	float by = (Height*0.5f) + cy;

	Vector(ax, by, 0.f, p[0]);
	Vector(ax, ay, 0.f, p[1]);
	Vector(bx, ay, 0.f, p[2]);
	Vector(bx, by, 0.f, p[3]);

	vec3_t Angle;
	Vector(0.f,0.f,Rotate,Angle);
	float Matrix[3][4];
	AngleMatrix(Angle,Matrix);

	float c[4][2];
	TEXCOORD(c[0],0.f,0.f);
	TEXCOORD(c[3],1.f,0.f);
	TEXCOORD(c[2],1.f,1.f);
	TEXCOORD(c[1],0.f,1.f);

	glBegin(GL_TRIANGLE_FAN);
	for(int i=0;i<4;i++)
	{
		glTexCoord2f(c[i][0],c[i][1]);
		VectorRotate(p[i],Matrix,p2[i]);
		glVertex2f(p2[i][0]+(WindowWidth/2.f),p2[i][1]+(WindowHeight/2.f));
	}
	glEnd();
}

void RenderPointRotate(int Texture,float ix,float iy,float iWidth,float iHeight,float x,float y,float Width,float Height,float Rotate,float Rotate_Loc,float uWidth,float vHeight,int Num)
{
	int i = 0;
	vec3_t p,p2[4],p3,p4[4],Angle;
	float c[4][2],Matrix[3][4];

	ix = ConvertX(ix);
	iy = ConvertY(iy);
	x = ConvertX(x);
	y = ConvertY(y);
	Width = ConvertX(Width);
	Height = ConvertY(Height);

    BindTexture(Texture);

	y = Height - y;
	iy = Height - iy;

	Vector((ix - (Width*0.5f)) + ((Width/2.f) - (Width - x)), (iy - (Height*0.5f)) + ((Height/2.f) - (Height - y)), 0.f, p);

	Vector(0.f,0.f,Rotate,Angle);
	AngleMatrix(Angle,Matrix);

	VectorRotate(p,Matrix,p3);

	Vector(-(iWidth*0.5f), (iHeight*0.5f), 0.f, p2[0]);
	Vector(-(iWidth*0.5f), -(iHeight*0.5f), 0.f, p2[1]);
	Vector((iWidth*0.5f), -(iHeight*0.5f), 0.f, p2[2]);
	Vector((iWidth*0.5f), (iHeight*0.5f), 0.f, p2[3]);

	Vector(0.f,0.f,Rotate_Loc,Angle);
	AngleMatrix(Angle,Matrix);

	TEXCOORD(c[0],0.f       ,0.f        );
	TEXCOORD(c[3],uWidth,0.f       );
	TEXCOORD(c[2],uWidth,vHeight);
	TEXCOORD(c[1],0.f       ,vHeight);

	glBegin(GL_TRIANGLE_FAN);
	for(i=0;i<4;i++)
	{
		glTexCoord2f(c[i][0],c[i][1]);

		Matrix[0][3] = p3[0]+25;
		Matrix[1][3] = p3[1];
		VectorTransform(p2[i], Matrix, p4[i]);

		glVertex2f(p4[i][0]+(WindowWidth/2.f),p4[i][1]+(WindowHeight/2.f));
	}
	glEnd();

	if(Num > -1)
	{
		float dx,dy;
		dx = p4[0][0]+(WindowWidth/2.f);
		dy = p4[0][1]+(WindowHeight/2.f);
		dx = dx * (float)(640.f/WindowWidth);
		dy = dy * (float)(480.f/WindowHeight);
		if(Num >= 100)
		{
			g_pNewUIMiniMap->SetBtnPos(Num - 100,dx - (iWidth/2) , (480 - dy) - (iHeight/2), iWidth,iHeight);
		}
		else
		{
			g_pNewUIMiniMap->SetBtnPos(Num ,dx, 480 - dy, iWidth/2,iHeight/2);
		}
	}
}

void RenderBitmapLocalRotate(int Texture,float x,float y,float Width,float Height,float Rotate,float u,float v,float uWidth,float vHeight)
{
    BindTexture(Texture);

	vec3_t p[4];
	x = ConvertX(x);
	y = ConvertY(y);
	y = WindowHeight - y;
	Width = ConvertX(Width);
	Height = ConvertY(Height);

	vec3_t vCenter, vDir;
	Vector(x, y, 0, vCenter);
	Vector(Width*0.5f, -Height*0.5f, 0, vDir);
	p[0][0] = vCenter[0] + (vDir[0])*cosf(Rotate);
	p[0][1] = vCenter[1] + (vDir[1])*sinf(Rotate);
	p[1][0] = vCenter[0] + (vDir[0])*sinf(Rotate);
	p[1][1] = vCenter[1] - (vDir[1])*cosf(Rotate);
	p[2][0] = vCenter[0] - (vDir[0])*cosf(Rotate);
	p[2][1] = vCenter[1] - (vDir[1])*sinf(Rotate);
	p[3][0] = vCenter[0] - (vDir[0])*sinf(Rotate);
	p[3][1] = vCenter[1] + (vDir[1])*cosf(Rotate);

	float c[4][2];
	TEXCOORD(c[0],u       ,v        );
	TEXCOORD(c[3],u+uWidth,v        );
	TEXCOORD(c[2],u+uWidth,v+vHeight);
	TEXCOORD(c[1],u       ,v+vHeight);

	glBegin(GL_TRIANGLE_FAN);
	for(int i=0;i<4;i++)
	{
		glTexCoord2f(c[i][0],c[i][1]);
		glVertex2f(p[i][0],p[i][1]);
	}
	glEnd();
}

void RenderBitmapAlpha(int Texture,float sx,float sy,float Width,float Height)
{
    EnableAlphaTest();
    BindTexture(Texture);

	sy = WindowHeight - sy;
	for(int y=0;y<4;y++)
	{
		for(int x=0;x<4;x++)
		{
         	float p[4][2];
			p[0][0] = sx+((x  )*Width)*0.25f; p[0][1] = sy-((y  )*Height)*0.25f;
			p[1][0] = sx+((x  )*Width)*0.25f; p[1][1] = sy-((y+1)*Height)*0.25f;
			p[2][0] = sx+((x+1)*Width)*0.25f; p[2][1] = sy-((y+1)*Height)*0.25f;
			p[3][0] = sx+((x+1)*Width)*0.25f; p[3][1] = sy-((y  )*Height)*0.25f;
			
         	float c[4][2];
			TEXCOORD(c[0],(x  )*0.25f,(y  )*0.25f);
			TEXCOORD(c[1],(x  )*0.25f,(y+1)*0.25f);
			TEXCOORD(c[2],(x+1)*0.25f,(y+1)*0.25f);
			TEXCOORD(c[3],(x+1)*0.25f,(y  )*0.25f);

			float Alpha[4] = {1.f,1.f,1.f,1.f};
			if(x==0) {Alpha[0] = 0.f;Alpha[1] = 0.f;}
			if(x==3) {Alpha[2] = 0.f;Alpha[3] = 0.f;}
			if(y==0) {Alpha[0] = 0.f;Alpha[3] = 0.f;}
			if(y==3) {Alpha[1] = 0.f;Alpha[2] = 0.f;}
			/*if(x==0&&y==0) Alpha[0] = 0.f;
			if(x==0&&y==3) Alpha[1] = 0.f;
			if(x==3&&y==3) Alpha[2] = 0.f;
			if(x==3&&y==0) Alpha[3] = 0.f;*/
			
			glBegin(GL_TRIANGLE_FAN);
			for(int i=0;i<4;i++)
			{
      			glColor4f(1.f,1.f,1.f,Alpha[i]);
				glTexCoord2f(c[i][0],c[i][1]);
				glVertex2f(p[i][0],p[i][1]);
			}
			glEnd();
		}
	}
}
extern int DisplayWinCDepthBox;
extern int DisplayWin;
extern int DisplayHeight;
extern int DisplayWinMid;
extern int DisplayHeightExt;
extern int DisplayWinExt;
extern int DisplayWinReal;

void RenderBitmapUV(int Texture,float x,float y,float Width,float Height,float u,float v,float uWidth,float vHeight)
{
	return;
	//return x*(float)WindowWidth/640.f;
	//return (float)((double)x * g_fScreenRate_y);
	x = ConvertX(x);
	y = ConvertY(y);
	Width = ConvertX(Width+(float)DisplayWinCDepthBox);
	Height = ConvertY(Height);
    BindTexture(Texture);

	float p[4][2];
	y = WindowHeight - y;
	p[0][0] = x      ;p[0][1] = y;
	p[1][0] = x      ;p[1][1] = y-Height;
	p[2][0] = x+Width;p[2][1] = y-Height;
	p[3][0] = x+Width;p[3][1] = y;

	float c[4][2];
	TEXCOORD(c[0],u       ,v        +vHeight*0.25f);
	TEXCOORD(c[3],u+uWidth,v        );
	TEXCOORD(c[2],u+uWidth,v+vHeight);
	TEXCOORD(c[1],u       ,v+vHeight-vHeight*0.25f);

	glBegin(GL_TRIANGLE_FAN);
	for(int i=0;i<4;i++)
	{
		glTexCoord2f(c[i][0],c[i][1]);
		glVertex2f(p[i][0],p[i][1]);
	}
	glEnd();
}

///////////////////////////////////////////////////////////////////////////////
// collision detect util
///////////////////////////////////////////////////////////////////////////////

float absf(float a)
{
	if(a < 0.f) return -a;
	return a;
}

float minf(float a,float b)
{
	if(a > b) return b;
	return a;
}

float maxf(float a,float b)
{
	if(a > b) return a;
	return b;
}

int InsideTest(float x,float y,float z,int n,float *v1,float *v2,float *v3,float *v4,int flag,float type)
{
    if(type > 0.f) 
		flag <<= 3;

    int i;
    vec3_t *vtx[4];
	vtx[0] = (vec3_t *)v1;
	vtx[1] = (vec3_t *)v2;
	vtx[2] = (vec3_t *)v3;
	vtx[3] = (vec3_t *)v4;

    int j = n-1;
    switch(flag)
	{
	case 1:
        for(i=0;i<n;j=i,i++)
		{
			float d = ((*vtx[i])[1]-y) * ((*vtx[j])[2]-z) - ((*vtx[j])[1]-y) * ((*vtx[i])[2]-z);
			if(d <= 0.f) 
				return false;
		}
		break;
	case 2:
        for(i=0;i<n;j=i,i++)
		{
			float d = ((*vtx[i])[2]-z) * ((*vtx[j])[0]-x) - ((*vtx[j])[2]-z) * ((*vtx[i])[0]-x);
			if(d <= 0.f) 
				return false;
		}
		break;
	case 4:
        for(i=0;i<n;j=i,i++)
		{
			float d = ((*vtx[i])[0]-x) * ((*vtx[j])[1]-y) - ((*vtx[j])[0]-x) * ((*vtx[i])[1]-y);
			if(d <= 0.f) 
				return false;
		}
		break;
	case 8:
        for(i=0;i<n;j=i,i++)
		{
			float d = ((*vtx[i])[1]-y) * ((*vtx[j])[2]-z) - ((*vtx[j])[1]-y) * ((*vtx[i])[2]-z);
			if(d >= 0.f) 
				return false;
		}
		break;
	case 16:
        for(i=0;i<n;j=i,i++)
		{
			float d = ((*vtx[i])[2]-z) * ((*vtx[j])[0]-x) - ((*vtx[j])[2]-z) * ((*vtx[i])[0]-x);
			if(d >= 0.f) 
				return false;
		}
		break;
	case 32:
        for(i=0;i<n;j=i,i++)
		{
			float d = ((*vtx[i])[0]-x) * ((*vtx[j])[1]-y) - ((*vtx[j])[0]-x) * ((*vtx[i])[1]-y);
			if(d >= 0.f) 
				return false;
		}
		break;
	}
	
    return true;
}

float Distance;

void InitCollisionDetectLineToFace()
{
	Distance = 9999999.f;
}

vec3_t CollisionPosition;

bool CollisionDetectLineToFace(vec3_t Position,vec3_t Target,int Polygon,float *v1,float *v2,float *v3,float *v4,vec3_t Normal,bool Collision)
{
	vec3_t Direction;
	VectorSubtract(Target,Position,Direction);
	float a = DotProduct(Direction,Normal);
	if(a >= 0.f) return false;
	float b = DotProduct(Position,Normal) - DotProduct(v1,Normal);
	float t = -b/a;
	if(t >= 0.f && t <= Distance)
	{
		float X = Direction[0] * t + Position[0];
		float Y = Direction[1] * t + Position[1];
		float Z = Direction[2] * t + Position[2];
		int Count = 0;
		float MIN = minf(minf(absf(Direction[0]),absf(Direction[1])),absf(Direction[2]));
		if(MIN == absf(Direction[0]))
		{
			if( (Y >= minf(Position[1],Target[1]) && Y <= maxf(Position[1],Target[1])) &&
				(Z >= minf(Position[2],Target[2]) && Z <= maxf(Position[2],Target[2])) ) Count++;
		}
		else if(MIN == absf(Direction[1]))
		{
			if( (Z >= minf(Position[2],Target[2]) && Z <= maxf(Position[2],Target[2])) &&
				(X >= minf(Position[0],Target[0]) && X <= maxf(Position[0],Target[0])) ) Count++;
		}
		else
		{
			if( (X >= minf(Position[0],Target[0]) && X <= maxf(Position[0],Target[0])) &&
				(Y >= minf(Position[1],Target[1]) && Y <= maxf(Position[1],Target[1])) ) Count++;
		}
		if(Count == 0) return false;
		Count = 0;
		if(Normal[0]<=-0.5f || Normal[0]>=0.5f)
		{
			Count += InsideTest(X,Y,Z,Polygon,v1,v2,v3,v4,1,Normal[0]);
		}
		else if(Normal[1]<=-0.5f || Normal[1]>=0.5f)
		{
			Count += InsideTest(X,Y,Z,Polygon,v1,v2,v3,v4,2,Normal[1]);
		}
		else
		{
			Count += InsideTest(X,Y,Z,Polygon,v1,v2,v3,v4,4,Normal[2]);
		}
		if(Count == 0) return false;
		if(Collision)
		{
      		Distance = t;
			Vector(X,Y,Z,CollisionPosition);
		}
		return true;
	}
	return false;
}

bool ProjectLineBox(vec3_t ax, vec3_t p1, vec3_t p2, OBB_t obb)
{
	float P1 = DotProduct(ax, p1);
	float P2 = DotProduct(ax, p2);
	
	float mx1 = maxf(P1, P2);
	float mn1 = minf(P1, P2);
	
	float ST = DotProduct(ax, obb.StartPos);
	float Q1 = DotProduct(ax, obb.XAxis);
	float Q2 = DotProduct(ax, obb.YAxis);
	float Q3 = DotProduct(ax, obb.ZAxis);
	
	float mx2 = ST;
	float mn2 = ST;
	
	if (Q1>0)	mx2+=Q1; else mn2+=Q1;
	if (Q2>0)	mx2+=Q2; else mn2+=Q2;
	if (Q3>0) mx2+=Q3; else mn2+=Q3;
	
	if (mn1 > mx2) return false;
	if (mn2 > mx1) return false;
	
	return true;
}

bool CollisionDetectLineToOBB(vec3_t p1, vec3_t p2, OBB_t obb)
{
	vec3_t e1;
	vec3_t eq11,eq12,eq13;

	VectorSubtract(p2,p1,e1);

	CrossProduct( e1, obb.XAxis, eq11);
	CrossProduct( e1, obb.YAxis, eq12);
	CrossProduct( e1, obb.ZAxis, eq13);

	if (!ProjectLineBox(eq11,p1,p2,obb) ) return false;
	if (!ProjectLineBox(eq12,p1,p2,obb) ) return false;
	if (!ProjectLineBox(eq13,p1,p2,obb) ) return false;

	if (!ProjectLineBox(obb.XAxis,p1,p2,obb) ) return false;
	if (!ProjectLineBox(obb.YAxis,p1,p2,obb) ) return false;
	if (!ProjectLineBox(obb.ZAxis,p1,p2,obb) ) return false;

	return true;
}
void SetLineColor(int iType, float fAlphaRate = 1.0f);

void BRenderTabLine(float iPos_x, float iPos_y, float iTabWidth, float iTabHeight, int iTabNum, int iSelectNum)
{
	for (int i = 0; i < iTabNum; ++i)
	{
		SetLineColor(2);
		float fRPos_x = float(iPos_x + i * iTabWidth);
		if (i == iSelectNum)
		{
			RenderColor((float)fRPos_x, (float)iPos_y, (float)iTabWidth, (float)1);
			RenderColor((float)fRPos_x - 1, (float)iPos_y, (float)1, (float)iTabHeight);
			RenderColor((float)fRPos_x + iTabWidth - 1, (float)iPos_y, (float)1, (float)iTabHeight);
			SetLineColor(5);
			RenderColor((float)fRPos_x, (float)iPos_y + 1, (float)iTabWidth - 1, (float)iTabHeight - 1);
		}
		else
		{
			RenderColor((float)fRPos_x, (float)iPos_y + 1, (float)iTabWidth-1, (float)1); //Ngang Tren
			//RenderColor((float)fRPos_x + iTabWidth - 1, (float)iPos_y + 1, (float)1, (float)iTabHeight - 1);
			RenderColor((float)fRPos_x, (float)iPos_y + iTabHeight - 1, (float)iTabWidth-1, (float)1);
			SetLineColor(6);
			RenderColor((float)fRPos_x, (float)iPos_y + 2, (float)iTabWidth - 1, (float)iTabHeight - 3);
		}
	}
}

void BDrawOutLine(int iPos_x, int iPos_y, int iWidth, int iHeight)
{
	SetLineColor(5, 0.5f);
	RenderColor((float)iPos_x + 4, (float)iPos_y + 4, (float)iWidth - 8, (float)iHeight - 8); //Nen

	SetLineColor(0);
	RenderColor((float)iPos_x, (float)iPos_y, (float)iWidth, (float)1);
	SetLineColor(1);
	RenderColor((float)iPos_x, (float)iPos_y + 1, (float)iWidth, (float)3);
	SetLineColor(2);
	RenderColor((float)iPos_x, (float)iPos_y + 4, (float)iWidth, (float)1);

	SetLineColor(2);
	RenderColor((float)iPos_x, (float)iPos_y + iHeight - 5, (float)iWidth, (float)1);
	SetLineColor(1);
	RenderColor((float)iPos_x, (float)iPos_y + iHeight - 4, (float)iWidth, (float)3);
	SetLineColor(0);
	RenderColor((float)iPos_x, (float)iPos_y + iHeight - 1, (float)iWidth, (float)1);

	SetLineColor(0);
	RenderColor((float)iPos_x, (float)iPos_y, (float)1, (float)iHeight);
	SetLineColor(1);
	RenderColor((float)iPos_x + 1, (float)iPos_y + 5, (float)3, (float)iHeight - 10);
	SetLineColor(2);
	RenderColor((float)iPos_x + 4, (float)iPos_y + 1, (float)1, (float)iHeight - 2);

	SetLineColor(2);
	RenderColor((float)iPos_x + iWidth - 5, (float)iPos_y + 1, (float)1, (float)iHeight - 2);
	SetLineColor(1);
	RenderColor((float)iPos_x + iWidth - 4, (float)iPos_y + 5, (float)3, (float)iHeight - 10);
	SetLineColor(0);
	RenderColor((float)iPos_x + iWidth - 1, (float)iPos_y, (float)1, (float)iHeight);

	SetLineColor(4);
	RenderColor((float)iPos_x + 2, (float)iPos_y + 2, (float)1, (float)1);
	RenderColor((float)iPos_x + iWidth - 3, (float)iPos_y + 2, (float)1, (float)1);
	RenderColor((float)iPos_x + iWidth - 3, (float)iPos_y + iHeight - 3, (float)1, (float)1);
	RenderColor((float)iPos_x + 2, (float)iPos_y + iHeight - 3, (float)1, (float)1);

}


bool GetRenderEffect()
{
	return g_RenderEff;
}
