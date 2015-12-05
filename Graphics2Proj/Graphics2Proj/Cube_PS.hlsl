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
float4 main( IN_PIXEL input ) : SV_TARGET
{
	float4 lightDir = float4(-.5, -1, -.5, 0);
	float4 surfaceNormal = float4(input.projectedNormal.xyz, 0);
	float lightratio = clamp(dot(-lightDir, surfaceNormal), 0, 1);
	float4 lightcolor = float4(0, 1, 0, 1);

	float4 baseColor = baseTexture.Sample(filters[0], input.projectedUv);
	baseColor.argb = baseColor.bgra;
	float4 finalColor = baseColor;
		if (lightratio > 0){
			float4 result = lightratio * lightcolor * finalColor;
				return result;
		}
		else return finalColor;
}