#include "technique.h"
#include "Particles\ParticlesCommon.h"
#include "Particles\ParticlesStruct.h"
#include "Particles\ParticlesVertexShader.h"
#include "Particles\ParticlesPixelShader.h"



//Стандартные мягкие партиклы
Technique(Particles, Particles_VS_IN)
{
	pass P0
	{
		CullMode = none;
		ZWriteenable = false;

		SrcBlend = one;
		DestBlend = invsrcalpha;
		AlphaTestEnable = false;
		AlphaBlendEnable = true;

		VertexShader = compile vs_2_0 vs_Particles();
		PixelShader = compile ps_2_0 ps_Particles(false, true);
	}
}

//Стандартные партиклы (не мягкие) для отражений
Technique(Particles_simple, Particles_VS_IN)
{
	pass P0
	{
		CullMode = none;
		ZWriteenable = false;

		SrcBlend = one;
		DestBlend = invsrcalpha;
		AlphaTestEnable = false;
		AlphaBlendEnable = true;

		VertexShader = compile vs_2_0 vs_Particles();
		PixelShader = compile ps_2_0 ps_Particles(false, false);
	}
}

//Стандартные партиклы haze
Technique(ParticlesDistor, Particles_VS_IN)
{
	pass P0
	{
		CullMode = none;
		ZWriteenable = false;

		AlphaTestEnable = false;
		AlphaBlendEnable = false;

		VertexShader = compile vs_2_0 vs_Particles();
		PixelShader = compile ps_2_0 ps_ParticlesDistorsion(false);
	}
}


//Партиклы в плоскости XZ - мягкие
Technique(ParticlesXZ, Particles_VS_IN)
{
	pass P0
	{
		CullMode = none;
		ZWriteenable = false;

		SrcBlend = one;
		DestBlend = invsrcalpha;
		AlphaTestEnable = false;
		AlphaBlendEnable = true;

		VertexShader = compile vs_2_0 vs_ParticlesXZ();
		PixelShader = compile ps_2_0 ps_Particles(false, true);
	}
}

//Партиклы в плоскости XZ - для отражений
Technique(ParticlesXZ_simple, Particles_VS_IN)
{
	pass P0
	{
		CullMode = none;
		ZWriteenable = false;

		SrcBlend = one;
		DestBlend = invsrcalpha;
		AlphaTestEnable = false;
		AlphaBlendEnable = true;

		VertexShader = compile vs_2_0 vs_ParticlesXZ();
		PixelShader = compile ps_2_0 ps_Particles(false, false);
	}
}


//Партиклы в плоскости XZ - haze
Technique(ParticlesDistXZ, Particles_VS_IN)
{
	pass P0
	{
		CullMode = none;
		ZWriteenable = false;

		AlphaTestEnable = false;
		AlphaBlendEnable = false;

		VertexShader = compile vs_2_0 vs_ParticlesXZ();
		PixelShader = compile ps_2_0 ps_ParticlesDistorsion(false);
	}
}





//----------------------------------------------------------------------------
//
// overdraw
//
//----------------------------------------------------------------------------


Technique(Particles_overdraw, Particles_VS_IN)
{
	pass P0
	{
	    CullMode = none;
		ZWriteenable = false;

        SrcBlend = srcalpha;
		DestBlend = one;
                
        BlendOp = add;

		AlphaBlendEnable = true;
		AlphaTestEnable = false;

		VertexShader = compile vs_2_0 vs_Particles();
		PixelShader = compile ps_2_0 ps_Particles(true, false);
	}
}


Technique(ParticlesDistor_overdraw, Particles_VS_IN)
{
	pass P0
	{
	    CullMode = none;
		ZWriteenable = false;

        SrcBlend = srcalpha;
		DestBlend = one;

        BlendOp = add;

		AlphaBlendEnable = true;
		AlphaTestEnable = false;

		VertexShader = compile vs_2_0 vs_Particles();
		PixelShader = compile ps_2_0 ps_ParticlesDistorsion(true);
	}
}



Technique(ParticlesXZ_overdraw, Particles_VS_IN)
{
	pass P0
	{
	    CullMode = none;
		ZWriteenable = false;

        BlendOp = add;

        SrcBlend = srcalpha;
		DestBlend = one;

		AlphaBlendEnable = true;
		AlphaTestEnable = false;

		VertexShader = compile vs_2_0 vs_ParticlesXZ();
		PixelShader = compile ps_2_0 ps_Particles(true, false);
	}
}


Technique(ParticlesDistXZ_overdraw, Particles_VS_IN)
{
	pass P0
	{
	    CullMode = none;
		ZWriteenable = false;

        BlendOp = add;

        SrcBlend = srcalpha;
		DestBlend = one;

		AlphaBlendEnable = true;
		AlphaTestEnable = false;

		VertexShader = compile vs_2_0 vs_ParticlesXZ();
		PixelShader = compile ps_2_0 ps_ParticlesDistorsion(true);
	}
}
