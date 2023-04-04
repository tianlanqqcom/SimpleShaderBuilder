#include "ShaderBuilder.h"
using namespace GLHelper;

bool ShaderBuilder::InitShaderBuilder()
{
    // Create shader first
    inputShader = glCreateShader(GL_VERTEX_SHADER);

    // Set shader source and compile
    glShaderSource(inputShader, 1, &inputShaderSource, 0);
    glCompileShader(inputShader);

    // Check if the shader is successfully compiled
    GLint success;
    glGetShaderiv(inputShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        // if failed, return false, note that the step is always successful.
        return false;
    }
    return true;
}

bool ShaderBuilder::strcpy_glchar(GLchar* dest, const std::string& source)
{
    // Due to the length of outputShaderSource_c is 512, so index should not be larger than 512
    int index = 0;

    // Get char from the source string and fill to the destination
    for (char c : source)
    {
        if (index >= 512)
        {
            return false;
        }
        dest[index++] = c;
    }

    // Add '\0' at the end of string
    dest[index] = 0;
    return true;
}

ShaderBuilder::ShaderBuilder()
{
    InitShaderBuilder();
}

ShaderBuilder::~ShaderBuilder()
{
    // Free the array
    delete outputShaderSource_c;

    // Delete output shader and clear map
    for (auto& oshader : compiledOutputShaderCache)
    {
        glDeleteShader(oshader.second);
    }
    compiledOutputShaderCache.clear();

    // Delete shader program and clear map
    for (auto& program : linkedProgramCache)
    {
        glDeleteProgram(program.second);
    }
    linkedProgramCache.clear();

    // Delete senior shader program and clear map
    for (auto& program : seniorShaderProgramCache)
    {
        glDeleteProgram(program.second);
    }
    seniorShaderProgramCache.clear();
}

void ShaderBuilder::SetOutputShaderSource(const Vector3float& vec3)
{
    // Call SetOutputShaderSource(float,float,float,float) instead
    SetOutputShaderSource(vec3.red, vec3.green, vec3.blue, 1.0f);
}

void ShaderBuilder::SetOutputShaderSource(const Vector4float& vec4)
{
    // Call SetOutputShaderSource(float,float,float,float) instead
    SetOutputShaderSource(vec4.red, vec4.green, vec4.blue, vec4.alpha);
}

void ShaderBuilder::SetOutputShaderSource(const float red, const  float green, const float blue, const float alpha)
{
    // Create shader source and write to buffer
    char buffer[512];
    snprintf(buffer, sizeof(buffer), "#version 330 core\n"
        "out vec4 color;\n"
        "void main()\n"
        "{\n"
        "color = vec4(%ff, %ff, %ff, %ff);\n"
        "}\n\0",
        red, green, blue, alpha);

    // Assign outputShaderSource with buffer
    outputShaderSource = buffer;
}

bool ShaderBuilder::BuildOutputShaderSource()
{
    // Create output shader
    outputShader = glCreateShader(GL_FRAGMENT_SHADER);
    // Copy std::string to a C-style string
    if (!strcpy_glchar(outputShaderSource_c, outputShaderSource))
    {
        return false;
    }
    
    // Create shader with the above string and compile
    glShaderSource(outputShader, 1, const_cast<const GLchar**>(&outputShaderSource_c), 0);
    glCompileShader(outputShader);

    // Check if the shader is successfully compiled
    GLint success;
    glGetShaderiv(outputShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        // if failed, return false
        return false;
    }
    return true;
}

bool ShaderBuilder::SetAndBuildOutputShaderSource(const Vector3float& vec3)
{
    // Call SetAndBuildOutputShaderSource(float,float,float,float) instead
    return SetAndBuildOutputShaderSource(vec3.red, vec3.green, vec3.blue, 1.0f);
}

bool ShaderBuilder::SetAndBuildOutputShaderSource(const Vector4float& vec4)
{
    // Call SetAndBuildOutputShaderSource(float,float,float,float) instead
    return SetAndBuildOutputShaderSource(vec4.red, vec4.green, vec4.blue, vec4.alpha);
}

bool ShaderBuilder::SetAndBuildOutputShaderSource(const float red, const float green, const float blue, const float alpha)
{
    // Call set and build function
    auto color = Vector4float(red, green, blue, alpha);
    SetOutputShaderSource(red, green, blue, alpha);
    if (!BuildOutputShaderSource())
    {
        return false;
    }

    // If successfully build, add to map 
    compiledOutputShaderCache[color] = outputShader;
    // Notes:
    // There is no need to check if there is already a shader in map, 
    // because the function is private and only be called in GetShaderProgram(const Vector4float& color),
    // the GetShaderProgram(const Vector4float& color) will check this.

    return true;
}

bool ShaderBuilder::LinkProgram()
{
    // Attach input and output shader and link
    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, inputShader);
    glAttachShader(shaderProgram, outputShader);
    glLinkProgram(shaderProgram);

    // Check success
    GLint success;
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success)
    {
        return false;
    }
    return true;
}

const GLuint ShaderBuilder::GetShaderProgram() const
{
    // Unused now.
    // Just return now shaderProgram, don't check anything,
    // because the called will check
    return shaderProgram;
}

const GLuint ShaderBuilder::GetShaderProgram(const float red, const float green, const float blue, const float alpha)
{
    // Call GetShaderProgram(Vector4float) instead
    return GetShaderProgram(Vector4float(red, green, blue, alpha));
}

const GLuint GLHelper::ShaderBuilder::GetUniqueShaderProgram(const int markId)
{
    // Find the markId in the map
    auto it = seniorShaderProgramCache.find(markId);
    if (it != seniorShaderProgramCache.end())
    {
        // If found, return
        return seniorShaderProgramCache[markId];
    }

    // If failed, return 0
    return 0;
}

const GLuint ShaderBuilder::GetShaderProgram(const Vector4float& color)
{
    // If input shader has been changed, all linked program will be useless, but the output shader is still useful.
    // So delete all programs but not delete output shader
    if (inputShaderChanged)
    {
        // Delete all programs
        for (auto& program : linkedProgramCache)
        {
            glDeleteProgram(program.second);
        }
        linkedProgramCache.clear();
        inputShaderChanged = false;
    }
    else
    {
        // Check if there is linked program, if has, return directly
        auto it = linkedProgramCache.find(color);
        if (it != linkedProgramCache.end())
        {
            return linkedProgramCache[color];
        }
    }

    // Find compiled shader, if found, that means the input shader has been changed, 
    // so link them and return
    auto it_shader = compiledOutputShaderCache.find(color);
    if (it_shader != compiledOutputShaderCache.end())
    {
        outputShader = compiledOutputShaderCache[color];
        LinkProgram();
        return shaderProgram;
    }

    // If not found, the color is a completely new color, try build and link
    if (!SetAndBuildOutputShaderSource(color))
    {
        // If failed to build
        return 0;
    }
    
    if (!LinkProgram())
    {
        // If failed to link
        return 0;
    }
    
    // If success
    return shaderProgram;
}

const GLuint ShaderBuilder::GetShaderProgram(const Vector3float& color)
{
    // Call GetShaderProgram(Vector4float) instead
    return GetShaderProgram(color.red, color.green, color.blue);
}

bool ShaderBuilder::SetInputShader(const GLchar* newInputShader)
{
    // The function is used to support senior input shaders, 
    // it is better to ensure your shader is right before calling this function
    
    // Delete original input shader
    glDeleteShader(inputShader);

    // Create new shader and compile
    inputShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(inputShader, 1, &newInputShader, 0);
    glCompileShader(inputShader);

    // Check compile success
    GLint success;
    glGetShaderiv(inputShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        // If failed, delete now shader and resume to the default input shader, to ensure a valid input shader
        glDeleteShader(inputShader);
        InitShaderBuilder();
        return false;
    }

    // If success, set flag, and the old programs will be cleaned at next call of GetShaderProgram
    inputShaderChanged = true;
    return true;
}

const GLuint ShaderBuilder::GetUniqueShaderProgram(const int markId, const GLchar* shaderSource)
{
    // The function is used to support senior output shaders, 
    // it is better to ensure your shader is right before calling this function
    // The markId is given by yourself, and you should organize the markId yourself

    // Create new shader and compile
    outputShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(outputShader, 1, &shaderSource, 0);
    glCompileShader(outputShader);

    // Check compile success
    GLint success;
    glGetShaderiv(outputShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        return 0;
    }
    
    // Create program with present input shader and the new output shader
    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, inputShader);
    glAttachShader(shaderProgram, outputShader);
    glLinkProgram(shaderProgram);
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);

    // Check link success
    if (!success)
    {
        return 0;
    }
    
    // Add to map
    seniorShaderProgramCache[markId] = shaderProgram;
    return shaderProgram;
}
