#include "PostProcessCommon.inc"

PS_Input VS_Main(VS_Input vertex) {
	PS_Input vsOut = (PS_Input)0;
    vsOut.pos = float4(vertex.pos,1.0);
    vsOut.tex0 = vertex.tex;
    vsOut.color = vertex.color;
    return vsOut;
}

float4 PS_Main( PS_Input frag ) : SV_TARGET {
	return colorMap_.Sample(colorSampler_, frag.tex0) * frag.color;
}