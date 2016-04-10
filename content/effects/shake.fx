uniform extern float4x4 gWVP;
uniform extern texture gTex;
float timer = 1.0f;
float period = 5.0f;
float shakeAmount = 20.0f / 1024.0f;

sampler TexS = sampler_state {
	Texture = <gTex>;
	MinFilter = LINEAR;
	MagFilter = LINEAR;
	MipFilter = LINEAR;
	//AddressU  = WRAP;
    //AddressV  = WRAP;
};

struct OutputVS {
    float4 posH   : POSITION0;
	float2 Tex   : TEXCOORD0;
	float4 color0 : COLOR0;
};


OutputVS ShakeVS(float3 posL : POSITION0,float2 tex0 : TEXCOORD0 , float4 color : COLOR0) {
    OutputVS outVS = (OutputVS)0;	
	outVS.posH = mul(float4(posL, 1.0f), gWVP);		
	//outVS.posH.x += cos( ( timer / period ) * 2 * 3.14159 ) * shakeAmount;
	outVS.Tex = tex0;
	outVS.color0 = color;
	return outVS;
}

float4 ShakePS(OutputVS input) : COLOR {    
	float2 vTexCoord = input.Tex;
	vTexCoord.x += cos( ( timer * period ) * 2 * 3.14159 ) * shakeAmount;
	float4 tex  = tex2D(TexS,vTexCoord); 
	return tex;
}

technique ShakeTech {
    pass P0 {
        vertexShader = compile vs_2_0 ShakeVS();
        pixelShader  = compile ps_2_0 ShakePS();
    }
}
