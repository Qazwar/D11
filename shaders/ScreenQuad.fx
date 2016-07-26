Texture2D colorMap_ : register( t0 );
SamplerState colorSampler_ : register( s0 );


struct VS_Input {
    float3 pos  : POSITION;
	float4 tex : TEXCOORD0;
    float4 color : COLOR0;
};

struct PS_Input {
    float4 pos  : SV_POSITION;
    float2 tex0 : TEXCOORD0;
    float4 color : COLOR0;
};


GSPS_INPUT VS_Main( VS_Input vertex ) {
    GSPS_INPUT vsOut = ( GSPS_INPUT )0;
    vsOut.Pos = float4(vertex.pos,1.0);
    vsOut.Tex = vertex.tex;
    vsOut.Color = vertex.color;
    return vsOut;
}

float4 PS_Main( PS_Input frag ) : SV_TARGET {
    return colorMap_.Sample( colorSampler_, frag.tex0 ) * frag.color;    
}