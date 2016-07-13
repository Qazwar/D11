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

[maxvertexcount(4)]
void GS_Main(point GSPS_INPUT gin[1], inout TriangleStream<PS_Input> triStream)
{    
    float VP_ARRAY[8] = { -1.0f, 1.0f, 1.0f, 1.0f, -1.0f, -1.0f, 1.0f, -1.0f };
    float2 t[4];
    float4 ret = gin[0].Tex;
    float left = ret.x / screenDimension.z;
    float top = ret.y / screenDimension.w;
    float right = left + ret.z / screenDimension.z;
    float bottom = top + ret.w / screenDimension.w;
    t[0] = float2(left,top);
    t[1] = float2(right,top);
    t[2] = float2(left,bottom);
    t[3] = float2(right,bottom);

    float dx = ret.z / screenDimension.x;
    float dy = ret.w / screenDimension.y;
    float4 pos = gin[0].Pos;
    pos.x = pos.x / screenDimension.x * 2.0 - 1.0;
    pos.y = pos.y / screenDimension.y * 2.0 - 1.0;
    PS_Input gout;
    triStream.RestartStrip();
    [unroll]
    for(int i = 0; i < 4; ++i) {
        float px = VP_ARRAY[i * 2] * dx;
        float py = VP_ARRAY[i * 2 + 1] * dy;
        float sx = px * gin[0].Size.x;
        float sy = py * gin[0].Size.y;
        float xt = cos(gin[0].Size.z) * sx - sin(gin[0].Size.z) * sy;
        float yt = sin(gin[0].Size.z) * sx + cos(gin[0].Size.z) * sy;
        gout.pos = float4(xt + pos.x, yt + pos.y, 0.5f, 1.0f); 
        gout.tex0 = t[i];
        gout.color = gin[0].Color;
        triStream.Append(gout);
    }
    triStream.RestartStrip();
}


float4 PS_Main( PS_Input frag ) : SV_TARGET {
    return colorMap_.Sample( colorSampler_, frag.tex0 ) * frag.color;    
}