/*
 * Copyright 2013 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

//#define LOG_NDEBUG 0
#undef LOG_TAG
#define LOG_TAG "RenderEngine"
#define ATRACE_TAG ATRACE_TAG_GRAPHICS

#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

#include <ui/ColorSpace.h>
#include <ui/DebugUtils.h>
#include <ui/Rect.h>

#include <utils/String8.h>
#include <utils/Trace.h>

#include <cutils/compiler.h>
#include <gui/ISurfaceComposer.h>
#include <math.h>

#include "GLES20RenderEngine.h"
#include "Program.h"
#include "ProgramCache.h"
#include "Description.h"
#include "Mesh.h"
#include "Texture.h"

#include <sstream>
#include <fstream>

// ---------------------------------------------------------------------------
#ifdef USE_HWC2
bool checkGlError(const char* op, int lineNumber) {
    bool errorFound = false;
    GLint error = glGetError();
    while (error != GL_NO_ERROR) {
        errorFound = true;
        error = glGetError();
        ALOGV("after %s() (line # %d) glError (0x%x)\n", op, lineNumber, error);
    }
    return errorFound;
}

static constexpr bool outputDebugPPMs = false;

void writePPM(const char* basename, GLuint width, GLuint height) {
    ALOGV("writePPM #%s: %d x %d", basename, width, height);

    std::vector<GLubyte> pixels(width * height * 4);
    std::vector<GLubyte> outBuffer(width * height * 3);

    // TODO(courtneygo): We can now have float formats, need
    // to remove this code or update to support.
    // Make returned pixels fit in uint32_t, one byte per component
    glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());
    if (checkGlError(__FUNCTION__, __LINE__)) {
        return;
    }

    std::string filename(basename);
    filename.append(".ppm");
    std::ofstream file(filename.c_str(), std::ios::binary);
    if (!file.is_open()) {
        ALOGE("Unable to open file: %s", filename.c_str());
        ALOGE("You may need to do: \"adb shell setenforce 0\" to enable "
              "surfaceflinger to write debug images");
        return;
    }

    file << "P6\n";
    file << width << "\n";
    file << height << "\n";
    file << 255 << "\n";

    auto ptr = reinterpret_cast<char*>(pixels.data());
    auto outPtr = reinterpret_cast<char*>(outBuffer.data());
    for (int y = height - 1; y >= 0; y--) {
        char* data = ptr + y * width * sizeof(uint32_t);

        for (GLuint x = 0; x < width; x++) {
            // Only copy R, G and B components
            outPtr[0] = data[0];
            outPtr[1] = data[1];
            outPtr[2] = data[2];
            data += sizeof(uint32_t);
            outPtr += 3;
        }
    }
    file.write(reinterpret_cast<char*>(outBuffer.data()), outBuffer.size());
}
#endif

unsigned char xxx1_tab[] = {
     44, 117,  65, 204,   44, 117,  65, 204,   44, 117,  65, 204,   44, 117,  65, 204,
     44, 117,  65, 204,   44, 117,  65, 204,   44, 117,  65, 204,   44, 117,  65, 204,
     44, 169,  68,   9,   45,  91,  75,  95,   46,  16,  82, 117,   46, 201,  89,  78,
     47, 133,  95, 234,   48,  69, 102,  75,   49,   9, 108, 114,   49, 208, 114,  96,
     50, 154, 120,  25,   51, 104, 125, 155,   52,  57, 130, 234,   53,  14, 136,   7,
     53, 229, 140, 243,   54, 192, 145, 176,   55, 158, 150,  63,   56, 127, 154, 161,
     57,  99, 158, 217,   58,  75, 162, 231,   59,  53, 166, 205,   60,  34, 170, 140,
     61,  18, 174,  38,   62,   5, 177, 156,   62, 251, 180, 238,   63, 243, 184,  32,
     64, 239, 187,  49,   65, 237, 190,  34,   66, 237, 192, 246,   67, 241, 195, 173,
     68, 247, 198,  71,   69, 255, 200, 199,   71,  10, 203,  46,   72,  24, 205, 123,
     73,  40, 207, 176,   74,  58, 209, 206,   75,  79, 211, 215,   76, 102, 213, 201,
     77, 127, 215, 168,   78, 155, 217, 115,   79, 185, 219,  43,   80, 217, 220, 208,
     81, 251, 222, 101,   83,  32, 223, 232,   84,  70, 225,  91,   85, 111, 226, 191,
     86, 153, 228,  20,   87, 198, 229,  91,   88, 244, 230, 148,   90,  37, 231, 192,
     91,  87, 232, 223,   92, 140, 233, 242,   93, 194, 234, 249,   94, 250, 235, 246,
     96,  51, 236, 231,   97, 111, 237, 206,   98, 172, 238, 171,   99, 235, 239, 127,
    101,  43, 240,  74,  102, 109, 241,  11,  103, 177, 241, 197,  104, 246, 242, 118,
    106,  61, 243,  32,  107, 133, 243, 194,  108, 207, 244,  93,  110,  26, 244, 241,
    111, 103, 245, 127,  112, 181, 246,   6,  114,   4, 246, 135,  115,  85, 247,   3,
    116, 167, 247, 121,  117, 250, 247, 233,  119,  78, 248,  85,  120, 164, 248, 187,
    121, 251, 249,  29,  123,  83, 249, 122,  124, 172, 249, 211,  126,   7, 250,  40,
    127,  98, 250, 121,  128, 191, 250, 198,  130,  29, 251,  16,  131, 123, 251,  86,
    132, 219, 251, 152,  134,  60, 251, 216,  135, 157, 252,  20,  137,   0, 252,  78,
    138,  99, 252, 132,  139, 200, 252, 184,  141,  45, 252, 233,  142, 147, 253,  24,
    143, 250, 253,  69,  145,  97, 253, 111,  146, 202, 253, 151,  148,  51, 253, 189,
    149, 157, 253, 225,  151,   8, 254,   3,  152, 115, 254,  35,  153, 223, 254,  66,
    155,  76, 254,  95,  156, 186, 254, 122,  158,  40, 254, 148,  159, 150, 254, 173,
    161,   6, 254, 196,  162, 117, 254, 217,  163, 230, 254, 238,  165,  87, 255,   1,
    166, 200, 255,  19,  168,  58, 255,  37,  169, 172, 255,  53,  171,  31, 255,  68,
    172, 147, 255,  82,  174,   6, 255,  95,  175, 123, 255, 108,  176, 239, 255, 119,
    178, 100, 255, 130,  179, 218, 255, 141,  181,  79, 255, 150,  182, 198, 255, 159,
    184,  60, 255, 167,  185, 179, 255, 175,  187,  42, 255, 182,  188, 161, 255, 189,
    190,  25, 255, 195,  191, 145, 255, 201,  193,   9, 255, 206,  194, 129, 255, 211,
    195, 250, 255, 216,  197, 113, 255, 222,  198, 231, 255, 230,  200,  92, 255, 239,
    201, 206, 255, 251,  203,  64, 255, 247,  204, 175, 255, 231,  206,  28, 255, 213,
    207, 135, 255, 193,  208, 240, 255, 170,  210,  86, 255, 144,  211, 186, 255, 116,
    213,  26, 255,  84,  214, 120, 255,  50,  215, 211, 255,  12,  217,  43, 254, 227,
    218, 127, 254, 183,  219, 208, 254, 135,  221,  29, 254,  84,  222, 102, 254,  29,
    223, 171, 253, 226,  224, 236, 253, 163,  226,  41, 253,  97,  227,  98, 253,  26,
    228, 150, 252, 208,  229, 197, 252, 129,  230, 240, 252,  46,  232,  22, 251, 214,
    233,  54, 251, 123,  234,  82, 251,  27,  235, 104, 250, 182,  236, 120, 250,  76,
    237, 131, 249, 222,  238, 136, 249, 108,  239, 135, 248, 244,  240, 128, 248, 120,
    241, 115, 247, 247,  242,  96, 247, 113,  243,  70, 246, 230,  244,  37, 246,  85,
    244, 254, 245, 192,  245, 208, 245,  38,  246, 154, 244, 134,  247,  94, 243, 225,
    248,  26, 243,  55,  248, 207, 242, 136,  249, 124, 241, 211,  250,  34, 241,  24,
    250, 192, 240,  89,  251,  85, 239, 147,  251, 227, 238, 200,  252, 104, 237, 248,
    252, 229, 237,  34,  253,  89, 236,  70,  253, 197, 235, 101,  254,  39, 234, 126,
    254, 129, 233, 145,  254, 210, 232, 158,  255,  25, 231, 166,  255,  87, 230, 167,
    255, 140, 229, 163,  255, 183, 228, 153,  255, 216, 227, 137,  255, 239, 226, 114,
    255, 252, 225,  86,  255, 255, 224, 115,  255, 255, 224, 115,  255, 255, 224, 115,
    255, 255, 224, 115,  255, 255, 224, 115,  255, 255, 224, 115,  255, 255, 224, 115,
    255, 255, 224, 115,  255, 255, 224, 115,  255, 255, 224, 115,  255, 255, 224, 115,
    255, 255, 224, 115,  255, 255, 224, 115,  255, 255, 224, 115,  255, 255, 224, 115,
    255, 255, 224, 115,  255, 255, 224, 115,  255, 255, 224, 115,  255, 255, 224, 115,
    255, 255, 224, 115,  255, 255, 224, 115,  255, 255, 224, 115,  255, 255, 224, 115,
    255, 255, 224, 115,  255, 255, 224, 115,  255, 255, 224, 115,  255, 255, 224, 115,
    255, 255, 224, 115,  255, 255, 224, 115,  255, 255, 224, 115,  255, 255, 224, 115,
    255, 255, 224, 115,  255, 255, 224, 115,  255, 255, 224, 115,  255, 255, 224, 115,
    255, 255, 224, 115,  255, 255, 224, 115,  255, 255, 224, 115,  255, 255, 224, 115,
    255, 255, 224, 115,  255, 255, 224, 115,  255, 255, 224, 115,  255, 255, 224, 115,
    255, 255, 224, 115,  255, 255, 224, 115,  255, 255, 224, 115,  255, 255, 224, 115,
    255, 255, 224, 115,  255, 255, 224, 115,  255, 255, 224, 115,  255, 255, 224, 115,
    255, 255, 224, 115,  255, 255, 224, 115,  255, 255, 224, 115,  255, 255, 224, 115,
    255, 255, 224, 115,  255, 255, 224, 115,  255, 255, 224, 115,  255, 255, 224, 115
};

// ---------------------------------------------------------------------------
namespace android {
// ---------------------------------------------------------------------------

GLES20RenderEngine::GLES20RenderEngine(uint32_t featureFlags) :
         mVpWidth(0),
         mVpHeight(0),
         mPlatformHasWideColor((featureFlags & WIDE_COLOR_SUPPORT) != 0) {

    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &mMaxTextureSize);
    glGetIntegerv(GL_MAX_VIEWPORT_DIMS, mMaxViewportDims);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
    glPixelStorei(GL_PACK_ALIGNMENT, 4);

    const uint16_t protTexData[] = { 0 };
    glGenTextures(1, &mProtectedTexName);
    glBindTexture(GL_TEXTURE_2D, mProtectedTexName);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1, 1, 0,
            GL_RGB, GL_UNSIGNED_SHORT_5_6_5, protTexData);

    //mColorBlindnessCorrection = M;
    loadMRatioTexturing();

#ifdef USE_HWC2
    if (mPlatformHasWideColor) {
        // Compute sRGB to DisplayP3 color transform
        // NOTE: For now, we are limiting wide-color support to
        // Display-P3 only.
        mat3 srgbToP3 = ColorSpaceConnector(ColorSpace::sRGB(), ColorSpace::DisplayP3()).getTransform();

        // color transform needs to be expanded to 4x4 to be what the shader wants
        // mat has an initializer that expands mat3 to mat4, but
        // not an assignment operator
        mat4 gamutTransform(srgbToP3);
        mSrgbToDisplayP3 = gamutTransform;
    }
#endif
}

GLES20RenderEngine::~GLES20RenderEngine() {
}


size_t GLES20RenderEngine::getMaxTextureSize() const {
    return mMaxTextureSize;
}

size_t GLES20RenderEngine::getMaxViewportDims() const {
    return
        mMaxViewportDims[0] < mMaxViewportDims[1] ?
            mMaxViewportDims[0] : mMaxViewportDims[1];
}

void GLES20RenderEngine::setViewportAndProjection(
        size_t vpw, size_t vph, Rect sourceCrop, size_t hwh, bool yswap,
        Transform::orientation_flags rotation) {

    size_t l = sourceCrop.left;
    size_t r = sourceCrop.right;

    // In GL, (0, 0) is the bottom-left corner, so flip y coordinates
    size_t t = hwh - sourceCrop.top;
    size_t b = hwh - sourceCrop.bottom;

    mat4 m;
    if (yswap) {
        m = mat4::ortho(l, r, t, b, 0, 1);
    } else {
        m = mat4::ortho(l, r, b, t, 0, 1);
    }

    // Apply custom rotation to the projection.
    float rot90InRadians = 2.0f * static_cast<float>(M_PI) / 4.0f;
    switch (rotation) {
        case Transform::ROT_0:
            break;
        case Transform::ROT_90:
            m = mat4::rotate(rot90InRadians, vec3(0,0,1)) * m;
            break;
        case Transform::ROT_180:
            m = mat4::rotate(rot90InRadians * 2.0f, vec3(0,0,1)) * m;
            break;
        case Transform::ROT_270:
            m = mat4::rotate(rot90InRadians * 3.0f, vec3(0,0,1)) * m;
            break;
        default:
            break;
    }

    glViewport(0, 0, vpw, vph);
    mState.setProjectionMatrix(m);
    mVpWidth = vpw;
    mVpHeight = vph;
}

#ifdef USE_HWC2
void GLES20RenderEngine::setupLayerBlending(bool premultipliedAlpha,
        bool opaque, float alpha) {
#else
void GLES20RenderEngine::setupLayerBlending(
    bool premultipliedAlpha, bool opaque, int alpha) {
#endif

    mState.setPremultipliedAlpha(premultipliedAlpha);
    mState.setOpaque(opaque);
#ifdef USE_HWC2
    mState.setPlaneAlpha(alpha);

    if (alpha < 1.0f || !opaque) {
#else
    mState.setPlaneAlpha(alpha / 255.0f);

    if (alpha < 0xFF || !opaque) {
#endif
        glEnable(GL_BLEND);

#if RK_USE_BLEND_SEPARATE
/*
    come from:
    commit 0d07097700683bb1c4b34fd6ea000fc0a4bf233d
    Author: huangds <hds@rock-chips.com>
    Date:   Sun Jan 4 11:31:07 2015 +0800

        modify for hwc new policy

        Signed-off-by: wzq <wzq@rock-chips.com>
*/
       glBlendFuncSeparate(premultipliedAlpha ? GL_ONE : GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
#endif

    } else {
        glDisable(GL_BLEND);
    }
}

#ifdef USE_HWC2
void GLES20RenderEngine::setupDimLayerBlending(float alpha) {
#else
void GLES20RenderEngine::setupDimLayerBlending(int alpha) {
#endif
    mState.setPlaneAlpha(1.0f);
    mState.setPremultipliedAlpha(true);
    mState.setOpaque(false);
#ifdef USE_HWC2
    mState.setColor(0, 0, 0, alpha);
#else
    mState.setColor(0, 0, 0, alpha/255.0f);
#endif
    mState.disableTexture();

#ifdef USE_HWC2
    if (alpha == 1.0f) {
#else
    if (alpha == 0xFF) {
#endif
        glDisable(GL_BLEND);
    } else {
        glEnable(GL_BLEND);
        glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
    }
}

#ifdef USE_HWC2
void GLES20RenderEngine::setColorMode(android_color_mode mode) {
    ALOGV("setColorMode: %s (0x%x)", decodeColorMode(mode).c_str(), mode);

    if (mColorMode == mode) return;

    if (!mPlatformHasWideColor || !mDisplayHasWideColor || mode == HAL_COLOR_MODE_SRGB ||
        mode == HAL_COLOR_MODE_NATIVE) {
        // We are returning back to our default color_mode
        mUseWideColor = false;
        mWideColorFrameCount = 0;
    } else {
        mUseWideColor = true;
    }

    mColorMode = mode;
}

void GLES20RenderEngine::setSourceDataSpace(android_dataspace source) {
    if (source == HAL_DATASPACE_UNKNOWN) {
        // Treat UNKNOWN as SRGB
        source = HAL_DATASPACE_V0_SRGB;
    }
    mDataSpace = source;
}

void GLES20RenderEngine::setWideColor(bool hasWideColor) {
    ALOGV("setWideColor: %s", hasWideColor ? "true" : "false");
    mDisplayHasWideColor = hasWideColor;
}

bool GLES20RenderEngine::usesWideColor() {
    return mUseWideColor;
}
#endif

void GLES20RenderEngine::setupLayerTexturing(const Texture& texture) {
    GLuint target = texture.getTextureTarget();
    glBindTexture(target, texture.getTextureName());
    GLenum filter = GL_NEAREST;
    if (texture.getFiltering()) {
        filter = GL_LINEAR;
    }
    glTexParameteri(target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(target, GL_TEXTURE_MAG_FILTER, filter);
    glTexParameteri(target, GL_TEXTURE_MIN_FILTER, filter);

    mState.setTexture(texture);
}

void GLES20RenderEngine::loadMRatioTexturing() {
    glGenTextures(1, &mMRatioTextureName);
    mMRatioTexture.init(Texture::TEXTURE_2D, mMRatioTextureName);
    mMRatioTexture.setDimensions(256, 1);
    mMRatioTexture.setFiltering(true); // GL_LINEAR

    GLuint target = mMRatioTexture.getTextureTarget();
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(target, mMRatioTexture.getTextureName());
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 256, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, (void *)xxx1_tab);
    GLenum filter = GL_NEAREST;
    if (mMRatioTexture.getFiltering()) {
        filter = GL_LINEAR;
    }
    glTexParameteri(target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glActiveTexture(GL_TEXTURE0);
}

void GLES20RenderEngine::setupHdr(bool status) {
    mState.setHdr(status);
}

void GLES20RenderEngine::setupMRatioTexturing() {
    mState.setMRatioTexture(mMRatioTexture);
}

void GLES20RenderEngine::setupLayerBlackedOut() {
    glBindTexture(GL_TEXTURE_2D, mProtectedTexName);
    Texture texture(Texture::TEXTURE_2D, mProtectedTexName);
    texture.setDimensions(1, 1); // FIXME: we should get that from somewhere
    mState.setTexture(texture);
}

mat4 GLES20RenderEngine::setupColorTransform(const mat4& colorTransform) {
    mat4 oldTransform = mState.getColorMatrix();
    mState.setColorMatrix(colorTransform);
    return oldTransform;
}

void GLES20RenderEngine::disableTexturing() {
    mState.disableTexture();
}

void GLES20RenderEngine::disableBlending() {
    glDisable(GL_BLEND);
}


void GLES20RenderEngine::bindImageAsFramebuffer(EGLImageKHR image,
        uint32_t* texName, uint32_t* fbName, uint32_t* status) {
    GLuint tname, name;
    // turn our EGLImage into a texture
    glGenTextures(1, &tname);
    glBindTexture(GL_TEXTURE_2D, tname);
    glEGLImageTargetTexture2DOES(GL_TEXTURE_2D, (GLeglImageOES)image);

    // create a Framebuffer Object to render into
    glGenFramebuffers(1, &name);
    glBindFramebuffer(GL_FRAMEBUFFER, name);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tname, 0);

    *status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    *texName = tname;
    *fbName = name;
}

void GLES20RenderEngine::bindyuvimg(EGLImageKHR image,GLuint name) {
    // turn our EGLImage into a texture
    //glGenTextures(1, &name);
    glBindTexture(GL_TEXTURE_EXTERNAL_OES, name);
    glEGLImageTargetTexture2DOES(GL_TEXTURE_EXTERNAL_OES, (GLeglImageOES)image);
}

void GLES20RenderEngine::unbindFramebuffer(uint32_t texName, uint32_t fbName) {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDeleteFramebuffers(1, &fbName);
    glDeleteTextures(1, &texName);
}

void GLES20RenderEngine::setupFillWithColor(float r, float g, float b, float a) {
    mState.setPlaneAlpha(1.0f);
    mState.setPremultipliedAlpha(true);
    mState.setOpaque(false);
    mState.setColor(r, g, b, a);
    mState.disableTexture();
    glDisable(GL_BLEND);
}

void GLES20RenderEngine::drawMesh(const Mesh& mesh) {

    if (mesh.getTexCoordsSize()) {
        glEnableVertexAttribArray(Program::texCoords);
        glVertexAttribPointer(Program::texCoords,
                mesh.getTexCoordsSize(),
                GL_FLOAT, GL_FALSE,
                mesh.getByteStride(),
                mesh.getTexCoords());
    }

    glVertexAttribPointer(Program::position,
            mesh.getVertexSize(),
            GL_FLOAT, GL_FALSE,
            mesh.getByteStride(),
            mesh.getPositions());

#ifdef USE_HWC2
    if (usesWideColor()) {
        Description wideColorState = mState;
        if (mDataSpace != HAL_DATASPACE_DISPLAY_P3) {
            wideColorState.setColorMatrix(mState.getColorMatrix() * mSrgbToDisplayP3);
            wideColorState.setWideGamut(true);
            ALOGV("drawMesh: gamut transform applied");
        }
        ProgramCache::getInstance().useProgram(wideColorState);

        glDrawArrays(mesh.getPrimitive(), 0, mesh.getVertexCount());

        if (outputDebugPPMs) {
            std::ostringstream out;
            out << "/data/texture_out" << mWideColorFrameCount++;
            writePPM(out.str().c_str(), mVpWidth, mVpHeight);
        }
    } else {
        ProgramCache::getInstance().useProgram(mState);

        glDrawArrays(mesh.getPrimitive(), 0, mesh.getVertexCount());
    }
#else
    ProgramCache::getInstance().useProgram(mState);

    glDrawArrays(mesh.getPrimitive(), 0, mesh.getVertexCount());
#endif

    if (mesh.getTexCoordsSize()) {
        glDisableVertexAttribArray(Program::texCoords);
    }
}

void GLES20RenderEngine::dump(String8& result) {
    RenderEngine::dump(result);
#ifdef USE_HWC2
    if (usesWideColor()) {
        result.append("Wide-color: On\n");
    } else {
        result.append("Wide-color: Off\n");
    }
#endif
}

// ---------------------------------------------------------------------------
}; // namespace android
// ---------------------------------------------------------------------------

#if defined(__gl_h_)
#error "don't include gl/gl.h in this file"
#endif
