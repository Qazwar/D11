cbuffer cbChangesPerFrame : register(b0) {
	float4 screenDimension;
	float4 screenCenter;
	matrix wvp;
};


Texture2D colorMap_ : register(t0);
SamplerState colorSampler_ : register(s0);


struct VS_Input {
	float3 pos  : POSITION;
	float2 tex : TEXCOORD0;
	float4 color : COLOR0;
};

struct PS_Input {
	float4 pos  : SV_POSITION;
	float2 tex0 : TEXCOORD0;
	float4 color : COLOR0;
};


PS_Input VS_Main(VS_Input vertex) {
	PS_Input vsOut = (PS_Input)0;
	float4 pos = float4(vertex.pos, 1.0);
	pos -= float4(screenCenter.x, screenCenter.y, 0.0, 1.0);
	vsOut.pos = mul(float4(pos.x,pos.y,0.0,1.0), wvp);
	vsOut.pos.z = 1.0;
	vsOut.tex0 = vertex.tex;
	vsOut.color = vertex.color;
	return vsOut;
}

float4 PS_Main(PS_Input frag) : SV_TARGET{
	return colorMap_.Sample(colorSampler_, frag.tex0) * frag.color;
}