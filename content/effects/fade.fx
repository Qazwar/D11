uniform extern float4x4 gWVP;
uniform extern texture gTex;
float timer = 1.0f;
float4 grayscale = float4(0.2125,0.7154,0.0721,1.0);

sampler TexS = sampler_state {
	Texture = <gTex>;
	MinFilter = LINEAR;
	MagFilter = LINEAR;
	MipFilter = LINEAR;
	AddressU  = WRAP;
    AddressV  = WRAP;
};

struct OutputVS {
    float4 posH   : POSITION0;
	float2 Tex   : TEXCOORD0;
	float4 color0 : COLOR0;
};


OutputVS FadeVS(float3 posL : POSITION0,float2 tex0 : TEXCOORD0 , float4 color : COLOR0) {
    OutputVS outVS = (OutputVS)0;	
	outVS.posH = mul(float4(posL, 1.0f), gWVP);		
	outVS.Tex = tex0;
	outVS.color0 = color;
	return outVS;
}

float4 FadePS(OutputVS input) : COLOR {    
	float4 tex      = tex2D(TexS,input.Tex); 
    float3 greyscale = dot(tex.rgb, float3(0.30, 0.59, 0.11));     
    float3 rgb      = lerp(tex.rgb,greyscale, timer);      
	return float4(rgb,tex.a);	
}

technique FadeTech {
    pass P0 {
        vertexShader = compile vs_2_0 FadeVS();
        pixelShader  = compile ps_2_0 FadePS();
    }
}
