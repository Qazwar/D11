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


PS_Input VS_Main( VS_Input vertex )
{
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


float4 PS_Main( PS_Input frag ) : SV_TARGET
{
    float4 ambientColor = float4(0.2,0.2,0.2,1.0);
    float4 textureColor =  colorMap_.Sample( colorSampler_, frag.tex0 ) * frag.color;
    float4 dc = float4(1,1,1,1);    
    float4 color = float4(0,0,0,0);    
    float3 n = normalize(frag.normal);
    float3 ln = normalize(frag.lightVec);
    float lightIntensity = saturate(dot(n,ln));    
    if ( lightIntensity > 0.0f ) {
         color += (dc * lightIntensity);   
         color.a = frag.color.a;        
    }
    else {
        color = ambientColor;  
    }     
    color = color * textureColor;
    color = saturate(color);    
    return color;

}

float4 PS_Main_Specular( PS_Input frag ) : SV_TARGET
{
    float4 specular;
    float3 ambientColor = float3(0.3f,0.3f,0.3f);
    float4 textureColor = colorMap_.Sample( colorSampler_, frag.tex0 ) * frag.color;
    float4 diffuseColor = float4(1,1,1,1);    
    float3 lightVec = -frag.lightVec;//normalize(frag.lightVec);
    float3 normal = normalize(frag.normal);
    float lightIntensity = saturate(dot(normal,lightVec));
    float specularTerm = 0.0f;
    float4 color = float4(0,0,0,0);
    if ( lightIntensity > 0.0f ) {
         color += (diffuseColor * lightIntensity);   
         color = saturate(color);
         float3 reflection = normalize(2 * lightIntensity * frag.normal - lightVec); 
         specular = pow(saturate(dot(reflection, frag.viewVec)), 25.0);
    }
    color = color * textureColor;
    color = saturate(color + specular);
    return color;

}