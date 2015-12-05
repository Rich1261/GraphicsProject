#pragma pack_matrix(row_major)

textureCUBE baseTexture : register(t0);
SamplerState filters[1] : register(s0);

struct IN_PIXEL
{
	float4 projectedCoordinate : SV_POSITION;
	float4 projectedUv : TEXCOORD;
};
float4 main(IN_PIXEL input) : SV_TARGET
{
	float4 base = baseTexture.Sample(filters[0], input.projectedUv.xyz);
	return base;
}