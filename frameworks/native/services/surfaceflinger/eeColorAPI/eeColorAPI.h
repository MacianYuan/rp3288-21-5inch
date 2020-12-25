#ifndef EECOLOR_API_H
#define EECOLOR_API_H

#include <vector>

#include <math/mat4.h>

typedef void(*__CompileShaders)();
typedef void(*__ReadTable)(int index, int bytes, std::vector<int32_t>& dataVector);
typedef void(*__EnableEEColor)(bool enable);
typedef bool(*__IsEEColorEnabled)();
typedef void(*__SetTableType)(int type);
typedef int(*__GetTableType)();
typedef void(*__UseProgram)();
typedef void(*__SetUniforms)(const android::mat4& texture, const android::mat4& color, const android::mat4& projection); 

namespace eeColor
{

const static char COMPILE_SHADERS_MANGLED[] = "_ZN7eeColor14CompileShadersEv";
const static char READ_TABLE_MANGLED[] = "_ZN7eeColor9ReadTableEiiRNSt3__16vectorIiNS0_9allocatorIiEEEE";
const static char ENABLE_EECOLOR_MANGLED[] = "_ZN7eeColor13EnableEEColorEb";
const static char IS_EECOLOR_ENABLED_MANGLED[] = "_ZN7eeColor16IsEEColorEnabledEv";
const static char SET_TABLE_TYPE_MANGLED[] = "_ZN7eeColor12SetTableTypeEi";
const static char GET_TABLE_TYPE_MANGLED[] = "_ZN7eeColor12GetTableTypeEv";
const static char USE_PROGRAM_MANGLED[] = "_ZN7eeColor10UseProgramEv";
const static char SET_UNIFORMS_MANGLED[] = "_ZN7eeColor11SetUniformsERKN7android6tmat44IfEES4_S4_";

struct APIFunctions
{
	__CompileShaders pCompileShaders;
	__ReadTable pReadTable;
	__EnableEEColor pEnableEEColor;
	__IsEEColorEnabled pIsEEColorEnabled;
	__SetTableType pSetTableType;
	__GetTableType pGetTableType;
	__UseProgram pUseProgram;
	__SetUniforms pSetUniforms;
};

void CompileShaders();
void ReadTable(int index, int bytes, std::vector<int32_t>& dataVector);
void EnableEEColor(bool enable);
bool IsEEColorEnabled();
void SetTableType(int type);
int GetTableType();
void UseProgram();
void SetUniforms(const android::mat4& texture, const android::mat4& color, const android::mat4& projection);

}

#endif
