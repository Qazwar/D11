#include "Sprite.inc"

[maxvertexcount(4)]
void main(point GSPS_INPUT gin[1], inout TriangleStream<PS_Input> triStream)
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
