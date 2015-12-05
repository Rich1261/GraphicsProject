#pragma pack_matrix(row_major)

texture2D baseTexture : register(t0);
SamplerState filters[1] : register(s0);

struct IN_PIXEL
{
	//float4 colorOut : COLOR;
	float4 projectedCoordinate : SV_POSITION;
	float2 projectedUv : TEXCOORD;
	float3 projectedNormal : NORMAL;
};
float4 main(IN_PIXEL input) : SV_TARGET
{
	float4 baseColor = baseTexture.Sample(filters[0], input.projectedUv);
	return baseColor;
}