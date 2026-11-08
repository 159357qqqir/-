#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <arm_neon.h>
#include <sched.h>
#include <unistd.h>
#include <EGL/egl.h>
#include <GLES2/gl2.h>

#define CPU_THREADS 8
#define MEM_PER_THREAD_MB 96

static void pin_cpu(int core_id)
{
    pid_t tid = gettid();
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(core_id, &cpuset);
    sched_setaffinity(tid, sizeof(cpu_set_t), &cpuset);
}

static void *cpu_burn(void *arg)
{
    int core = *(int *)arg;
    pin_cpu(core);

    size_t sz = (size_t)MEM_PER_THREAD_MB * 1024 * 1024;
    uint8_t *buf = malloc(sz);
    if (!buf) return NULL;
    memset(buf, 0xAA, sz);

    uint64x2_t v0 = vdupq_n_u64(0x1111111111111111ULL);
    uint64x2_t v1 = vdupq_n_u64(0x2222222222222222ULL);
    uint64x2_t v2 = vdupq_n_u64(0x3333333333333333ULL);
    uint64x2_t v3 = vdupq_n_u64(0x4444444444444444ULL);
    uint64x2_t v4 = vdupq_n_u64(0x5555555555555555ULL);
    uint64x2_t v5 = vdupq_n_u64(0x6666666666666666ULL);
    uint64x2_t v6 = vdupq_n_u64(0x7777777777777777ULL);
    uint64x2_t v7 = vdupq_n_u64(0x8888888888888888ULL);

    for (;;)
    {
        for (size_t p = 0; p < sz; p += 128)
        {
            __asm__ volatile (
                "prfm pldl1keep, [%0, #256]\n"
                "prfm pldl2keep, [%0, #512]\n"
                : : "r"(buf + p)
            );
            v0 = vaddq_u64(v0, v4);
            v1 = veorq_u64(v1, v5);
            v2 = vshlq_n_u64(v2, 4);
            v3 = vshrq_n_u64(v3, 6);
            v4 = vaddq_u64(v4, v7);
            v5 = veorq_u64(v5, v6);
            v6 = vshlq_n_u64(v6, 2);
            v7 = vshrq_n_u64(v7, 3);
            v0 = veorq_u64(v0, v3);
            v1 = vaddq_u64(v1, v2);
            v2 = veorq_u64(v2, v0);
            v3 = vaddq_u64(v3, v1);
            v4 = veorq_u64(v4, v7);
            v5 = vaddq_u64(v5, v6);
            v6 = veorq_u64(v6, v4);
            v7 = vaddq_u64(v7, v5);
            vst1q_u64((uint64_t *)(buf + p), v0);
            vst1q_u64((uint64_t *)(buf + p + 16), v1);
            vst1q_u64((uint64_t *)(buf + p + 32), v2);
            vst1q_u64((uint64_t *)(buf + p + 48), v3);
        }
    }
    free(buf);
    return NULL;
}

static const char *vert_src =
"attribute vec4 pos;\n"
"void main(){ gl_Position = pos; }\n";

static const char *frag_src =
"precision highp float;\n"
"void main(){\n"
"    vec4 v = vec4(0.1,0.2,0.3,0.4);\n"
"    for(int i=0;i<2048;i++){\n"
"        v = sin(v * 3.14159 + vec4(0.5));\n"
"        v = fract(v * 123.456);\n"
"    }\n"
"    gl_FragColor = v;\n"
"}\n";

static GLuint compile_shader(GLenum type, const char *src)
{
    GLuint s = glCreateShader(type);
    glShaderSource(s, 1, &src, NULL);
    glCompileShader(s);
    return s;
}

static void *gpu_burn(void *arg)
{
    (void)arg;
    EGLDisplay dpy = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    EGLint maj, min;
    if (!eglInitialize(dpy, &maj, &min))
    {
        fprintf(stderr, "EGL init failed\n");
        return NULL;
    }

    const EGLint cfg_attr[] = {
        EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
        EGL_SURFACE_TYPE, EGL_PBUFFER_BIT,
        EGL_NONE
    };
    EGLConfig cfg;
    EGLint ncfg;
    if (!eglChooseConfig(dpy, cfg_attr, &cfg, 1, &ncfg))
    {
        fprintf(stderr, "eglChooseConfig fail\n");
        return NULL;
    }

    const EGLint ctx_attr[] = {
        EGL_CONTEXT_CLIENT_VERSION, 2,
        EGL_NONE
    };
    EGLContext ctx = eglCreateContext(dpy, cfg, EGL_NO_CONTEXT, ctx_attr);
    if (ctx == EGL_NO_CONTEXT)
    {
        fprintf(stderr, "eglCreateContext fail\n");
        return NULL;
    }

    const EGLint pbuf_attr[] = {
        EGL_WIDTH, 1440,
        EGL_HEIGHT, 3200,
        EGL_NONE
    };
    EGLSurface surf = eglCreatePbufferSurface(dpy, cfg, pbuf_attr);
    if (surf == EGL_NO_SURFACE)
    {
        fprintf(stderr, "eglCreatePbufferSurface fail\n");
        return NULL;
    }

    if (!eglMakeCurrent(dpy, surf, surf, ctx))
    {
        fprintf(stderr, "eglMakeCurrent fail\n");
        return NULL;
    }

    GLuint vs = compile_shader(GL_VERTEX_SHADER, vert_src);
    GLuint fs = compile_shader(GL_FRAGMENT_SHADER, frag_src);
    GLuint prog = glCreateProgram();
    glAttachShader(prog, vs);
    glAttachShader(prog, fs);
    glLinkProgram(prog);
    glUseProgram(prog);

    GLfloat quad[] = {
        -1,-1, 1,-1, -1,1,
        -1,1, 1,-1, 1,1
    };
    GLuint vbo;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quad), quad, GL_STATIC_DRAW);

    GLint pos_loc = glGetAttribLocation(prog, "pos");
    glEnableVertexAttribArray(pos_loc);
    glVertexAttribPointer(pos_loc, 2, GL_FLOAT, GL_FALSE, 0, NULL);

    for (;;)
    {
        glClear(GL_COLOR_BUFFER_BIT);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glFlush();
    }
    return NULL;
}

int main(void)
{
    pthread_t cpu_tid[CPU_THREADS];
    int core_id[CPU_THREADS];
    for (int i = 0; i < CPU_THREADS; i++)
    {
        core_id[i] = i;
        pthread_create(&cpu_tid[i], NULL, cpu_burn, &core_id[i]);
    }

    pthread_t gpu_tid;
    pthread_create(&gpu_tid, NULL, gpu_burn, NULL);

    for (int i = 0; i < CPU_THREADS; i++)
    {
        pthread_join(cpu_tid[i], NULL);
    }
    pthread_join(gpu_tid, NULL);
    return 0;
}
