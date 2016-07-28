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
