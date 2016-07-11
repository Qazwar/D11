cbuffer cbChangesPerFrame : register( b0 )
{
    matrix mvp_;
    matrix world;
    float3 camera;
    float3 light;
    float4 diffuseColor;
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


PS_Input VS_Main( VS_Input vertex )
{
    PS_Input vsOut = ( PS_Input )0;
    vsOut.pos = mul( vertex.pos, mvp_ );
    vsOut.tex0 = vertex.tex0;
    vsOut.color = vertex.color;
    vsOut.normal = mul(vertex.normal,(float3x3)world);
    vsOut.normal = normalize(vsOut.normal);
    vsOut.lightVec = normalize(light);
    vsOut.viewVec = normalize(camera);
    return vsOut;
}

float4 PS_Main_Plain( PS_Input frag ) : SV_TARGET
{
    return frag.color;
}

float4 PS_Main( PS_Input frag ) : SV_TARGET {
    float4 tmp = float4(frag.normal,1);
    tmp.x = (tmp.x + 1) / 2;
    tmp.y = (tmp.y + 1) / 2;
    tmp.z = (tmp.z + 1) / 2;
    return tmp;
}
