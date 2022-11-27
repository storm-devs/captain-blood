#ifndef shadername_GENERATOR
#define shadername_GENERATOR


#include "technique.h"
#include "gmxService\gmx_VertexShader.h"
#include "gmxService\gmx_VertexShaderAnim.h"





//Вертексные шейдеры
VertexShader vshader_anim01 = compile vs_3_0 GMX_VertexShaderAnim(1);
VertexShader vshader_anim02 = compile vs_3_0 GMX_VertexShaderAnim(2);
VertexShader vshader_anim03 = compile vs_3_0 GMX_VertexShaderAnim(3);
VertexShader vshader_static = compile vs_3_0 GMX_VertexShader();



#define ANIMATED_UBERSHADER_01_PRIMARY(name,shadername,mode) \
	Technique(name##_anim_1, GMX_VS_IN_ANIM) \
{ \
	pass P0 \
	{ \
	AlphaBlendEnable = true; \
	AlphaTestEnable = false; \
	SrcBlend = srcalpha; \
	DestBlend = invsrcalpha; \
	VertexShader = (vshader_anim01); \
	PixelShader = compile ps_3_0 shadername(true, mode); \
	} \
} \
	Technique(LIGHT_##name##_anim_1, GMX_VS_IN_ANIM) \
{ \
	pass P0 \
{ \
	AlphaBlendEnable = true; \
	AlphaTestEnable = false; \
	SrcBlend = srcalpha; \
	DestBlend = invsrcalpha; \
	VertexShader = (vshader_anim01); \
	PixelShader = compile ps_3_0 shadername(false, mode); \
} \
}
//*******************************************************************************************


#define ANIMATED_UBERSHADER_02_PRIMARY(name,shadername,mode) \
	Technique(name##_anim_2, GMX_VS_IN_ANIM) \
{ \
	pass P0 \
	{ \
	AlphaBlendEnable = true; \
	AlphaTestEnable = false; \
	SrcBlend = srcalpha; \
	DestBlend = invsrcalpha; \
	VertexShader = (vshader_anim02); \
	PixelShader = compile ps_3_0 shadername(true, mode); \
	} \
} \
Technique(LIGHT_##name##_anim_2, GMX_VS_IN_ANIM) \
{ \
	pass P0 \
{ \
	AlphaBlendEnable = true; \
	AlphaTestEnable = false; \
	SrcBlend = srcalpha; \
	DestBlend = invsrcalpha; \
	VertexShader = (vshader_anim02); \
	PixelShader = compile ps_3_0 shadername(false,mode); \
} \
} 
//*******************************************************************************************







#define ANIMATED_UBERSHADER_03_PRIMARY(name,shadername,mode) \
	Technique(name##_anim_3, GMX_VS_IN_ANIM) \
{ \
	pass P0 \
	{ \
	AlphaBlendEnable = true; \
	AlphaTestEnable = false; \
	SrcBlend = srcalpha; \
	DestBlend = invsrcalpha; \
	VertexShader = (vshader_anim03); \
	PixelShader = compile ps_3_0 shadername(true, mode); \
	} \
} \
Technique(LIGHT_##name##_anim_3, GMX_VS_IN_ANIM) \
{ \
 pass P0 \
  { \
	AlphaBlendEnable = true; \
	AlphaTestEnable = false; \
	SrcBlend = srcalpha; \
	DestBlend = invsrcalpha; \
	VertexShader = (vshader_anim03); \
	PixelShader = compile ps_3_0 shadername(false, mode); \
  } \
}//*******************************************************************************************



#define STATIC_UBERSHADER_PRIMARY(name,shadername,mode) \
	Technique(name, GMX_VS_IN_STATIC) \
{ \
	pass P0 \
	{ \
	AlphaBlendEnable = true; \
	AlphaTestEnable = false; \
	SrcBlend = srcalpha; \
	DestBlend = invsrcalpha; \
	VertexShader = (vshader_static); \
	PixelShader = compile ps_3_0 shadername(true, mode); \
	} \
} \
	Technique(LIGHT_##name, GMX_VS_IN_STATIC) \
{ \
	pass P0 \
	{ \
	AlphaBlendEnable = true; \
	AlphaTestEnable = false; \
	SrcBlend = srcalpha; \
	DestBlend = invsrcalpha; \
	VertexShader = (vshader_static); \
	PixelShader = compile ps_3_0 shadername(false,mode); \
	} \
} 
//*******************************************************************************************


#define STATIC_UBERSHADER(name,shadername,mode) STATIC_UBERSHADER_PRIMARY(name,shadername,mode)
#define ANIMATED_UBERSHADER_01(name,shadername,mode) ANIMATED_UBERSHADER_01_PRIMARY(name,shadername,mode)
#define ANIMATED_UBERSHADER_02(name,shadername,mode) ANIMATED_UBERSHADER_02_PRIMARY(name,shadername,mode)
#define ANIMATED_UBERSHADER_03(name,shadername,mode) ANIMATED_UBERSHADER_03_PRIMARY(name,shadername,mode)


#endif