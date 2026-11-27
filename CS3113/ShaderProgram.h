#ifndef SHADER_PROGRAM_H
#define SHADER_PROGRAM_H

#include "cs3113.h"

// Handles loading a vertex/fragment shader pair, binding/unbinding it around draw calls, and uploading CPU values into GLSL uniforms.
class ShaderProgram
{
private:
    Shader mShader{};
    bool   mIsLoaded = false;

public:
    static constexpr int NOT_LOADED = -1;

    ShaderProgram() = default;
    ~ShaderProgram();

    // Load a vertex and fragment shader from disk via Raylib's LoadShader()
    bool load(const char *vertexFilePath, const char *fragmentFilePath);
    
    // Unload the shader from GPU memory safely (safe to call more than once)
    void unload();

    // Enable/disable this shader for subsequent Raylib draw calls
    void begin() const;
    void end() const;

    // Upload CPU variables to named GLSL uniforms
    void setVector2(const char *uniformName, Vector2 value) const;
    void setFloat(const char *uniformName, float value) const;
    void setInt(const char *uniformName, int value) const;

    bool   isLoaded()  const { return mIsLoaded; }
    Shader getShader() const { return mShader;   }
};

#endif // SHADER_PROGRAM_H
