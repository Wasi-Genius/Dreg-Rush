#include "ShaderProgram.h"

ShaderProgram::~ShaderProgram()
{
    unload();
}

bool ShaderProgram::load(
    const char *vertexFilePath,
    const char *fragmentFilePath
)
{
    unload();

    mShader = LoadShader(vertexFilePath, fragmentFilePath);
    mIsLoaded = mShader.id > 0;

    if (!mIsLoaded)
        LOG("Failed to load shader program.");

    return mIsLoaded;
}

void ShaderProgram::unload()
{
    if (!mIsLoaded) return;

    UnloadShader(mShader);
    mShader = {};
    mIsLoaded = false;
}

void ShaderProgram::begin() const
{
    if (!mIsLoaded) return;
    BeginShaderMode(mShader);
}

void ShaderProgram::end() const
{
    if (!mIsLoaded) return;
    EndShaderMode();
}

void ShaderProgram::setVector2(const char *uniformName, Vector2 value) const
{
    if (!mIsLoaded) return;

    const int location = GetShaderLocation(mShader, uniformName);
    if (location != NOT_LOADED)
        SetShaderValue(mShader, location, &value, SHADER_UNIFORM_VEC2);
}

void ShaderProgram::setFloat(const char *uniformName, float value) const
{
    if (!mIsLoaded) return;

    const int location = GetShaderLocation(mShader, uniformName);
    if (location != NOT_LOADED)
        SetShaderValue(mShader, location, &value, SHADER_UNIFORM_FLOAT);
}

void ShaderProgram::setInt(const char *uniformName, int value) const
{
    if (!mIsLoaded) return;

    const int location = GetShaderLocation(mShader, uniformName);
    if (location != NOT_LOADED)
        SetShaderValue(mShader, location, &value, SHADER_UNIFORM_INT);
}
