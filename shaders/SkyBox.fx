cbuffer cbChangesPerFrame : register( b0 ) {
    matrix mvp_;
    matrix world;
    float3 camera;
    float3 light;
};

float3 scale = float3(20,20,20);

TextureCube colorMap_ : register( t0 );
SamplerState colorSampler_ : register( s0 );

struct VS_Input
{
    float4 pos  : POSITION;
    float3 normal : NORMAL;
    float2 tex0 : TEXCOORD0;
    float4 color : COLOR0;
};

struct PS_Input
{
    float4 pos  : SV_POSITION;
    float3 tex : TEXCOORD;
};

PS_Input VS_Main( VS_Input vertex ) {
    PS_Input vsOut = ( PS_Input )0;
    vsOut.pos = mul( vertex.pos, mvp_ ).xyww;
    float4 VertexPosition = mul(vertex.pos, world);
    vsOut.tex = VertexPosition.xyz - camera;
    return vsOut;
}

float4 PS_Main( PS_Input frag ) : SV_TARGET {
// return texCUBE(cubeSampler,input.PosTex);
    return colorMap_.Sample( colorSampler_, frag.tex );    
}