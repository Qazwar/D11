cbuffer cbChangesPerFrame : register(b0) {
	float timer;
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
	float4 tex = colorMap_.Sample(colorSampler_, frag.tex0) * frag.color;
	float3 greyscale = dot(tex.rgb, float3(0.30, 0.59, 0.11));
	float3 rgb = lerp(tex.rgb, greyscale, timer);
	return float4(rgb, tex.a);
}