#if defined(__ANDROID__) || defined(MU_IOS)

#include "MobilePlatform.h"

#include <sokol_app.h>

#include <algorithm>
#include <array>
#include <cstring>
#include <initializer_list>

#if defined(__ANDROID__)
#include <android/log.h>
#include <android/native_activity.h>
#include <jni.h>
#include <unistd.h>
#include <dirent.h>
#include <strings.h>

#include <GLES3/gl3.h>
#include <GLES2/gl2ext.h>
#include <string>
#include <vector>

#include "../_define.h"
#endif

namespace
{
std::array<Uint8, SDL_NUM_SCANCODES> g_keyboardState = {};
SDL_Rect g_textInputRect = {};
bool g_textInputActive = false;
#if defined(__ANDROID__)
bool s_loginBgMovieActive = false;
bool s_loginBgJvmStartIssued = false;
bool s_loginBgGlPendingTeardown = false;
std::string s_loginBgAbsPath;
GLuint s_loginBgOesTex = 0;
GLuint s_loginBgProg = 0;
GLuint s_loginBgVao = 0;
GLuint s_loginBgVbo = 0;
bool s_loginBgShaderBuilt = false;
#endif

bool MU_PathExists(const char* path)
{
#if defined(__ANDROID__)
    return (path != nullptr) && (path[0] != '\0') && (access(path, F_OK) == 0);
#else
    (void)path;
    return false;
#endif
}

std::string MU_GetFirstExistingPath(std::initializer_list<const char*> candidates)
{
    for (const char* candidate : candidates)
    {
        if (MU_PathExists(candidate))
        {
            return candidate;
        }
    }
    return {};
}
} // namespace

#if defined(__ANDROID__)
static void MU_AndroidCallActivityVoidMethod(const char* methodName, const char* methodSig)
{
    const void* pActivity = sapp_android_get_native_activity();
    if (pActivity == nullptr)
    {
        return;
    }

    auto* nativeActivity = static_cast<ANativeActivity*>(const_cast<void*>(pActivity));
    JavaVM* vm = nativeActivity->vm;
    JNIEnv* env = nullptr;
    const jint getEnvResult = vm->GetEnv(reinterpret_cast<void**>(&env), JNI_VERSION_1_6);
    bool needDetach = false;
    if (getEnvResult == JNI_EDETACHED)
    {
        if (vm->AttachCurrentThread(&env, nullptr) != JNI_OK || env == nullptr)
        {
            return;
        }
        needDetach = true;
    }
    else if (getEnvResult != JNI_OK || env == nullptr)
    {
        return;
    }

    jclass clazz = env->GetObjectClass(nativeActivity->clazz);
    if (clazz != nullptr)
    {
        jmethodID mid = env->GetMethodID(clazz, methodName, methodSig);
        if (mid == nullptr)
        {
            env->ExceptionClear();
        }
        else
        {
            env->CallVoidMethod(nativeActivity->clazz, mid);
        }
        if (env->ExceptionCheck())
        {
            env->ExceptionDescribe();
            env->ExceptionClear();
        }
        env->DeleteLocalRef(clazz);
    }

    if (needDetach)
    {
        vm->DetachCurrentThread();
    }
}

void MU_AndroidPlayLoginIntroMoviePath(const char* utf8PathAbs)
{
    if (utf8PathAbs == nullptr || utf8PathAbs[0] == '\0')
    {
        return;
    }

    const void* pActivity = sapp_android_get_native_activity();
    if (pActivity == nullptr)
    {
        return;
    }

    auto* nativeActivity = static_cast<ANativeActivity*>(const_cast<void*>(pActivity));
    JavaVM* vm = nativeActivity->vm;
    JNIEnv* env = nullptr;
    const jint getEnvResult = vm->GetEnv(reinterpret_cast<void**>(&env), JNI_VERSION_1_6);
    bool needDetach = false;
    if (getEnvResult == JNI_EDETACHED)
    {
        if (vm->AttachCurrentThread(&env, nullptr) != JNI_OK || env == nullptr)
        {
            return;
        }
        needDetach = true;
    }
    else if (getEnvResult != JNI_OK || env == nullptr)
    {
        return;
    }

    jclass clazz = env->GetObjectClass(nativeActivity->clazz);
    if (clazz != nullptr)
    {
        jmethodID mid = env->GetMethodID(clazz, "playLoginIntroMovie", "(Ljava/lang/String;)V");
        if (mid == nullptr)
        {
            env->ExceptionClear();
        }
        else
        {
            jstring jPath = env->NewStringUTF(utf8PathAbs);
            if (jPath != nullptr)
            {
                env->CallVoidMethod(nativeActivity->clazz, mid, jPath);
                env->DeleteLocalRef(jPath);
            }
        }
        if (env->ExceptionCheck())
        {
            env->ExceptionDescribe();
            env->ExceptionClear();
        }
        env->DeleteLocalRef(clazz);
    }

    if (needDetach)
    {
        vm->DetachCurrentThread();
    }
}

void MU_AndroidStopLoginIntroMovie()
{
    MU_AndroidCallActivityVoidMethod("stopLoginIntroMovie", "()V");
}

bool MU_AndroidIsLoginBackgroundMovieActive()
{
    return s_loginBgMovieActive;
}

void MU_AndroidMarkLoginBackgroundMovieStarted()
{
    s_loginBgMovieActive = true;
}

static void MU_AndroidLoginBgVideoShutdownGl();

void MU_AndroidMarkLoginBackgroundMovieStopped()
{
    s_loginBgMovieActive = false;
    s_loginBgJvmStartIssued = false;
    s_loginBgAbsPath.clear();
    s_loginBgGlPendingTeardown = true;
}

static GLuint MU_AndroidLoginBgCompileShader(GLenum type, const char* src)
{
    const GLuint sh = glCreateShader(type);
    if (sh == 0)
    {
        return 0;
    }
    glShaderSource(sh, 1, &src, nullptr);
    glCompileShader(sh);
    GLint ok = 0;
    glGetShaderiv(sh, GL_COMPILE_STATUS, &ok);
    if (!ok)
    {
        char log[768];
        glGetShaderInfoLog(sh, sizeof(log), nullptr, log);
        __android_log_print(ANDROID_LOG_INFO, "TakumiErrorReport", "[TakumiLoginBg] shader compile failed: %s\r\n", log);
        glDeleteShader(sh);
        return 0;
    }
    return sh;
}

static void MU_AndroidLoginBgSetupVao()
{
    if (s_loginBgVao == 0)
    {
        glGenVertexArrays(1, &s_loginBgVao);
    }
    if (s_loginBgVbo == 0)
    {
        glGenBuffers(1, &s_loginBgVbo);
    }
    glBindVertexArray(s_loginBgVao);
    glBindBuffer(GL_ARRAY_BUFFER, s_loginBgVbo);
    const float verts[] = {
        -1.f, -1.f, 0.f, 1.f,
        1.f,  -1.f, 1.f, 1.f,
        -1.f, 1.f,  0.f, 0.f,
        1.f,  1.f,  1.f, 0.f,
    };
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), reinterpret_cast<void*>(0));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), reinterpret_cast<void*>(2 * sizeof(float)));
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

static bool MU_AndroidLoginBgTryLinkProgram(GLuint vs, GLuint fs, const char* pathLabel)
{
    if (vs == 0 || fs == 0)
    {
        if (vs != 0)
        {
            glDeleteShader(vs);
        }
        if (fs != 0)
        {
            glDeleteShader(fs);
        }
        return false;
    }

    const GLuint prog = glCreateProgram();
    glAttachShader(prog, vs);
    glAttachShader(prog, fs);
    glBindAttribLocation(prog, 0, "aPos");
    glBindAttribLocation(prog, 1, "aUv");
    glLinkProgram(prog);
    GLint linked = 0;
    glGetProgramiv(prog, GL_LINK_STATUS, &linked);
    glDeleteShader(vs);
    glDeleteShader(fs);
    if (!linked)
    {
        char log[768];
        glGetProgramInfoLog(prog, sizeof(log), nullptr, log);
        __android_log_print(
            ANDROID_LOG_INFO,
            "TakumiErrorReport",
            "[TakumiLoginBg] program link failed (%s): %s\r\n",
            pathLabel,
            log);
        glDeleteProgram(prog);
        return false;
    }

    s_loginBgProg = prog;
    s_loginBgShaderBuilt = true;
    MU_AndroidLoginBgSetupVao();
    return true;
}

static bool MU_AndroidLoginBgEnsureProgram()
{
    if (s_loginBgShaderBuilt && s_loginBgProg != 0)
    {
        return true;
    }

    static const char* kVs300 = R"(#version 300 es
layout(location = 0) in vec2 aPos;
layout(location = 1) in vec2 aUv;
out vec2 vUv;
void main() {
  vUv = aUv;
  gl_Position = vec4(aPos, 0.0, 1.0);
}
)";
    static const char* kFs300 = R"(#version 300 es
#extension GL_OES_EGL_image_external_essl3 : require
precision mediump float;
in vec2 vUv;
uniform samplerExternalOES uTex;
out vec4 oFragColor;
void main() {
  oFragColor = texture(uTex, vUv);
}
)";

    GLuint vs300 = MU_AndroidLoginBgCompileShader(GL_VERTEX_SHADER, kVs300);
    GLuint fs300 = MU_AndroidLoginBgCompileShader(GL_FRAGMENT_SHADER, kFs300);
    if (vs300 != 0 && fs300 != 0 && MU_AndroidLoginBgTryLinkProgram(vs300, fs300, "GLES300+external_essl3"))
    {
        __android_log_print(
            ANDROID_LOG_INFO,
            "TakumiErrorReport",
            "[TakumiLoginBg] using GLES3 external_essl3 program for login video\r\n");
        return true;
    }
    if (vs300 == 0 || fs300 == 0)
    {
        if (vs300 != 0)
        {
            glDeleteShader(vs300);
        }
        if (fs300 != 0)
        {
            glDeleteShader(fs300);
        }
    }

    static const char* kVs100 = R"(#version 100
attribute vec2 aPos;
attribute vec2 aUv;
varying vec2 vUv;
void main() {
  vUv = aUv;
  gl_Position = vec4(aPos, 0.0, 1.0);
}
)";
    static const char* kFs100 = R"(#version 100
#extension GL_OES_EGL_image_external : require
precision mediump float;
varying vec2 vUv;
uniform samplerExternalOES uTex;
void main() {
  gl_FragColor = texture2D(uTex, vUv);
}
)";

    GLuint vs100 = MU_AndroidLoginBgCompileShader(GL_VERTEX_SHADER, kVs100);
    GLuint fs100 = MU_AndroidLoginBgCompileShader(GL_FRAGMENT_SHADER, kFs100);
    if (vs100 != 0 && fs100 != 0 && MU_AndroidLoginBgTryLinkProgram(vs100, fs100, "GLES100+external"))
    {
        __android_log_print(
            ANDROID_LOG_INFO,
            "TakumiErrorReport",
            "[TakumiLoginBg] using GLES2/100 external_OES fallback for login video\r\n");
        return true;
    }
    if (vs100 != 0 && fs100 != 0)
    {
        // shaders deleted by TryLinkProgram on failure
    }
    else
    {
        if (vs100 != 0)
        {
            glDeleteShader(vs100);
        }
        if (fs100 != 0)
        {
            glDeleteShader(fs100);
        }
    }

    __android_log_print(
        ANDROID_LOG_INFO,
        "TakumiErrorReport",
        "[TakumiLoginBg] no working OES video shader; disabling GL login movie (terrain restored)\r\n");
    return false;
}

static void MU_AndroidLoginBgVideoShutdownGl()
{
    if (s_loginBgVao != 0)
    {
        glDeleteVertexArrays(1, &s_loginBgVao);
        s_loginBgVao = 0;
    }
    if (s_loginBgVbo != 0)
    {
        glDeleteBuffers(1, &s_loginBgVbo);
        s_loginBgVbo = 0;
    }
    if (s_loginBgProg != 0)
    {
        glDeleteProgram(s_loginBgProg);
        s_loginBgProg = 0;
    }
    s_loginBgShaderBuilt = false;
    if (s_loginBgOesTex != 0)
    {
        glDeleteTextures(1, &s_loginBgOesTex);
        s_loginBgOesTex = 0;
    }
}

static bool MU_AndroidCallBindLoginBgToGlTexture(GLuint oesTex, const char* utf8PathAbs)
{
    if (utf8PathAbs == nullptr || utf8PathAbs[0] == '\0')
    {
        return false;
    }

    const void* pActivity = sapp_android_get_native_activity();
    if (pActivity == nullptr)
    {
        return false;
    }

    auto* nativeActivity = static_cast<ANativeActivity*>(const_cast<void*>(pActivity));
    JavaVM* vm = nativeActivity->vm;
    JNIEnv* env = nullptr;
    const jint getEnvResult = vm->GetEnv(reinterpret_cast<void**>(&env), JNI_VERSION_1_6);
    bool needDetach = false;
    if (getEnvResult == JNI_EDETACHED)
    {
        if (vm->AttachCurrentThread(&env, nullptr) != JNI_OK || env == nullptr)
        {
            return false;
        }
        needDetach = true;
    }
    else if (getEnvResult != JNI_OK || env == nullptr)
    {
        return false;
    }

    bool ok = false;
    jclass clazz = env->GetObjectClass(nativeActivity->clazz);
    if (clazz != nullptr)
    {
        jmethodID mid = env->GetMethodID(clazz, "bindLoginBackgroundMovieToGlTexture", "(ILjava/lang/String;)Z");
        if (mid == nullptr)
        {
            env->ExceptionClear();
            __android_log_print(
                ANDROID_LOG_INFO,
                "TakumiErrorReport",
                "[TakumiLoginBg] bindLoginBackgroundMovieToGlTexture not found on activity\r\n");
        }
        else
        {
            jstring jPath = env->NewStringUTF(utf8PathAbs);
            if (jPath != nullptr)
            {
                const jboolean jOk = env->CallBooleanMethod(nativeActivity->clazz, mid, static_cast<jint>(oesTex), jPath);
                ok = (jOk == JNI_TRUE) && !env->ExceptionCheck();
                if (!ok && env->ExceptionCheck())
                {
                    env->ExceptionDescribe();
                    env->ExceptionClear();
                }
                env->DeleteLocalRef(jPath);
            }
        }
        env->DeleteLocalRef(clazz);
    }

    if (needDetach)
    {
        vm->DetachCurrentThread();
    }
    return ok;
}

static void MU_AndroidCallUpdateLoginBgTexture()
{
    MU_AndroidCallActivityVoidMethod("updateLoginBackgroundMovieTexture", "()V");
}

static bool MU_AndroidTryResolveMovieFromDataMovieScan(char* resolved, size_t resolvedSize)
{
    if (resolved == nullptr || resolvedSize == 0)
    {
        return false;
    }

    DIR* const dir = opendir("Data/Movie");
    if (dir == nullptr)
    {
        return false;
    }

    std::vector<std::string> mp4s;
    std::vector<std::string> wmvs;
    for (dirent* ent = readdir(dir); ent != nullptr; ent = readdir(dir))
    {
        const char* const name = ent->d_name;
        if (name[0] == '.')
        {
            continue;
        }
        const size_t len = std::strlen(name);
        if (len < 5)
        {
            continue;
        }
        const char* const ext = name + (len - 4);
        if (strcasecmp(ext, ".mp4") == 0)
        {
            mp4s.emplace_back(name);
        }
        else if (strcasecmp(ext, ".wmv") == 0)
        {
            wmvs.emplace_back(name);
        }
    }
    closedir(dir);

    std::sort(mp4s.begin(), mp4s.end());
    std::sort(wmvs.begin(), wmvs.end());

    std::string pick;
    auto preferExact = [](const std::vector<std::string>& v, const char* want) -> std::string {
        for (const std::string& n : v)
        {
            if (strcasecmp(n.c_str(), want) == 0)
            {
                return n;
            }
        }
        return {};
    };

    // Same order as LoginMainWin / legacy PC: WMV before MP4 (some packs ship a broken empty MU.mp4).
    pick = preferExact(wmvs, "MU.wmv");
    if (pick.empty())
    {
        pick = preferExact(mp4s, "MU.mp4");
    }
    if (pick.empty() && !wmvs.empty())
    {
        pick = wmvs.front();
    }
    if (pick.empty() && !mp4s.empty())
    {
        pick = mp4s.front();
    }
    if (pick.empty())
    {
        return false;
    }

    char rel[512];
    std::snprintf(rel, sizeof(rel), "Data/Movie/%s", pick.c_str());
    if (access(rel, F_OK) != 0)
    {
        return false;
    }
    if (realpath(rel, resolved) != nullptr)
    {
        __android_log_print(
            ANDROID_LOG_INFO,
            "TakumiErrorReport",
            "[TakumiLoginBg] using Data/Movie scan: %s\r\n",
            rel);
        return true;
    }
    if (std::strlen(rel) < resolvedSize)
    {
        std::memcpy(resolved, rel, std::strlen(rel) + 1U);
        __android_log_print(
            ANDROID_LOG_INFO,
            "TakumiErrorReport",
            "[TakumiLoginBg] using Data/Movie scan (no realpath): %s\r\n",
            rel);
        return true;
    }
    return false;
}

void MU_AndroidTryStartLoginBackgroundMovie()
{
    if (s_loginBgMovieActive || s_loginBgJvmStartIssued)
    {
        return;
    }

    // WMV before MP4: match LoginMainWin / legacy; empty or bad MU.mp4 must not shadow MU.wmv.
    static const char* const kMovieCandidates[] = {
        MOVIE_FILE_WMV,
        MOVIE_FILE_MP4,
    };
    char resolved[8192];
    bool found = false;
    for (size_t ci = 0; ci < sizeof(kMovieCandidates) / sizeof(kMovieCandidates[0]); ++ci)
    {
        const char* src = kMovieCandidates[ci];
        char pathRel[512];
        size_t o = 0;
        for (; src[o] && o + 1 < sizeof(pathRel); ++o)
        {
            const char c = src[o];
            pathRel[o] = (c == '\\') ? '/' : c;
        }
        pathRel[o] = '\0';
        if (access(pathRel, F_OK) != 0)
        {
            continue;
        }
        if (realpath(pathRel, resolved) != nullptr)
        {
            found = true;
            break;
        }
        if (strlen(pathRel) < sizeof(resolved))
        {
            memcpy(resolved, pathRel, strlen(pathRel) + 1);
            found = true;
            break;
        }
    }
    if (!found)
    {
        found = MU_AndroidTryResolveMovieFromDataMovieScan(resolved, sizeof(resolved));
    }
    if (!found)
    {
        char cwdBuf[768];
        const char* const cwdPtr = getcwd(cwdBuf, sizeof(cwdBuf));
        __android_log_print(
            ANDROID_LOG_INFO,
            "TakumiErrorReport",
            "[TakumiLoginBg] no MU.wmv/MU.mp4 and no .wmv/.mp4 in Data/Movie (cwd=%s) — add Data/Movie/MU.wmv or valid intro\r\n",
            cwdPtr != nullptr ? cwdPtr : "(getcwd failed)");
        return;
    }

    s_loginBgAbsPath.assign(resolved);
    s_loginBgJvmStartIssued = true;
    __android_log_print(
        ANDROID_LOG_INFO,
        "TakumiErrorReport",
        "[TakumiLoginBg] pending GL login movie path=%s\r\n",
        s_loginBgAbsPath.c_str());
}

void MU_AndroidStopLoginBackgroundMovie()
{
    MU_AndroidCallActivityVoidMethod("releaseLoginBackgroundMovieGl", "()V");
    s_loginBgGlPendingTeardown = false;
    MU_AndroidLoginBgVideoShutdownGl();
    s_loginBgMovieActive = false;
    s_loginBgJvmStartIssued = false;
    s_loginBgAbsPath.clear();
}

void MU_AndroidLoginBgVideoRenderTick()
{
    if (s_loginBgGlPendingTeardown)
    {
        MU_AndroidLoginBgVideoShutdownGl();
        s_loginBgGlPendingTeardown = false;
    }

    if (!s_loginBgJvmStartIssued && !s_loginBgMovieActive)
    {
        return;
    }

    if (!s_loginBgMovieActive)
    {
        if (s_loginBgOesTex != 0)
        {
            MU_AndroidLoginBgVideoShutdownGl();
        }
        glGenTextures(1, &s_loginBgOesTex);
        glBindTexture(GL_TEXTURE_EXTERNAL_OES, s_loginBgOesTex);
        glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glBindTexture(GL_TEXTURE_EXTERNAL_OES, 0);

        if (!MU_AndroidCallBindLoginBgToGlTexture(s_loginBgOesTex, s_loginBgAbsPath.c_str()))
        {
            __android_log_print(
                ANDROID_LOG_INFO,
                "TakumiErrorReport",
                "[TakumiLoginBg] bindLoginBackgroundMovieToGlTexture failed path=%s\r\n",
                s_loginBgAbsPath.c_str());
            MU_AndroidLoginBgVideoShutdownGl();
            s_loginBgJvmStartIssued = false;
            s_loginBgAbsPath.clear();
            return;
        }
        MU_AndroidMarkLoginBackgroundMovieStarted();
        __android_log_print(
            ANDROID_LOG_INFO,
            "TakumiErrorReport",
            "[TakumiLoginBg] MediaPlayer bound to GL OES texture id=%u\r\n",
            static_cast<unsigned>(s_loginBgOesTex));
    }

    if (!s_loginBgMovieActive || s_loginBgOesTex == 0)
    {
        return;
    }

    MU_AndroidCallUpdateLoginBgTexture();

    if (!MU_AndroidLoginBgEnsureProgram())
    {
        __android_log_print(
            ANDROID_LOG_INFO,
            "TakumiErrorReport",
            "[TakumiLoginBg] GL shader unavailable; stopping login movie (restore 3D)\r\n");
        MU_AndroidCallActivityVoidMethod("releaseLoginBackgroundMovieGl", "()V");
        s_loginBgGlPendingTeardown = false;
        MU_AndroidLoginBgVideoShutdownGl();
        s_loginBgMovieActive = false;
        s_loginBgJvmStartIssued = false;
        s_loginBgAbsPath.clear();
        return;
    }

    GLint prevViewport[4];
    glGetIntegerv(GL_VIEWPORT, prevViewport);
    GLint prevProg = 0;
    glGetIntegerv(GL_CURRENT_PROGRAM, &prevProg);
    const GLboolean depthWas = glIsEnabled(GL_DEPTH_TEST);
    const GLboolean blendWas = glIsEnabled(GL_BLEND);
    const GLboolean cullWas = glIsEnabled(GL_CULL_FACE);
    const GLboolean scissorWas = glIsEnabled(GL_SCISSOR_TEST);

    glDisable(GL_SCISSOR_TEST);
    int vw = sapp_width();
    int vh = sapp_height();
    if (vw <= 0 || vh <= 0)
    {
        vw = prevViewport[2] > 0 ? prevViewport[2] : 1;
        vh = prevViewport[3] > 0 ? prevViewport[3] : 1;
    }
    glViewport(0, 0, vw, vh);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
    glDisable(GL_CULL_FACE);

    glUseProgram(s_loginBgProg);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_EXTERNAL_OES, s_loginBgOesTex);
    const GLint loc = glGetUniformLocation(s_loginBgProg, "uTex");
    if (loc >= 0)
    {
        glUniform1i(loc, 0);
    }
    glBindVertexArray(s_loginBgVao);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_EXTERNAL_OES, 0);

    if (prevProg > 0)
    {
        glUseProgram(static_cast<GLuint>(prevProg));
    }
    else
    {
        glUseProgram(0);
    }
    glViewport(prevViewport[0], prevViewport[1], prevViewport[2], prevViewport[3]);
    if (depthWas)
    {
        glEnable(GL_DEPTH_TEST);
    }
    if (blendWas)
    {
        glEnable(GL_BLEND);
    }
    if (cullWas)
    {
        glEnable(GL_CULL_FACE);
    }
    if (scissorWas)
    {
        glEnable(GL_SCISSOR_TEST);
    }
}

static void MU_AndroidSyncImeBridgeBounds(int x, int y, int w, int h)
{
    const void* pActivity = sapp_android_get_native_activity();
    if (pActivity == nullptr)
    {
        return;
    }

    auto* nativeActivity = static_cast<ANativeActivity*>(const_cast<void*>(pActivity));
    JavaVM* vm = nativeActivity->vm;
    JNIEnv* env = nullptr;
    const jint getEnvResult = vm->GetEnv(reinterpret_cast<void**>(&env), JNI_VERSION_1_6);
    bool needDetach = false;
    if (getEnvResult == JNI_EDETACHED)
    {
        if (vm->AttachCurrentThread(&env, nullptr) != JNI_OK || env == nullptr)
        {
            return;
        }
        needDetach = true;
    }
    else if (getEnvResult != JNI_OK || env == nullptr)
    {
        return;
    }

    jclass clazz = env->GetObjectClass(nativeActivity->clazz);
    if (clazz != nullptr)
    {
        jmethodID mid = env->GetMethodID(clazz, "syncImeBridgeBounds", "(IIII)V");
        if (mid == nullptr)
        {
            env->ExceptionClear();
        }
        else
        {
            env->CallVoidMethod(nativeActivity->clazz, mid, x, y, w, h);
        }
        if (env->ExceptionCheck())
        {
            env->ExceptionDescribe();
            env->ExceptionClear();
        }
        env->DeleteLocalRef(clazz);
    }

    if (needDetach)
    {
        vm->DetachCurrentThread();
    }
}
#endif

void MU_MobilePlatformInit()
{
    MU_MobileClearKeyboardState();
    g_textInputRect = {};
    g_textInputActive = false;
}

void MU_MobilePlatformShutdown()
{
    g_textInputActive = false;
    g_textInputRect = {};
    MU_MobileClearKeyboardState();
}

const Uint8* MU_MobileGetKeyboardState()
{
    return g_keyboardState.data();
}

void MU_MobileSetKeyState(SDL_Scancode scancode, bool isDown)
{
    if ((scancode >= 0) && (static_cast<size_t>(scancode) < g_keyboardState.size()))
    {
        g_keyboardState[static_cast<size_t>(scancode)] = isDown ? 1u : 0u;
    }
}

void MU_MobileClearKeyboardState()
{
    std::fill(g_keyboardState.begin(), g_keyboardState.end(), static_cast<Uint8>(0));
}

void MU_MobileStartTextInput()
{
    g_textInputActive = true;
#if defined(__ANDROID__)
    // Sokol uses ANativeActivity_showSoftInput, which often does not show IME for this app;
    // the Java bridge view receives composition and forwards to native.
    MU_AndroidCallActivityVoidMethod("showImeBridgeKeyboard", "()V");
    {
        const SDL_Rect& r = g_textInputRect;
        const int bw = (r.w > 0) ? r.w : 1;
        const int bh = (r.h > 0) ? r.h : 1;
        MU_AndroidSyncImeBridgeBounds(r.x, r.y, bw, bh);
    }
#else
    sapp_show_keyboard(true);
#endif
}

void MU_MobileStopTextInput()
{
    g_textInputActive = false;
#if defined(__ANDROID__)
    MU_AndroidCallActivityVoidMethod("hideImeBridgeKeyboard", "()V");
#else
    sapp_show_keyboard(false);
#endif
}

bool MU_MobileIsTextInputActive()
{
    return g_textInputActive || sapp_keyboard_shown();
}

void MU_MobileSetTextInputRect(const SDL_Rect* rect)
{
    if (rect)
    {
        g_textInputRect = *rect;
#if defined(__ANDROID__)
        const int bw = (rect->w > 0) ? rect->w : 1;
        const int bh = (rect->h > 0) ? rect->h : 1;
        MU_AndroidSyncImeBridgeBounds(rect->x, rect->y, bw, bh);
#endif
    }
    else
    {
        g_textInputRect = {};
    }
}

void MU_MobileRequestExit()
{
    sapp_request_quit();
#if defined(__ANDROID__)
    const void* pActivity = sapp_android_get_native_activity();
    if (pActivity != nullptr)
    {
        ANativeActivity_finish(static_cast<ANativeActivity*>(const_cast<void*>(pActivity)));
    }
#endif
}

std::string MU_MobileGetExternalDataPath()
{
    return MU_GetFirstExistingPath({
        "/sdcard/Android/data/com.muonline.client/files",
        "/storage/emulated/0/Android/data/com.muonline.client/files"
    });
}

std::string MU_MobileGetInternalDataPath()
{
    return MU_GetFirstExistingPath({
        "/data/user/0/com.muonline.client/files",
        "/data/data/com.muonline.client/files"
    });
}

#endif // defined(__ANDROID__) || defined(MU_IOS)
