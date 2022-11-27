/*
#ifndef COMPILER_MODE_DESCRIPTION_H
#define COMPILER_MODE_DESCRIPTION_H


enum ShaderCompilerMode
{
        //Пробовать загрузить уже скомпиленый fxo (win32shaders30.fxo или X360Shaders.fxo)
        //если файла нужного нет, то Assert
        SCM_USE_ONLY_COMPILED_FXO = 0,

        //Пробовать загрузить уже скомпиленый fxo (win32shaders30.fxo или X360Shaders.fxo)
        //если файла нужного нет, то компилить из папки Modules\\Shaders
        SCM_USE_COMPILED__COMPILE_IF_NOT_FOUND = 1,

        //Только сгенерить файл : win32shaders30.fxo
        SCM_ONLY_COMPILE_PC_SHADERMODEL_3 = 3,

        //Только сгенерить файл : x360shaders.fx
        SCM_ONLY_PERPARE_FOR_CONVERT_X360 = 4,

        SCM_FORCE_DWORD = 0x7fffffff
};


#endif
*/