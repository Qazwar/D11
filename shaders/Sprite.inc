cbuffer cbChangesPerFrame : register( b0 ) {
    float4 screenDimension;
};


Texture2D colorMap_ : register( t0 );
SamplerState colorSampler_ : register( s0 );


struct VS_Input {
    float3 pos  : POSITION;
    float4 tex : COLOR0;
    float3 size : NORMAL0;
    float4 color : COLOR1;
};

struct GSPS_INPUT {
    float4 Pos : SV_POSITION;
    float4 Tex : COLOR0;
    float3 Size : NORMAL0;    
    float4 Color : COLOR1;
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
    vsOut.Size = vertex.size;
    return vsOut;
}
