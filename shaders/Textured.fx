cbuffer cbChangesPerFrame : register( b0 )
{
    matrix mvp_;
    matrix world;
    float3 camera;
    float3 light;
};


Texture2D colorMap_ : register( t0 );
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
    float2 tex0 : TEXCOORD0;
    float4 color : COLOR0;
    float3 normal : NORMAL;
    float3 lightVec : TEXCOORD1;
    float3 viewVec : TEXCOORD2;
};


PS_Input VS_Main( VS_Input vertex ) {
    PS_Input vsOut = ( PS_Input )0;
    vsOut.pos = mul( vertex.pos, mvp_ );
    vsOut.tex0 = vertex.tex0;
    vsOut.color = vertex.color;
    vsOut.normal = mul(vertex.normal,(float3x3)world);
    vsOut.normal = normalize(vsOut.normal);
    float3 worldPosition = mul(vertex.pos,(float3x3)world);
    vsOut.lightVec = normalize(light);
    vsOut.viewVec = normalize(camera - worldPosition);
    return vsOut;
}


float4 PS_Main( PS_Input frag ) : SV_TARGET {
    return colorMap_.Sample( colorSampler_, frag.tex0 ) * frag.color;    
}