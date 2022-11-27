
#ifndef RENDER_SHADER_ID
#define RENDER_SHADER_ID

//Индетификатор шейдера
typedef void* ShaderId;

//Индетификатор шейдера который может рисоватся с освещением (UberShader)
struct ShaderLightingId
{
        union
        {
                ShaderId mode[2];
                struct
                {
                        //lighting disabled (default render)
                        ShaderId off;
                        //lighting enabled
                        ShaderId on;
                };
        };


        ShaderLightingId()
        {
        }
};


#endif