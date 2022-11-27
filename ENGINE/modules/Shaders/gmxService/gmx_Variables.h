#ifndef GMXVARIABLES
#define GMXVARIABLES





//Отключить specular компоненту точечных источников
#define DISABLE_SPECUALR_LIGHT_FROM_POINT_LIGHTS 1


#define GMX_MAX_TEMP_REGS 8
#define GMX_RECOMMENDED_TEMP_REGS 6


//Параметры источников света, для ShaderModel 3.0
#define MAX_LIGHTS_IN_30_MODEL 16


bool bLightEnabled30[32];



/*
x = float fOneDivPointLightNearPow30[MAX_LIGHTS_IN_30_MODEL];
y = float fPointLightNear30[MAX_LIGHTS_IN_30_MODEL]; //unused
z = float fPointLightFar30[MAX_LIGHTS_IN_30_MODEL];  //unused
w = float fPointShadowAffect30[MAX_LIGHTS_IN_30_MODEL];
*/

float4 fPointLightParams[MAX_LIGHTS_IN_30_MODEL];

float4 vPointLightPos30[MAX_LIGHTS_IN_30_MODEL];
float4 vPointLightColor30[MAX_LIGHTS_IN_30_MODEL];

#ifdef DISABLE_SPECUALR_LIGHT_FROM_POINT_LIGHTS
float4 vPointLightColorSpecular30[MAX_LIGHTS_IN_30_MODEL];
#endif



float gmxAlphaRef = 0.5;



float4 vAmbientLight;
float4 vAmbientLightSpecular;
float4 vGlobalLightColor;
float4 vGlobalLightColorSpecular;
float4 vGlobalLightDirection;
float4 vGlobalLightDirectionSpec;

float4 vGlobalLightBackColor;



//Текстуры модели
texture GMX_TextureStage0;
texture GMX_TextureStage1;
texture GMX_TextureStage2;
texture GMX_TextureStage3;



/*
sampler SingleMap =
sampler_state
{
        Texture = <GMX_TextureStage0>;
        MipFilter = LINEAR;
        MinFilter = LINEAR;
        MagFilter = LINEAR;

        AddressU = Wrap;
        AddressV = Wrap;

};

sampler LightMap =
sampler_state
{
        Texture = <GMX_TextureStage1>;
        MipFilter = LINEAR;
        MinFilter = LINEAR;
        MagFilter = LINEAR;

        AddressU = Wrap;
        AddressV = Wrap;
};
*/


sampler DiffuseMap =
sampler_state
{
        Texture = <GMX_TextureStage1>;
        MipFilter = LINEAR;
        MinFilter = LINEAR;
        MagFilter = LINEAR;

        AddressU = Wrap;
        AddressV = Wrap;
};

sampler SpecularMap =
sampler_state
{
        Texture = <GMX_TextureStage0>;
        MipFilter = LINEAR;
        MinFilter = LINEAR;
        MagFilter = LINEAR;

        AddressU = Wrap;
        AddressV = Wrap;
};

sampler NormalMap =
sampler_state
{
        Texture = <GMX_TextureStage2>;
        MipFilter = LINEAR;
        MinFilter = LINEAR;
        MagFilter = LINEAR;

        AddressU = Wrap;
        AddressV = Wrap;
};






//Экранное изображение тени
texture GMX_ShadowQuad;

sampler ShadowMap =
sampler_state
{
        Texture = <GMX_ShadowQuad>;
        MinFilter = Linear;
        MagFilter = Linear;

        AddressU = Clamp;
        AddressV = Clamp;
};



//Цвет пользователя, для подсветки моделей
float4 GMX_ObjectsUserColor;


//Цвет тени
float4 GMX_ShadowColor;

//ViewPort текущий
float4 GMX_ShadowLimiters;

float4x4 GMX_ShadowMatrix;



#ifndef _XBOX

//Матрицы костей тут, на боксе через vfetch выбираем
float4 GMX_skinBones[225];

#endif

float3 mulByMatrix4x3 (float3 vec, float4 boneConst0, float4 boneConst1, float4 boneConst2)
{
        float fScale = boneConst2.y;

        float4 bone_part0 = boneConst0;
        float4 bone_part1 = boneConst1;

        float3 res; 
        res.x = dot(float4(vec, 1.0f), float4(bone_part0.xyz * fScale, bone_part0.w));
        res.y = dot(float4(vec, 1.0f), float4(bone_part1.xyz * fScale, bone_part1.w));

        float3 bone_part2 = cross(boneConst0.xyz, boneConst1.xyz);
        res.z = dot(float4(vec, 1.0f), float4(bone_part2 * fScale, boneConst2.w));

        return res;
}

float3 mulByMatrix3x3 (float3 vec, float4 boneConst0, float4 boneConst1, float4 boneConst2)
{
        float3 res; 
        res.x = dot((float3)vec, boneConst0.xyz);
        res.y = dot((float3)vec, boneConst1.xyz);

        float3 bone = cross(boneConst0.xyz, boneConst1.xyz);
        res.z = dot((float3)vec, float4(bone, boneConst2.w));
        return res;
}





#endif