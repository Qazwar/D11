cbuffer cbChangesPerFrame : register(b0) {
	float time;
	float period;
	float shakeAmount;
	float frequency;
};

Texture2D colorMap_ : register( t0 );
SamplerState colorSampler_ : register( s0 );

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
    vsOut.pos = float4(vertex.pos,1.0);
    vsOut.tex0 = vertex.tex;
    vsOut.color = vertex.color;
    return vsOut;
}

float4 PS_Main( PS_Input frag ) : SV_TARGET {
	float2 uv = frag.tex0;
	uv.x += cos((time / period) * 2 * 3.14159 * frequency) * shakeAmount;
	uv.y += cos((time / period) * 2 * 3.14159 * frequency) * shakeAmount;
	return colorMap_.Sample(colorSampler_, uv) * frag.color;
}