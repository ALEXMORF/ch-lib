#define RS "RootFlags( ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT)"

struct vs_in
{
    float3 P: POSITION;
    float3 Col: COLOR;
};

struct vs_out
{
    float4 SV_P: SV_Position;
    float3 Col: COL;
};

[RootSignature(RS)]
vs_out VS(vs_in In)
{
    vs_out Out;
    
    Out.SV_P = float4(In.P, 1.0);
    Out.Col = In.Col;
    
    return Out;
}

float4 PS(vs_out In): SV_Target
{
    return float4(sqrt(In.Col), 1.0);
}