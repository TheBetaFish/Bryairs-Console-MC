#include "GLRenderer.h"

#include "SDL.h"
#include "SDL_error.h"
#include "SDL_events.h"
#include "SDL_stdinc.h"
#include "SDL_video.h"
#include "java/ByteBuffer.h"
#include "java/FloatBuffer.h"
#include "java/IntBuffer.h"
#include "platform/PlatformTypes.h"
#include "platform/renderer/renderer.h"

// undefine macros from header to avoid argument mismatch
#include "gl3_loader.h"
#undef glClearColor
#define glClearColor glad_glClearColor
#undef glClear
#define glClear glad_glClear
#undef glViewport
#define glViewport glad_glViewport
#undef glUseProgram
#define glUseProgram glad_glUseProgram
#undef glDrawArrays
#define glDrawArrays glad_glDrawArrays
#undef glBindBuffer
#define glBindBuffer glad_glBindBuffer
#undef glBufferData
#define glBufferData glad_glBufferData
#undef glBufferSubData
#define glBufferSubData glad_glBufferSubData
#undef glEnableVertexAttribArray
#define glEnableVertexAttribArray glad_glEnableVertexAttribArray
#undef glVertexAttribPointer
#define glVertexAttribPointer glad_glVertexAttribPointer
#undef glActiveTexture
#define glActiveTexture glad_glActiveTexture
#undef glBindTexture
#define glBindTexture glad_glBindTexture
#undef glTexParameteri
#define glTexParameteri glad_glTexParameteri
#undef glUniform1i
#define glUniform1i glad_glUniform1i
#undef glUniform1f
#define glUniform1f glad_glUniform1f
#undef glUniform2fv
#define glUniform2fv glad_glUniform2fv
#undef glUniform3fv
#define glUniform3fv glad_glUniform3fv
#undef glUniform4fv
#define glUniform4fv glad_glUniform4fv
#undef glUniformMatrix3fv
#define glUniformMatrix3fv glad_glUniformMatrix3fv
#undef glUniformMatrix4fv
#define glUniformMatrix4fv glad_glUniformMatrix4fv
#undef glReadPixels
#define glReadPixels glad_glReadPixels
#undef glGetError
#define glGetError glad_glGetError
#undef glGenTextures
#define glGenTextures glad_glGenTextures
#undef glDeleteTextures
#define glDeleteTextures glad_glDeleteTextures
#undef glTexImage2D
#define glTexImage2D glad_glTexImage2D
#undef glTexSubImage2D
#define glTexSubImage2D glad_glTexSubImage2D
#undef glGenerateMipmap
#define glGenerateMipmap glad_glGenerateMipmap
#undef glEnable
#define glEnable glad_glEnable
#undef glDisable
#define glDisable glad_glDisable
#undef glBlendFunc
#define glBlendFunc glad_glBlendFunc
#undef glDepthMask
#define glDepthMask glad_glDepthMask
#undef glColorMask
#define glColorMask glad_glColorMask
#undef glLineWidth
#define glLineWidth glad_glLineWidth
#undef glFrontFace
#define glFrontFace glad_glFrontFace
#undef glPolygonOffset
#define glPolygonOffset glad_glPolygonOffset
#undef glStencilFunc
#define glStencilFunc glad_glStencilFunc
#undef glStencilMask
#define glStencilMask glad_glStencilMask
#undef glDepthFunc
#define glDepthFunc glad_glDepthFunc
#undef glBlendColor
#define glBlendColor glad_glBlendColor
#undef glGetString
#define glGetString glad_glGetString
#undef glGetIntegerv
#define glGetIntegerv glad_glGetIntegerv
#undef glGetProgramiv
#define glGetProgramiv glad_glGetProgramiv
#undef glGetProgramInfoLog
#define glGetProgramInfoLog glad_glGetProgramInfoLog
#undef glCreateShader
#define glCreateShader glad_glCreateShader
#undef glShaderSource
#define glShaderSource glad_glShaderSource
#undef glCompileShader
#define glCompileShader glad_glCompileShader
#undef glGetShaderiv
#define glGetShaderiv glad_glGetShaderiv
#undef glGetShaderInfoLog
#define glGetShaderInfoLog glad_glGetShaderInfoLog
#undef glDeleteShader
#define glDeleteShader glad_glDeleteShader
#undef glCreateProgram
#define glCreateProgram glad_glCreateProgram
#undef glAttachShader
#define glAttachShader glad_glAttachShader
#undef glBindAttribLocation
#define glBindAttribLocation glad_glBindAttribLocation
#undef glLinkProgram
#define glLinkProgram glad_glLinkProgram
#undef glDeleteProgram
#define glDeleteProgram glad_glDeleteProgram
#undef glGetUniformLocation
#define glGetUniformLocation glad_glGetUniformLocation
#undef glFlush
#define glFlush glad_glFlush

#undef glGenTextures
#define glGenTextures glad_glGenTextures
#undef glDeleteTextures
#define glDeleteTextures glad_glDeleteTextures
#undef glTexImage2D
#define glTexImage2D glad_glTexImage2D
#undef glEnable
#define glEnable glad_glEnable
#undef glDisable
#define glDisable glad_glDisable
#undef glBlendFunc
#define glBlendFunc glad_glBlendFunc
#undef glDepthMask
#define glDepthMask glad_glDepthMask
#undef glColorMask
#define glColorMask glad_glColorMask
#undef glLineWidth
#define glLineWidth glad_glLineWidth
#undef glFrontFace
#define glFrontFace glad_glFrontFace
#undef glPolygonOffset
#define glPolygonOffset glad_glPolygonOffset
#undef glStencilFunc
#define glStencilFunc glad_glStencilFunc
#undef glStencilMask
#define glStencilMask glad_glStencilMask

#undef glCallLists
#undef glFog
#undef glLight
#undef glLightModel
#undef glTexGen
#undef glTexCoordPointer
#undef glNormalPointer
#undef glColorPointer
#undef glVertexPointe

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define GLM_FORCE_RADIANS

#include <cmath>
#include <cstdio>
#include <cstring>
#include <mutex>
#include <optional>
#include <thread>
#include <unordered_map>
#include <utility>
#include <vector>

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

namespace platform_internal {
IPlatformRenderer& PlatformRenderer_get() {
    static GLRenderer instance;
    return instance;
}
}  // namespace platform_internal

// MARK: Shaders

#define CPP_GLSL_INCLUDE

#ifdef GLES
static const char* VERT_SRC_ES2 =
#include "./shaders/vertex_es.vert"

    ;
static const char* FRAG_SRC_ES2 =
#include "./shaders/fragment_es.frag"

    ;
static const char* VERT_SRC_ES3 =
#include "./shaders/vertex_es3.vert"

    ;
static const char* FRAG_SRC_ES3 =
#include "./shaders/fragment_es3.frag"

    ;
#else
static const char* VERT_SRC =
#include "./shaders/vertex.vert"

    ;
static const char* FRAG_SRC =
#include "./shaders/fragment.frag"
    ;
#endif

static int s_glesMajor = 2;

static const char* shaderVertexSource() {
#ifdef GLES
    return (s_glesMajor >= 3) ? VERT_SRC_ES3 : VERT_SRC_ES2;
#else
    return VERT_SRC;
#endif
}

static const char* shaderFragmentSource() {
#ifdef GLES
    return (s_glesMajor >= 3) ? FRAG_SRC_ES3 : FRAG_SRC_ES2;
#else
    return FRAG_SRC;
#endif
}

#undef CPP_GLSL_INCLUDE

// MARK: OpenGL state

// Hello SDL and opengl 3.3
// bye bye opengl 3.3, hello gles2
static SDL_Window* s_window = nullptr;
static SDL_GLContext s_glContext = nullptr;
static bool s_shouldClose = false;
static int s_windowWidth = 1920;
static int s_windowHeight = 1080;
static int s_reqWidth = 1920;
static int s_reqHeight = 1080;
static bool s_fullscreen = false;

static thread_local SDL_GLContext s_glCtx = nullptr;

static std::once_flag s_glCtxKeyOnce;

static const int MAX_SHARED_CTXS = 6;
static SDL_Window* s_sharedWins[MAX_SHARED_CTXS] = {};
static SDL_GLContext s_sharedCtxs[MAX_SHARED_CTXS] = {};
static int s_sharedCtxCount = 0;
static int s_nextSharedCtx = 0;
static std::mutex s_sharedMtx;
static std::mutex s_glCallMtx;
static std::thread::id s_mainThreadId;
static bool s_mainThreadSet = false;
static thread_local unsigned int s_rs_dirty_mask = 0xFFFFFFFF;

struct GLShadowState {
    bool blend;
    bool cull;
    bool depth;
    bool polygon;
    bool stencil;
    GLint blendSrc;
    GLint blendDst;
    GLboolean depthMask;
    GLboolean colorMask[4];
    float lineWidth;
    GLenum frontFace;
    float polySlope;
    float polyBias;
    GLenum stencilFunc;
    GLint stencilRef;
    GLuint stencilMask;
    GLuint stencilWriteMask;
};

static GLShadowState s_gl_state;
static unsigned int s_gl_shadow_mask = 0;

enum GLShadowBits {
    SHADOW_BLEND = 1 << 0,
    SHADOW_CULL = 1 << 1,
    SHADOW_DEPTH = 1 << 2,
    SHADOW_BLEND_FUNC = 1 << 3,
    SHADOW_DEPTH_MASK = 1 << 4,
    SHADOW_COLOR_MASK = 1 << 5,
    SHADOW_LINE_WIDTH = 1 << 6,
    SHADOW_FRONT_FACE = 1 << 7,
    SHADOW_POLY_OFFSET = 1 << 8,
    SHADOW_POLY_OFFSET_PARAMS = 1 << 9,
    SHADOW_STENCIL = 1 << 10,
    SHADOW_STENCIL_PARAMS = 1 << 11,
};

static void onFramebufferResize(int w, int h) {
    if (w < 1) w = 1;
    if (h < 1) h = 1;
    s_windowWidth = w;
    s_windowHeight = h;
    glViewport(0, 0, w, h);
}

static GLuint compileShader(GLenum type, const char* src) {
    GLuint s = glCreateShader(type);
    glShaderSource(s, 1, &src, nullptr);
    glCompileShader(s);
    GLint ok = 0;
    glGetShaderiv(s, GL_COMPILE_STATUS, &ok);
    if (!ok) {
        char log[1024];
        glGetShaderInfoLog(s, sizeof(log), nullptr, log);
        fprintf(stderr, "[4J_Render] shader error:\n%s\n", log);
        glDeleteShader(s);
        return 0;
    }
    return s;
}

#if defined(__EMSCRIPTEN__)
extern "C" GLuint bryair_glCreateShader(GLenum type) {
    return glad_glCreateShader(type);
}
extern "C" void bryair_glDeleteShader(GLuint shader) {
    glad_glDeleteShader(shader);
}
extern "C" void bryair_glShaderSource(GLuint shader, GLsizei count,
                                      const GLchar** strings,
                                      const GLint* lengths) {
    glad_glShaderSource(shader, count, strings, lengths);
}
extern "C" void bryair_glCompileShader(GLuint shader) {
    glad_glCompileShader(shader);
}
extern "C" void bryair_glGetShaderiv(GLuint shader, GLenum pname, GLint* params) {
    glad_glGetShaderiv(shader, pname, params);
}
extern "C" void bryair_glGetShaderInfoLog(GLuint shader, GLsizei maxLength,
                                          GLsizei* length, GLchar* infoLog) {
    glad_glGetShaderInfoLog(shader, maxLength, length, infoLog);
}
extern "C" GLuint bryair_glCreateProgram(void) {
    return glad_glCreateProgram();
}
extern "C" void bryair_glDeleteProgram(GLuint program) {
    glad_glDeleteProgram(program);
}
extern "C" void bryair_glAttachShader(GLuint program, GLuint shader) {
    glad_glAttachShader(program, shader);
}
extern "C" void bryair_glLinkProgram(GLuint program) {
    glad_glLinkProgram(program);
}
extern "C" GLint bryair_glGetUniformLocation(GLuint program, const GLchar* name) {
    return glad_glGetUniformLocation(program, name);
}
extern "C" void bryair_glUseProgram(GLuint program) {
    glad_glUseProgram(program);
}
extern "C" void bryair_glGetProgramiv(GLuint program, GLenum pname, GLint* params) {
    glad_glGetProgramiv(program, pname, params);
}
extern "C" void bryair_glGetProgramInfoLog(GLuint program, GLsizei maxLength,
                                           GLsizei* length, GLchar* infoLog) {
    glad_glGetProgramInfoLog(program, maxLength, length, infoLog);
}
extern "C" void bryair_glUniform1i(GLint location, GLint v0) {
    glad_glUniform1i(location, v0);
}
extern "C" void bryair_glUniform4f(GLint location, GLfloat v0, GLfloat v1,
                                   GLfloat v2, GLfloat v3) {
    glad_glUniform4f(location, v0, v1, v2, v3);
}
extern "C" void bryair_glUniform4fv(GLint location, GLsizei count,
                                    const GLfloat* value) {
    glad_glUniform4fv(location, count, value);
}
extern "C" void bryair_glUniform1f(GLint location, GLfloat v0) {
    glad_glUniform1f(location, v0);
}
extern "C" void bryair_glBindAttribLocation(GLuint program, GLuint index,
                                            const GLchar* name) {
    glad_glBindAttribLocation(program, index, name);
}
#endif

static GLuint linkProgram(GLuint v, GLuint f) {
    GLuint p = glCreateProgram();
    glAttachShader(p, v);
    glAttachShader(p, f);

    glBindAttribLocation(p, 0, "aPos");
    glBindAttribLocation(p, 1, "aUV0");
    glBindAttribLocation(p, 2, "aColor");
    glBindAttribLocation(p, 3, "aNormal");
    glBindAttribLocation(p, 4, "aLMraw");

    glLinkProgram(p);
    GLint ok = 0;
    glGetProgramiv(p, GL_LINK_STATUS, &ok);
    if (!ok) {
        char log[1024];
        glGetProgramInfoLog(p, sizeof(log), nullptr, log);
        fprintf(stderr, "[4J_Render] link error:\n%s\n", log);
        glDeleteProgram(p);
        return 0;
    }
    return p;
}

// Shader struct
struct ShaderUniforms {
    GLuint prog = 0;

    GLint uMVP = -1, uMV = -1, uBaseColor = -1;
    GLint uTexMat0 = -1;
    GLint uNormalMatrix = -1, uNormalSign = -1;
    GLint uLighting = -1, uLight0Dir = -1, uLight1Dir = -1;
    GLint uLightDiffuse = -1, uLightAmbient = -1;
    GLint uFogMode = -1, uFogStart = -1, uFogEnd = -1;
    GLint uFogDensity = -1, uFogColor = -1, uFogEnable = -1;
    GLint uLMTransform = -1, uUseLightmap = -1, uAlphaRef = -1;
    GLint uTex0 = -1, uTex1 = -1, uGlobalLM = -1;
    GLint uUseTexture = -1;
    GLint uInvGamma = -1;
    GLint uChunkOffset = -1;
    GLint uFlatShading = -1;
    GLint uTexGenMask = -1, uTexGenEye = -1;
    GLint uTexGenPS = -1, uTexGenPT = -1, uTexGenPR = -1, uTexGenPQ = -1;

    void build(const char* vs, const char* fs) {
        GLuint v = compileShader(GL_VERTEX_SHADER, vs);
        GLuint f = compileShader(GL_FRAGMENT_SHADER, fs);
        prog = linkProgram(v, f);
        glDeleteShader(v);
        glDeleteShader(f);
        if (!prog) return;

#define L(x) x = glGetUniformLocation(prog, #x)
        L(uMVP);
        L(uMV);
        L(uNormalMatrix);
        L(uNormalSign);
        L(uTexMat0);
        L(uBaseColor);
        L(uLighting);
        L(uLight0Dir);
        L(uLight1Dir);
        L(uLightDiffuse);
        L(uLightAmbient);
        L(uFogMode);
        L(uFogStart);
        L(uFogEnd);
        L(uFogDensity);
        L(uFogColor);
        L(uFogEnable);
        L(uLMTransform);
        L(uUseLightmap);
        L(uAlphaRef);
        L(uTex0);
        L(uTex1);
        L(uGlobalLM);
        L(uUseTexture);
        L(uInvGamma);
        L(uChunkOffset);
        L(uFlatShading);
        L(uTexGenMask);
        L(uTexGenEye);
        L(uTexGenPS);
        L(uTexGenPT);
        L(uTexGenPR);
        L(uTexGenPQ);
#undef L

        glUseProgram(prog);
        glUniform1i(uTex0, 0);
        glUniform1i(uTex1, 1);
    }
} s_shader;

static std::vector<GLuint> s_vbosToDelete;
static std::mutex s_vbosToDeleteMtx;

static void queueVboDeletion(GLuint vbo) {
    if (!vbo) return;
    std::lock_guard<std::mutex> lk(s_vbosToDeleteMtx);
    s_vbosToDelete.push_back(vbo);
}

static void processPendingDeletions() {
    std::vector<GLuint> toDelete;
    {
        std::lock_guard<std::mutex> lk(s_vbosToDeleteMtx);
        if (s_vbosToDelete.empty()) return;
        toDelete = std::move(s_vbosToDelete);
        s_vbosToDelete.clear();
    }
    glDeleteBuffers((GLsizei)toDelete.size(), toDelete.data());
}

// Matrix stacks
static const int STACK_DEPTH = 64;
struct MatrixStack {
    glm::mat4 stack[STACK_DEPTH];
    int top = 0;
    MatrixStack() { stack[0] = glm::mat4(1.f); }
    glm::mat4& cur() { return stack[top]; }
    void push() {
        if (top < STACK_DEPTH - 1) {
            stack[top + 1] = stack[top];
            ++top;
        }
    }
    void pop() {
        if (top > 0) --top;
    }
    void load(const glm::mat4& m) { cur() = m; }
    void mul(const glm::mat4& m) { cur() = cur() * m; }
};
static thread_local MatrixStack s_proj, s_mv, s_tex[2];
static thread_local int s_matMode = 0;  // 0=MV 1=proj 2=tex0 3=tex1

// cache normal matrix
static thread_local bool s_normalMatDirty = true;
static thread_local glm::mat3 s_cachedNormalMat;
static thread_local float s_cachedNormalSign = 1.0f;
static thread_local bool s_matDirty = true;
static inline void markNormalDirty() { s_normalMatDirty = true; }
static inline void markMatrixDirty() { s_matDirty = true; }

static MatrixStack& activeStack() {
    switch (s_matMode) {
        case 1:
            return s_proj;
        case 2:
            return s_tex[0];
        case 3:
            return s_tex[1];
    }
    return s_mv;
}

static void flushMatrices() {
    if (s_matDirty) {
        glm::mat4 mvp = s_proj.cur() * s_mv.cur();
        glUniformMatrix4fv(s_shader.uMVP, 1, GL_FALSE, glm::value_ptr(mvp));
        glUniformMatrix4fv(s_shader.uMV, 1, GL_FALSE,
                           glm::value_ptr(s_mv.cur()));

        // Send the texture matrix to the depths of hell...
        glUniformMatrix4fv(s_shader.uTexMat0, 1, GL_FALSE,
                           glm::value_ptr(s_tex[0].cur()));
        s_matDirty = false;
    }

    if (s_shader.uNormalMatrix >= 0 && s_normalMatDirty) {
        glm::mat3 m3 = glm::mat3(s_mv.cur());
        s_cachedNormalMat = glm::transpose(glm::inverse(m3));
        s_cachedNormalSign = glm::determinant(m3) < 0.0f ? -1.0f : 1.0f;
        s_normalMatDirty = false;
        glUniformMatrix3fv(s_shader.uNormalMatrix, 1, GL_FALSE,
                           glm::value_ptr(s_cachedNormalMat));
        glUniform1f(s_shader.uNormalSign, s_cachedNormalSign);
    }
}

// Render state
struct RenderState {
    glm::vec4 baseColor = {1, 1, 1, 1};
    glm::vec4 fogColor = {0, 0, 0, 1};
    float fogStart = 0, fogEnd = 1000, fogDensity = 0;
    int fogMode = 0;
    bool fogEnable = false;
    float alphaRef = 0.1f;
    float gamma = 1.0f;
    bool useTexture = true, useLightmap = false, lighting = false;
    glm::vec3 l0 = {0.173913f, 0.869565f, -0.608696f};
    glm::vec3 l1 = {-0.173913f, 0.869565f, 0.608696f};
    glm::vec3 ldiff = {0.6f, 0.6f, 0.6f};
    glm::vec3 lamb = {0.4f, 0.4f, 0.4f};
    glm::vec4 lmt = {1, 1, 0, 0};
    glm::vec2 globalLM = {240.f, 240.f};  // fullbright default
    int activeTexture = 0;
    bool flatShading = false;
    int texGenMask = 0, texGenEye = 0;
    glm::vec4 texGenS = {0, 0, 0, 0}, texGenT = {0, 0, 0, 0};
    glm::vec4 texGenR = {0, 0, 0, 0}, texGenQ = {0, 0, 0, 0};
};

enum RenderDirtyBits {
    DIRTY_BASECOLOR = 1 << 0,
    DIRTY_LIGHTING = 1 << 1,
    DIRTY_FOG = 1 << 2,
    DIRTY_ALPHA = 1 << 3,
    DIRTY_GAMMA = 1 << 4,
    DIRTY_TEXTURE = 1 << 5,
    DIRTY_LMT = 1 << 6,
    DIRTY_GLOBAL_LM = 1 << 7,
    DIRTY_SHADE = 1 << 8,
    DIRTY_TEXGEN = 1 << 9,
};

static inline void markDirty(unsigned int bit) { s_rs_dirty_mask |= bit; }

static thread_local RenderState s_rs;

// track currently bound program to avoid iggy shitting up
static GLuint s_boundProgram = 0;

static void glShadowSetBlend(bool e) {
    if (!(s_gl_shadow_mask & SHADOW_BLEND) || s_gl_state.blend != e) {
        if (e)
            glEnable(GL_BLEND);
        else
            glDisable(GL_BLEND);
        s_gl_state.blend = e;
        s_gl_shadow_mask |= SHADOW_BLEND;
    }
}

static void glShadowSetCull(bool e) {
    if (!(s_gl_shadow_mask & SHADOW_CULL) || s_gl_state.cull != e) {
        if (e)
            glEnable(GL_CULL_FACE);
        else
            glDisable(GL_CULL_FACE);
        s_gl_state.cull = e;
        s_gl_shadow_mask |= SHADOW_CULL;
    }
}

static void glShadowSetDepthTest(bool e) {
    if (!(s_gl_shadow_mask & SHADOW_DEPTH) || s_gl_state.depth != e) {
        if (e)
            glEnable(GL_DEPTH_TEST);
        else
            glDisable(GL_DEPTH_TEST);
        s_gl_state.depth = e;
        s_gl_shadow_mask |= SHADOW_DEPTH;
    }
}

static void glShadowSetBlendFunc(GLint s, GLint d) {
    if (!(s_gl_shadow_mask & SHADOW_BLEND_FUNC) || s_gl_state.blendSrc != s ||
        s_gl_state.blendDst != d) {
        glBlendFunc(s, d);
        s_gl_state.blendSrc = s;
        s_gl_state.blendDst = d;
        s_gl_shadow_mask |= SHADOW_BLEND_FUNC;
    }
}

static void glShadowSetDepthMask(GLboolean e) {
    if (!(s_gl_shadow_mask & SHADOW_DEPTH_MASK) || s_gl_state.depthMask != e) {
        glDepthMask(e);
        s_gl_state.depthMask = e;
        s_gl_shadow_mask |= SHADOW_DEPTH_MASK;
    }
}

static void glShadowSetColorMask(GLboolean r, GLboolean g, GLboolean b,
                                 GLboolean a) {
    if (!(s_gl_shadow_mask & SHADOW_COLOR_MASK) ||
        s_gl_state.colorMask[0] != r || s_gl_state.colorMask[1] != g ||
        s_gl_state.colorMask[2] != b || s_gl_state.colorMask[3] != a) {
        glColorMask(r, g, b, a);
        s_gl_state.colorMask[0] = r;
        s_gl_state.colorMask[1] = g;
        s_gl_state.colorMask[2] = b;
        s_gl_state.colorMask[3] = a;
        s_gl_shadow_mask |= SHADOW_COLOR_MASK;
    }
}

static void glShadowSetLineWidth(float w) {
    if (!(s_gl_shadow_mask & SHADOW_LINE_WIDTH) || s_gl_state.lineWidth != w) {
        glLineWidth(w);
        s_gl_state.lineWidth = w;
        s_gl_shadow_mask |= SHADOW_LINE_WIDTH;
    }
}

static void glShadowSetFrontFace(GLenum mode) {
    if (!(s_gl_shadow_mask & SHADOW_FRONT_FACE) ||
        s_gl_state.frontFace != mode) {
        glFrontFace(mode);
        s_gl_state.frontFace = mode;
        s_gl_shadow_mask |= SHADOW_FRONT_FACE;
    }
}

static void glShadowSetPolygonOffset(float slope, float bias) {
    bool enable = (slope != 0.0f || bias != 0.0f);
    if (!(s_gl_shadow_mask & SHADOW_POLY_OFFSET) ||
        s_gl_state.polygon != enable) {
        if (enable)
            glEnable(GL_POLYGON_OFFSET_FILL);
        else
            glDisable(GL_POLYGON_OFFSET_FILL);
        s_gl_state.polygon = enable;
        s_gl_shadow_mask |= SHADOW_POLY_OFFSET;
    }
    if (enable) {
        if (!(s_gl_shadow_mask & SHADOW_POLY_OFFSET_PARAMS) ||
            s_gl_state.polySlope != slope || s_gl_state.polyBias != bias) {
            glPolygonOffset(slope, bias);
            s_gl_state.polySlope = slope;
            s_gl_state.polyBias = bias;
            s_gl_shadow_mask |= SHADOW_POLY_OFFSET_PARAMS;
        }
    }
}

static void glShadowSetStencil(GLenum fn, uint8_t ref, uint8_t fmask,
                               uint8_t wmask) {
    if (!(s_gl_shadow_mask & SHADOW_STENCIL) || !s_gl_state.stencil) {
        glEnable(GL_STENCIL_TEST);
        s_gl_state.stencil = true;
        s_gl_shadow_mask |= SHADOW_STENCIL;
    }
    if (!(s_gl_shadow_mask & SHADOW_STENCIL_PARAMS) ||
        s_gl_state.stencilFunc != fn || s_gl_state.stencilRef != (GLint)ref ||
        s_gl_state.stencilMask != fmask ||
        s_gl_state.stencilWriteMask != wmask) {
        glStencilFunc(fn, ref, fmask);
        glStencilMask(wmask);
        s_gl_state.stencilFunc = fn;
        s_gl_state.stencilRef = (GLint)ref;
        s_gl_state.stencilMask = fmask;
        s_gl_state.stencilWriteMask = wmask;
        s_gl_shadow_mask |= SHADOW_STENCIL_PARAMS;
    }
}
static thread_local bool s_chunkOffsetValid = false;
static thread_local glm::vec3 s_chunkOffset;

static void pushRenderState() {
    if (!s_shader.prog) return;

    // only call glUseProgram when something actually changed the binding
    if (s_boundProgram != s_shader.prog) {
        glUseProgram(s_shader.prog);
        s_boundProgram = s_shader.prog;
        s_matDirty = true;
        s_normalMatDirty = true;
        s_rs_dirty_mask = 0xFFFFFFFF;
    }

    if (s_rs_dirty_mask) {
        if (s_rs_dirty_mask & DIRTY_BASECOLOR)
            glUniform4fv(s_shader.uBaseColor, 1,
                         glm::value_ptr(s_rs.baseColor));
        if (s_rs_dirty_mask & DIRTY_LIGHTING) {
            glUniform1i(s_shader.uLighting, s_rs.lighting ? 1 : 0);
            glUniform3fv(s_shader.uLight0Dir, 1, glm::value_ptr(s_rs.l0));
            glUniform3fv(s_shader.uLight1Dir, 1, glm::value_ptr(s_rs.l1));
            glUniform3fv(s_shader.uLightDiffuse, 1, glm::value_ptr(s_rs.ldiff));
            glUniform3fv(s_shader.uLightAmbient, 1, glm::value_ptr(s_rs.lamb));
        }
        if (s_rs_dirty_mask & DIRTY_FOG) {
            glUniform1i(s_shader.uFogMode, s_rs.fogMode);
            glUniform1f(s_shader.uFogStart, s_rs.fogStart);
            glUniform1f(s_shader.uFogEnd, s_rs.fogEnd);
            glUniform1f(s_shader.uFogDensity, s_rs.fogDensity);
            glUniform4fv(s_shader.uFogColor, 1, glm::value_ptr(s_rs.fogColor));
            glUniform1i(s_shader.uFogEnable, s_rs.fogEnable ? 1 : 0);
        }
        if (s_rs_dirty_mask & DIRTY_TEXTURE) {
            glUniform1i(s_shader.uUseTexture, s_rs.useTexture ? 1 : 0);
            glUniform1i(s_shader.uUseLightmap, s_rs.useLightmap ? 1 : 0);
        }
        if (s_rs_dirty_mask & DIRTY_ALPHA)
            glUniform1f(s_shader.uAlphaRef, s_rs.alphaRef);
        if (s_rs_dirty_mask & DIRTY_GAMMA)
            glUniform1f(s_shader.uInvGamma, 1.0f / s_rs.gamma);
        if (s_rs_dirty_mask & DIRTY_LMT)
            glUniform4fv(s_shader.uLMTransform, 1, glm::value_ptr(s_rs.lmt));
        if (s_rs_dirty_mask & DIRTY_GLOBAL_LM)
            glUniform2fv(s_shader.uGlobalLM, 1, glm::value_ptr(s_rs.globalLM));
        if (s_rs_dirty_mask & DIRTY_SHADE)
            glUniform1i(s_shader.uFlatShading, s_rs.flatShading ? 1 : 0);
        if (s_rs_dirty_mask & DIRTY_TEXGEN) {
            glUniform1i(s_shader.uTexGenMask, s_rs.texGenMask);
            glUniform1i(s_shader.uTexGenEye, s_rs.texGenEye);
            glUniform4fv(s_shader.uTexGenPS, 1, glm::value_ptr(s_rs.texGenS));
            glUniform4fv(s_shader.uTexGenPT, 1, glm::value_ptr(s_rs.texGenT));
            glUniform4fv(s_shader.uTexGenPR, 1, glm::value_ptr(s_rs.texGenR));
            glUniform4fv(s_shader.uTexGenPQ, 1, glm::value_ptr(s_rs.texGenQ));
        }
        s_rs_dirty_mask = 0;
    }
    flushMatrices();
}

static GLuint s_sVBO_std = 0;
static GLsizeiptr s_streamVBOSize = 0;

static void bindStdAttribs() {
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    glEnableVertexAttribArray(3);
    glEnableVertexAttribArray(4);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 32, (void*)0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 32, (void*)12);
    glVertexAttribPointer(2, 4, GL_UNSIGNED_BYTE, GL_TRUE, 32, (void*)20);
    glVertexAttribPointer(3, 3, GL_BYTE, GL_TRUE, 32, (void*)24);
    // Changed IPointer to Pointer
    glVertexAttribPointer(4, 2, GL_SHORT, GL_FALSE, 32, (void*)28);
}

static void initStreamingVBOs() { glGenBuffers(1, &s_sVBO_std); }

// Chunk buffer pool (shared, protected by s_glCallMtx)
struct ChunkDrawCall {
    GLenum prim;
    GLint first;
    GLsizei count;
};

struct ChunkBuffer {
    GLuint vbo = 0;
    std::vector<ChunkDrawCall> draws;
    std::vector<uint8_t> rawVerts;
    bool valid = false;
    bool vboReady = false;

    void destroy() {
        if (vbo) {
            // Check if we are on the main thread
            if (s_mainThreadSet &&
                std::this_thread::get_id() == s_mainThreadId) {
                glDeleteBuffers(1, &vbo);
            } else {
                // Background thread: safely defer the deletion to the main
                // thread
                queueVboDeletion(vbo);
            }
            vbo = 0;
        }
        draws.clear();
        rawVerts.clear();
        valid = false;
        vboReady = false;
    }
};

static std::unordered_map<int, ChunkBuffer> s_chunkPool;
static int s_nextListBase = 1;

// Per-thread recording state
static thread_local int s_recListId = -1;
static thread_local std::vector<uint8_t> s_recVerts;
static thread_local std::vector<ChunkDrawCall> s_recDraws;

// Primitive helpers
static bool isQuadPrim(int pt) {
    return (pt == 0x0007 /*GL_QUADS*/ ||
            pt == (int)GLRenderer::PRIMITIVE_TYPE_QUAD_LIST);
}

static GLenum mapPrim(int pt) {
    if (isQuadPrim(pt)) return GL_TRIANGLES;
    switch (pt) {
        case 0:
            return GL_TRIANGLES;
        case 1:
            return GL_LINES;
        case 2:
            return GL_TRIANGLE_FAN;
        case 3:
            return GL_LINE_STRIP;
        case 4:
            return GL_TRIANGLES;
        case 5:
            return GL_TRIANGLE_STRIP;
        case 6:
            return GL_TRIANGLE_FAN;
        default:
            return GL_TRIANGLES;
    }
}

// MARK: Renderer impl

// Initialises the renderer
void GLRenderer::Initialise() {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        fprintf(stderr, "[4J_Render] SDL_Init: %s\n", SDL_GetError());
        return;
    }
    SDL_DisplayMode dm;
    if (s_reqWidth > 0 && s_reqHeight > 0) {
        s_windowWidth = s_reqWidth;
        s_windowHeight = s_reqHeight;
    } else if (SDL_GetCurrentDisplayMode(0, &dm) == 0) {
        s_windowWidth = (int)(dm.w * 0.4f);
        s_windowHeight = (int)(dm.h * 0.4f);
    }
#ifdef GLES
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
#else
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK,
                        SDL_GL_CONTEXT_PROFILE_CORE);
#endif
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    Uint32 wf = SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE;
    if (s_fullscreen) wf |= SDL_WINDOW_FULLSCREEN_DESKTOP;
    s_window = SDL_CreateWindow("Minecraft Console Edition",
                                SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                s_windowWidth, s_windowHeight, wf);
    if (!s_window) {
        fprintf(stderr, "[4J_Render] Window: %s\n", SDL_GetError());
        return;
    }
    s_glContext = SDL_GL_CreateContext(s_window);
#ifdef GLES
    if (!s_glContext) {
        fprintf(stderr,
                "[4J_Render] GLES3 context unavailable, falling back to "
                "GLES2: %s\n",
                SDL_GetError());
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK,
                            SDL_GL_CONTEXT_PROFILE_ES);
        s_glContext = SDL_GL_CreateContext(s_window);
    }
#endif
    if (!s_glContext) {
        fprintf(stderr, "[4J_Render] Context: %s\n", SDL_GetError());
        return;
    }
    if (!gl3_load()) {
        return;
    }
#ifdef GLES
    s_glesMajor = gl3_gles_major_version();
#endif

    while (glGetError() != GL_NO_ERROR);
    fprintf(stderr, "=== OPENGL CONTEXT INFO ===\n");
    fprintf(stderr, "Vendor:   %s\n", glGetString(GL_VENDOR));
    fprintf(stderr, "Renderer: %s\n", glGetString(GL_RENDERER));
    fprintf(stderr, "Version:  %s\n", glGetString(GL_VERSION));
    fprintf(stderr, "GLSL:     %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));
    fprintf(stderr, "===========================\n");

    int fw, fh;
    SDL_GetWindowSize(s_window, &fw, &fh);
    onFramebufferResize(fw, fh);
    glShadowSetDepthTest(true);
    glDepthFunc(GL_LEQUAL);
#ifdef GLES
    glClearDepthf(1.0f);
#else
    glClearDepth(1.0);
#endif
    glShadowSetBlend(true);
    glShadowSetBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glShadowSetCull(true);
    glCullFace(GL_BACK);
    glClearColor(0, 0, 0, 1);
    glViewport(0, 0, s_windowWidth, s_windowHeight);
    s_shader.build(shaderVertexSource(), shaderFragmentSource());
    initStreamingVBOs();

    s_mainThreadId = std::this_thread::get_id();
    s_mainThreadSet = true;
    s_glCtx = s_glContext;

    SDL_GL_MakeCurrent(s_window, s_glContext);
    s_sharedCtxCount = 0;
    /*
     *   for (int i = 0; i < MAX_SHARED_CTXS; i++) {
     *       SDL_GL_SetAttribute(SDL_GL_SHARE_WITH_CURRENT_CONTEXT, 1);
     *       SDL_Window* w = SDL_CreateWindow("", SDL_WINDOWPOS_UNDEFINED,
     *                                        SDL_WINDOWPOS_UNDEFINED, 1, 1,
     *                                        SDL_WINDOW_HIDDEN |
SDL_WINDOW_OPENGL);
     *       if (!w) break;
     *       SDL_GLContext ctx = SDL_GL_CreateContext(w);
     *       if (!ctx) {
     *           SDL_DestroyWindow(w);
     *           break;
}
s_sharedWins[s_sharedCtxCount] = w;
s_sharedCtxs[s_sharedCtxCount] = ctx;
s_sharedCtxCount++;
}
*/
    SDL_GL_MakeCurrent(s_window, s_glContext);
    pushRenderState();

#ifdef ENABLE_VSYNC
    SDL_GL_SetSwapInterval(1);
#else
    SDL_GL_SetSwapInterval(0);
#endif
}

void GLRenderer::InitialiseContext() {
    if (!s_window) return;

    if (s_mainThreadSet && std::this_thread::get_id() == s_mainThreadId) {
        SDL_GL_MakeCurrent(s_window, s_glContext);
        s_glCtx = s_glContext;
        return;
    }

    if (s_glCtx) {
        for (int i = 0; i < s_sharedCtxCount; i++) {
            if (s_sharedCtxs[i] == s_glCtx) {
                SDL_GL_MakeCurrent(s_sharedWins[i], s_glCtx);
                return;
            }
        }
        return;
    }

    SDL_GLContext shared = nullptr;
    {
        std::lock_guard<std::mutex> lk(s_sharedMtx);
        if (s_nextSharedCtx < s_sharedCtxCount)
            shared = s_sharedCtxs[s_nextSharedCtx++];
    }
    if (!shared) return;

    for (int i = 0; i < s_sharedCtxCount; i++) {
        if (s_sharedCtxs[i] == shared)
            SDL_GL_MakeCurrent(s_sharedWins[i], shared);
    }
    s_glCtx = shared;
}

void GLRenderer::StartFrame() {
    processPendingDeletions();

    Set_matrixDirty();
    int w, h;
    SDL_GetWindowSize(s_window, &w, &h);
    s_windowWidth = w > 0 ? w : 1;
    s_windowHeight = h > 0 ? h : 1;
    glViewport(0, 0, s_windowWidth, s_windowHeight);
}

void GLRenderer::Present() {
    if (!s_window) return;
    SDL_Event ev;
    while (SDL_PollEvent(&ev)) {
        if (ev.type == SDL_QUIT)
            s_shouldClose = true;
        else if (ev.type == SDL_WINDOWEVENT) {
            if (ev.window.event == SDL_WINDOWEVENT_CLOSE)
                s_shouldClose = true;
            else if (ev.window.event == SDL_WINDOWEVENT_RESIZED)
                onFramebufferResize(ev.window.data1, ev.window.data2);
        }
    }
    glFlush();
    SDL_GL_SwapWindow(s_window);
}

void GLRenderer::SetWindowSize(int w, int h) {
    s_reqWidth = w;
    s_reqHeight = h;
}

void GLRenderer::SetFullscreen(bool fs) { s_fullscreen = fs; }

bool GLRenderer::ShouldClose() { return !s_window || s_shouldClose; }

void GLRenderer::GetFramebufferSize(int& w, int& h) {
    w = s_windowWidth;
    h = s_windowHeight;
}

void GLRenderer::Close() { s_window = nullptr; }

void GLRenderer::Shutdown() {
    {
        std::lock_guard<std::mutex> lk(s_glCallMtx);
        for (auto& kv : s_chunkPool) kv.second.destroy();
        s_chunkPool.clear();
    }
    // glDeleteVertexArrays(1, &s_sVAO_std);
    glDeleteBuffers(1, &s_sVBO_std);
    if (s_shader.prog) glDeleteProgram(s_shader.prog);
    if (s_glContext) {
        SDL_GL_DeleteContext(s_glContext);
        s_glContext = nullptr;
    }
    if (s_window) {
        SDL_DestroyWindow(s_window);
        s_window = nullptr;
    }
    for (int i = 0; i < s_sharedCtxCount; i++) {
        if (s_sharedCtxs[i]) SDL_GL_DeleteContext(s_sharedCtxs[i]);
        if (s_sharedWins[i]) SDL_DestroyWindow(s_sharedWins[i]);
    }
    SDL_Quit();
}

void GLRenderer::DrawVertices(ePrimitiveType ptype, int count, void* dataIn,
                              eVertexType vType, ePixelShaderType) {
    if (count <= 0 || !dataIn) return;

    bool wasQuad = isQuadPrim((int)ptype);
    GLenum glMode = mapPrim((int)ptype);
    static thread_local std::vector<uint8_t> stdData;
    static thread_local std::vector<uint8_t> triData;
    stdData.clear();
    triData.clear();

    if (vType == VERTEX_TYPE_COMPRESSED) {
        stdData.resize((size_t)count * 32);
        const int16_t* src = (const int16_t*)dataIn;
        uint8_t* dst = stdData.data();
        for (int i = 0; i < count; i++) {
            float* dstF = (float*)dst;

            // Position: int16 / 1024
            dstF[0] = src[0] / 1024.0f;
            dstF[1] = src[1] / 1024.0f;
            dstF[2] = src[2] / 1024.0f;

            // int16 / 8192
            dstF[3] = src[4] / 8192.0f;
            dstF[4] = src[5] / 8192.0f;

            // RGB565 −32768
            {
                uint16_t packed = (uint16_t)((int)src[3] + 32768);
                dst[20] = 255;
                dst[21] = (uint8_t)((packed & 0x1F) * 255 / 31);          // B
                dst[22] = (uint8_t)(((packed >> 5) & 0x3F) * 255 / 63);   // G
                dst[23] = (uint8_t)(((packed >> 11) & 0x1F) * 255 / 31);  // R
            }
            dst[24] = 0;
            dst[25] = 127;  // +Y (up)
            dst[26] = 0;
            dst[27] = 0;

            // Lightmap
            {
                int16_t* dstS = (int16_t*)(dst + 28);
                dstS[0] = src[6];
                dstS[1] = src[7];
            }

            src += 8;
            dst += 32;
        }
        dataIn = stdData.data();
    }

    static const size_t stride = 32;
    if (wasQuad) {
        int numQuads = count / 4;
        int triVerts = numQuads * 6;
        triData.resize((size_t)triVerts * stride);
        const uint8_t* src = (const uint8_t*)dataIn;
        uint8_t* dst = triData.data();
        for (int q = 0; q < numQuads; q++) {
            const uint8_t* v0 = src + (q * 4 + 0) * stride;
            const uint8_t* v1 = src + (q * 4 + 1) * stride;
            const uint8_t* v2 = src + (q * 4 + 2) * stride;
            const uint8_t* v3 = src + (q * 4 + 3) * stride;
            // Triangle 1: 0,1,2
            memcpy(dst + 0 * stride, v0, stride);
            memcpy(dst + 1 * stride, v1, stride);
            memcpy(dst + 2 * stride, v2, stride);
            // Triangle 2: 0,2,3
            memcpy(dst + 3 * stride, v0, stride);
            memcpy(dst + 4 * stride, v2, stride);
            memcpy(dst + 5 * stride, v3, stride);
            dst += 6 * stride;
        }
        dataIn = triData.data();
        count = triVerts;
        glMode = GL_TRIANGLES;
    }

    size_t bytes = (size_t)count * stride;

    if (s_recListId >= 0) {
        int first = (int)(s_recVerts.size() / stride);
        s_recVerts.insert(s_recVerts.end(), (const uint8_t*)dataIn,
                          (const uint8_t*)dataIn + bytes);
        s_recDraws.push_back({glMode, first, (GLsizei)count});
        return;
    }

    std::lock_guard<std::mutex> lk(s_glCallMtx);
    pushRenderState();

    glBindBuffer(GL_ARRAY_BUFFER, s_sVBO_std);

    glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr)bytes, nullptr, GL_STREAM_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, (GLsizeiptr)bytes, dataIn);
    s_streamVBOSize = (GLsizeiptr)bytes;

    // Call standard attributes right before drawing
    bindStdAttribs();

    glDrawArrays(glMode, 0, count);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void GLRenderer::ReadPixels(int x, int y, int w, int h, void* buf) {
    if (!buf) return;
    std::lock_guard<std::mutex> lk(s_glCallMtx);
    glReadPixels(x, y, w, h, GL_RGBA, GL_UNSIGNED_BYTE, buf);
}

int GLRenderer::CBuffCreate(int count) {
    std::lock_guard<std::mutex> lk(s_glCallMtx);
    int b = s_nextListBase;
    s_nextListBase += count;
    return b;
}

void GLRenderer::CBuffDelete(int first, int count) {
    std::lock_guard<std::mutex> lk(s_glCallMtx);
    for (int i = first; i < first + count; i++) {
        auto it = s_chunkPool.find(i);
        if (it != s_chunkPool.end()) {
            it->second.destroy();
            s_chunkPool.erase(it);
        }
    }
}

void GLRenderer::CBuffDeleteAll() {
    std::lock_guard<std::mutex> lk(s_glCallMtx);
    for (auto& kv : s_chunkPool) {
        kv.second.destroy();
    }
    s_chunkPool.clear();
    s_nextListBase = 1;
}

void GLRenderer::CBuffStart(int index, bool) {
    s_recListId = index;
    s_recVerts.clear();
    s_recDraws.clear();
}

void GLRenderer::CBuffEnd() {
    if (s_recListId < 0) return;
    std::lock_guard<std::mutex> lk(s_glCallMtx);
    ChunkBuffer& cb = s_chunkPool[s_recListId];
    cb.destroy();
    if (s_recVerts.empty()) {
        s_chunkPool.erase(s_recListId);
        s_recListId = -1;
        return;
    }
    cb.rawVerts = std::move(s_recVerts);
    cb.draws = std::move(s_recDraws);
    cb.valid = true;
    cb.vboReady = false;
    s_recListId = -1;
}

void GLRenderer::CBuffClear(int index) {
    std::lock_guard<std::mutex> lk(s_glCallMtx);
    auto it = s_chunkPool.find(index);
    if (it != s_chunkPool.end()) {
        it->second.destroy();
        s_chunkPool.erase(it);
    }
}

bool GLRenderer::CBuffCall(int index, bool) {
    std::lock_guard<std::mutex> lk(s_glCallMtx);
    auto it = s_chunkPool.find(index);
    if (it == s_chunkPool.end() || !it->second.valid) {
        return false;
    }
    ChunkBuffer& cb = it->second;
    if (!cb.vboReady) {
        if (cb.rawVerts.empty()) {
            return false;
        }

        glGenBuffers(1, &cb.vbo);
        glBindBuffer(GL_ARRAY_BUFFER, cb.vbo);
        glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr)cb.rawVerts.size(),
                     cb.rawVerts.data(), GL_STATIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        cb.rawVerts.clear();
        cb.rawVerts.shrink_to_fit();
        cb.vboReady = true;
    }

    pushRenderState();

    // Draw phase
    glBindBuffer(GL_ARRAY_BUFFER, cb.vbo);
    bindStdAttribs();  // Bind attributes right before drawing

    for (const auto& dc : cb.draws) glDrawArrays(dc.prim, dc.first, dc.count);

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    return true;
}

void GLRenderer::MatrixMode(int t) {
    if (t == GL_PROJECTION)
        s_matMode = 1;
    else if (t == GL_TEXTURE)
        s_matMode = 2;
    else
        s_matMode = 0;
}

void GLRenderer::MatrixSetIdentity() {
    activeStack().load(glm::mat4(1.f));
    markMatrixDirty();
    if (s_matMode == 0) markNormalDirty();
}
void GLRenderer::MatrixPush() {
    activeStack().push();
    // push doesn't change cur() so no dirty needed but mark anyway to be safe
    // ;w;
    markMatrixDirty();
    if (s_matMode == 0) markNormalDirty();
}
void GLRenderer::MatrixPop() {
    activeStack().pop();
    markMatrixDirty();
    if (s_matMode == 0) markNormalDirty();
}
void GLRenderer::MatrixTranslate(float x, float y, float z) {
    activeStack().mul(glm::translate(glm::mat4(1.f), {x, y, z}));
    markMatrixDirty();
    if (s_matMode == 0) markNormalDirty();
}
void GLRenderer::MatrixRotate(float a, float x, float y, float z) {
    activeStack().mul(glm::rotate(glm::mat4(1.f), a, {x, y, z}));
    markMatrixDirty();
    if (s_matMode == 0) markNormalDirty();
}
void GLRenderer::MatrixScale(float x, float y, float z) {
    activeStack().mul(glm::scale(glm::mat4(1.f), {x, y, z}));
    markMatrixDirty();
    if (s_matMode == 0) markNormalDirty();
}
void GLRenderer::MatrixPerspective(float fovy, float asp, float zn, float zf) {
    s_proj.cur() = glm::perspective(glm::radians(fovy), asp, zn, zf);
    markMatrixDirty();
}
void GLRenderer::MatrixOrthogonal(float l, float r, float b, float t, float zn,
                                  float zf) {
    s_proj.cur() = glm::ortho(l, r, b, t, zn, zf);
    markMatrixDirty();
}
void GLRenderer::MatrixMult(float* m) {
    activeStack().mul(glm::make_mat4(m));
    markMatrixDirty();
    if (s_matMode == 0) markNormalDirty();
}
const float* GLRenderer::MatrixGet(int t) {
    static float buf[16];
    glm::mat4* m = (t == GL_MODELVIEW_MATRIX)    ? &s_mv.cur()
                   : (t == GL_PROJECTION_MATRIX) ? &s_proj.cur()
                                                 : nullptr;
    if (m) memcpy(buf, glm::value_ptr(*m), 64);
    return buf;
}

void GLRenderer::Set_matrixDirty() {
    // iggy wipes opengl state
    s_boundProgram = 0;
    s_rs_dirty_mask = 0xFFFFFFFF;
    s_gl_shadow_mask = 0;
    s_normalMatDirty = true;  // normal matrix dirt after iggy reset
    s_matDirty = true;
    s_chunkOffsetValid = false;
    if (s_shader.prog) {
        glUseProgram(s_shader.prog);
        s_boundProgram = s_shader.prog;
    }
}

void GLRenderer::Clear(int f) { glClear(f); }
void GLRenderer::SetClearColour(const float c[4]) {
    glClearColor(c[0], c[1], c[2], c[3]);
}
bool GLRenderer::IsWidescreen() { return true; }
bool GLRenderer::IsHiDef() { return true; }
void GLRenderer::StateSetColour(float r, float g, float b, float a) {
    glm::vec4 v = {r, g, b, a};
    if (s_rs.baseColor != v) {
        s_rs.baseColor = v;
        markDirty(DIRTY_BASECOLOR);
    }
}
void GLRenderer::SetChunkOffset(float x, float y, float z) {
    if (s_shader.uChunkOffset < 0) return;
    glm::vec3 v = {x, y, z};
    if (!s_chunkOffsetValid || s_chunkOffset != v) {
        s_chunkOffset = v;
        s_chunkOffsetValid = true;
    }
    if (s_boundProgram == s_shader.prog) {
        glUniform3f(s_shader.uChunkOffset, x, y, z);
    }
}
void GLRenderer::StateSetDepthMask(bool e) {
    glShadowSetDepthMask(e ? GL_TRUE : GL_FALSE);
}
void GLRenderer::StateSetBlendEnable(bool e) { glShadowSetBlend(e); }
void GLRenderer::StateSetBlendFunc(int s, int d) { glShadowSetBlendFunc(s, d); }
void GLRenderer::StateSetDepthFunc(int f) { glDepthFunc(f); }
void GLRenderer::StateSetFaceCull(bool e) { glShadowSetCull(e); }
void GLRenderer::StateSetFaceCullCW(bool e) {
    glShadowSetFrontFace(e ? GL_CW : GL_CCW);
}
void GLRenderer::StateSetLineWidth(float w) {
#ifndef GLES
    glShadowSetLineWidth(w);
#else
    (void)w;
#endif
}
void GLRenderer::StateSetWriteEnable(bool r, bool g, bool b, bool a) {
    glShadowSetColorMask(r, g, b, a);
}
void GLRenderer::StateSetDepthTestEnable(bool e) { glShadowSetDepthTest(e); }
void GLRenderer::StateSetAlphaTestEnable(bool e) {
    float v = e ? 0.1f : 0.f;
    if (s_rs.alphaRef != v) {
        s_rs.alphaRef = v;
        markDirty(DIRTY_ALPHA);
    }
}
void GLRenderer::StateSetAlphaFunc(int, float p) {
    if (s_rs.alphaRef != p) {
        s_rs.alphaRef = p;
        markDirty(DIRTY_ALPHA);
    }
}

static int texGenBit(int coord) {
    switch (coord) {
        case 0x2000:
            return 1;  // GL_S
        case 0x2001:
            return 2;  // GL_T
        case 0x2002:
            return 4;  // GL_R
        case 0x2003:
            return 8;  // GL_Q
        default:
            return 0;
    }
}

void GLRenderer::StateSetShadeModel(int mode) {
    bool flat = (mode == 0x1D00 /*GL_FLAT*/);
    if (s_rs.flatShading != flat) {
        s_rs.flatShading = flat;
        markDirty(DIRTY_SHADE);
    }
}

void GLRenderer::StateSetTexGenCol(int coord, float x, float y, float z,
                                   float w, bool eyeSpace) {
    int bit = texGenBit(coord);
    if (!bit) return;
    glm::vec4 p = {x, y, z, w};
    switch (bit) {
        case 1:
            s_rs.texGenS = p;
            break;
        case 2:
            s_rs.texGenT = p;
            break;
        case 4:
            s_rs.texGenR = p;
            break;
        case 8:
            s_rs.texGenQ = p;
            break;
    }
    if (eyeSpace)
        s_rs.texGenEye |= bit;
    else
        s_rs.texGenEye &= ~bit;
    markDirty(DIRTY_TEXGEN);
}

void GLRenderer::StateSetTexGenEnable(int coord, bool enable) {
    int bit = texGenBit(coord);
    if (!bit) return;
    int mask = s_rs.texGenMask;
    if (enable)
        mask |= bit;
    else
        mask &= ~bit;
    if (mask != s_rs.texGenMask) {
        s_rs.texGenMask = mask;
        markDirty(DIRTY_TEXGEN);
    }
}
void GLRenderer::StateSetDepthSlopeAndBias(float s, float b) {
    glShadowSetPolygonOffset(s, b);
}
void GLRenderer::StateSetBlendFactor(unsigned int col) {
    float a = ((col >> 24) & 0xFF) / 255.f;
    float r = ((col >> 16) & 0xFF) / 255.f;
    float g = ((col >> 8) & 0xFF) / 255.f;
    float b = (col & 0xFF) / 255.f;
    glBlendColor(r, g, b, a);
}
void GLRenderer::StateSetFogEnable(bool e) {
    if (s_rs.fogEnable != e) {
        s_rs.fogEnable = e;
        markDirty(DIRTY_FOG);
    }
}
void GLRenderer::StateSetFogMode(int mode) {
    int v = (mode == GL_LINEAR) ? 1
            : (mode == GL_EXP)  ? 2
            : (mode == 0x0801)  ? 3
                                : 0;
    if (s_rs.fogMode != v) {
        s_rs.fogMode = v;
        markDirty(DIRTY_FOG);
    }
}
void GLRenderer::StateSetFogNearDistance(float d) {
    if (s_rs.fogStart != d) {
        s_rs.fogStart = d;
        markDirty(DIRTY_FOG);
    }
}
void GLRenderer::StateSetFogFarDistance(float d) {
    if (s_rs.fogEnd != d) {
        s_rs.fogEnd = d;
        markDirty(DIRTY_FOG);
    }
}
void GLRenderer::StateSetFogDensity(float d) {
    if (s_rs.fogDensity != d) {
        s_rs.fogDensity = d;
        markDirty(DIRTY_FOG);
    }
}
void GLRenderer::StateSetFogColour(float r, float g, float b) {
    glm::vec4 v = {r, g, b, 1};
    if (s_rs.fogColor != v) {
        s_rs.fogColor = v;
        markDirty(DIRTY_FOG);
    }
}
void GLRenderer::StateSetLightingEnable(bool e) {
    if (s_rs.lighting != e) {
        s_rs.lighting = e;
        markDirty(DIRTY_LIGHTING);
    }
}
void GLRenderer::StateSetLightColour(int, float r, float g, float b) {
    glm::vec3 v = {r, g, b};
    if (s_rs.ldiff != v) {
        s_rs.ldiff = v;
        markDirty(DIRTY_LIGHTING);
    }
}
void GLRenderer::StateSetLightAmbientColour(float r, float g, float b) {
    glm::vec3 v = {r, g, b};
    if (s_rs.lamb != v) {
        s_rs.lamb = v;
        markDirty(DIRTY_LIGHTING);
    }
}
void GLRenderer::StateSetLightDirection(int light, float x, float y, float z) {
    glm::vec3 d = glm::normalize(glm::mat3(s_mv.cur()) * glm::vec3(x, y, z));
    if (light == 0) {
        if (s_rs.l0 != d) {
            s_rs.l0 = d;
            markDirty(DIRTY_LIGHTING);
        }
    } else {
        if (s_rs.l1 != d) {
            s_rs.l1 = d;
            markDirty(DIRTY_LIGHTING);
        }
    }
}
void GLRenderer::StateSetViewport(eViewportType) {
    glViewport(0, 0, s_windowWidth, s_windowHeight);
}
void GLRenderer::StateSetVertexTextureUV(float u, float v) {
    glm::vec2 val = {u, v};
    if (s_rs.globalLM != val) {
        s_rs.globalLM = val;
        markDirty(DIRTY_GLOBAL_LM);
    }
}
void GLRenderer::StateSetStencil(int fn, uint8_t ref, uint8_t fmask,
                                 uint8_t wmask) {
    glShadowSetStencil(fn, ref, fmask, wmask);
}
void GLRenderer::StateSetTextureEnable(bool e) {
    if (s_rs.activeTexture == 0 && s_rs.useTexture != e) {
        s_rs.useTexture = e;
        markDirty(DIRTY_TEXTURE);
    }
}
void GLRenderer::StateSetActiveTexture(int tex) {
    s_rs.activeTexture = (tex == 0x84C1 /*GL_TEXTURE1*/) ? 1 : 0;
}

int GLRenderer::TextureCreate() {
    GLuint id;
    glGenTextures(1, &id);
    return (int)id;
}
void GLRenderer::TextureFree(int i) {
    GLuint id = (GLuint)i;
    glDeleteTextures(1, &id);
}
void GLRenderer::TextureBind(int idx) {
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, idx < 0 ? 0 : (GLuint)idx);
}
void GLRenderer::TextureBindVertex(int idx, bool scaleLight) {
    if (idx < 0) {
        if (s_rs.useLightmap) {
            s_rs.useLightmap = false;
            markDirty(DIRTY_TEXTURE);
        }
        glActiveTexture(GL_TEXTURE0);
        return;
    }
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, (GLuint)idx);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glActiveTexture(GL_TEXTURE0);
    if (!s_rs.useLightmap) {
        s_rs.useLightmap = true;
        markDirty(DIRTY_TEXTURE);
    }
    glm::vec4 newLmt = scaleLight
                           ? glm::vec4{1.f, 1.f, 8.f / 256.f, 8.f / 256.f}
                           : glm::vec4{1.f, 1.f, 0.f, 0.f};
    if (s_rs.lmt != newLmt) {
        s_rs.lmt = newLmt;
        markDirty(DIRTY_LMT);
    }
}
void GLRenderer::TextureSetTextureLevels(int l) {
    // GL_TEXTURE_MAX_LEVEL is not supported in GLES 2.0.
    // We only need to toggle the min filter to enable/disable mipmapping.
    if (l > 1)
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                        GL_NEAREST_MIPMAP_LINEAR);
    else
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
}
int GLRenderer::TextureGetTextureLevels() { return 1; }

bool isPowerOfTwo(int n) { return (n > 0 && (n & (n - 1)) == 0); }

void GLRenderer::TextureData(int w, int h, void* d, int lvl,
                             eTextureFormat format) {
    glTexImage2D(GL_TEXTURE_2D, lvl, GL_RGBA, w, h, 0, GL_RGBA,
                 GL_UNSIGNED_BYTE, d);

    if (lvl == 0) {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        if (isPowerOfTwo(w) && isPowerOfTwo(h)) {
            glGenerateMipmap(GL_TEXTURE_2D);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                            GL_NEAREST_MIPMAP_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        } else {
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        }
    }
}
void GLRenderer::TextureDataUpdate(int xo, int yo, int w, int h, void* d,
                                   int lvl) {
    glTexSubImage2D(GL_TEXTURE_2D, lvl, xo, yo, w, h, GL_RGBA, GL_UNSIGNED_BYTE,
                    d);
}
void GLRenderer::TextureSetParam(int p, int v) {
    glTexParameteri(GL_TEXTURE_2D, p, v);
}

static int stbLoad(unsigned char* data, int w, int h, D3DXIMAGE_INFO* info,
                   int** out) {
    int* px = new int[w * h];
    for (int i = 0; i < w * h; i++) {
        unsigned char r = data[i * 4], g = data[i * 4 + 1], b = data[i * 4 + 2],
                      a = data[i * 4 + 3];
        px[i] = (a << 24) | (r << 16) | (g << 8) | b;
    }
    if (info) {
        info->Width = w;
        info->Height = h;
    }
    *out = px;
    return 0;  // Success
}
int GLRenderer::LoadTextureData(const char* fn, D3DXIMAGE_INFO* i, int** o) {
    int w, h, c;
    unsigned char* d = stbi_load(fn, &w, &h, &c, 4);
    if (!d) return -1;  // Failure
    int hr = stbLoad(d, w, h, i, o);
    stbi_image_free(d);
    return hr;
}
int GLRenderer::LoadTextureData(uint8_t* pb, uint32_t nb, D3DXIMAGE_INFO* i,
                                int** o) {
    int w, h, c;
    unsigned char* d = stbi_load_from_memory(pb, (int)nb, &w, &h, &c, 4);
    if (!d) return -1;  // Failure
    int hr = stbLoad(d, w, h, i, o);
    stbi_image_free(d);
    return hr;
}

// TODO: TO REMOVE SOON.
void GLRenderer::UpdateGamma(unsigned short usGamma) {
    constexpr unsigned short GAMMA_MAX = 32768;
    s_rs.gamma = 0.5f + ((float)(usGamma) * (1.0f / GAMMA_MAX));
}

// MARK: C hooks

int glGenTextures_4J() {
    GLuint id = 0;
    glGenTextures(1, &id);
    return (int)id;
}

void glGenTextures_4J(int n, unsigned int* textures) {
    glGenTextures(n, textures);
}

void glDeleteTextures_4J(int id) {
    GLuint uid = (GLuint)id;
    glDeleteTextures(1, &uid);
}

void glDeleteTextures_4J(int n, const unsigned int* textures) {
    glDeleteTextures(n, textures);
}

// MARK: LinuxStubs

#ifdef GLES
extern "C" {
#undef glClearDepth
void glClearDepth(double depth) { glClearDepthf((float)depth); }
#undef glTexGeni
void glTexGeni(unsigned int, unsigned int, int) {}
#undef glTexGenfv
void glTexGenfv(unsigned int, unsigned int, const float*) {}
#undef glTexCoordPointer
void glTexCoordPointer(int, unsigned int, int, const void*) {}
#undef glNormalPointer
void glNormalPointer(unsigned int, int, const void*) {}
#undef glColorPointer
void glColorPointer(int, unsigned int, int, const void*) {}
#undef glVertexPointer
void glVertexPointer(int, unsigned int, int, const void*) {}
#undef glEndList
void glEndList(void) {}
#undef glCallLists
void glCallLists(int, unsigned int, const void*) {}
}
#endif

inline int* getIntPtr(IntBuffer* buf) {
    return buf ? (int*)buf->getBuffer() + buf->position() : nullptr;
}
inline void* getBytePtr(ByteBuffer* buf) {
    return buf ? (char*)buf->getBuffer() + buf->position() : nullptr;
}

void glGenTextures_4J(IntBuffer* buf) {
    if (!buf) return;
    int n = buf->limit() - buf->position();
    int* dst = getIntPtr(buf);
    for (int i = 0; i < n; i++) dst[i] = PlatformRenderer.TextureCreate();
}

void glDeleteTextures_4J(IntBuffer* buf) {
    if (!buf) return;
    int n = buf->limit() - buf->position();
    int* src = getIntPtr(buf);
    for (int i = 0; i < n; i++) PlatformRenderer.TextureFree(src[i]);
}

void glTexImage2D_4J(int target, int level, int internalformat, int width,
                     int height, int border, int format, int type,
                     ByteBuffer* pixels) {
    (void)target;
    (void)internalformat;
    (void)border;
    (void)format;
    (void)type;
    PlatformRenderer.TextureData(width, height, getBytePtr(pixels), level,
                                 IPlatformRenderer::TEXTURE_FORMAT_RxGyBzAw);
}

void glLight_4J(int light, int pname, FloatBuffer* params) {
    const float* p = params->_getDataPointer();
    int idx = (light == 0x4001) ? 1 : 0;
    if (pname == 0x1203)
        PlatformRenderer.StateSetLightDirection(idx, p[0], p[1], p[2]);
    else if (pname == 0x1201)
        PlatformRenderer.StateSetLightColour(idx, p[0], p[1], p[2]);
    else if (pname == 0x1200)
        PlatformRenderer.StateSetLightAmbientColour(p[0], p[1], p[2]);
}

void glLightModel_4J(int pname, FloatBuffer* params) {
    if (pname == 0x0B53) {
        const float* p = params->_getDataPointer();
        PlatformRenderer.StateSetLightAmbientColour(p[0], p[1], p[2]);
    }
}

void glFog_4J(int pname, FloatBuffer* params) {
    const float* p = params->_getDataPointer();
    if (pname == 0x0B66) PlatformRenderer.StateSetFogColour(p[0], p[1], p[2]);
}

void glGetFloat_4J(int pname, FloatBuffer* params) {
    const float* m = PlatformRenderer.MatrixGet(pname);
    if (m) memcpy(params->_getDataPointer(), m, 16 * sizeof(float));
}

void glCallLists_4J(IntBuffer* lists) {
    if (!lists) return;
    int count = lists->limit() - lists->position();
    int* ids = getIntPtr(lists);
    for (int i = 0; i < count; i++)
        (void)PlatformRenderer.CBuffCall(ids[i], false);
}

void glReadPixels_4J(int x, int y, int w, int h, int f, int t, ByteBuffer* p) {
    (void)f;
    (void)t;
    PlatformRenderer.ReadPixels(x, y, w, h, getBytePtr(p));
}

// dead stubs
void glTexCoordPointer_4J(int, int, FloatBuffer*) {}
void glNormalPointer_4J(int, ByteBuffer*) {}
void glColorPointer_4J(int, bool, int, ByteBuffer*) {}
void glVertexPointer_4J(int, int, FloatBuffer*) {}
void glEndList_4J(int) {}
void glTexGen_4J(int, int, FloatBuffer*) {}

#include <stdio.h>
#include <string.h>

void glGetFloat(int pname, FloatBuffer* params) {
    glGetFloat_4J(pname, params);
}
