#pragma pack_matrix(row_major)

texture2D baseTexture : register(t0);
SamplerState filters[1] : register(s0);

struct IN_PIXEL
{
	//float4 colorOut : COLOR;
	float4 projectedCoordinate : SV_POSITION;
	float2 projectedUv : TEXCOORD;
	float3 projectedNormal : NORMAL;
	float4 worldpos : WORLDPOS;
};

cbuffer SPOT_LIGHT : register(b0){
	float4 spotlightpos;
	float4 conedir;
};
cbuffer POINT_LIGHT : register(b1){
	float4 lightpos;
}
cbuffer DIRECTIONAL_LIGHT : register(b2){
	float4 lightDir;
}
float4 main(IN_PIXEL input) : SV_TARGET
{
	//point light
	float4 lightcolor = float4(0, 0, 1, 1);
	float4 lightdir = normalize(lightpos - input.worldpos);
	float lightratio = clamp(dot(lightdir, input.projectedNormal), 0, 1);

	float4 baseColor = baseTexture.Sample(filters[0], input.projectedUv);
		float attenuation = 1 - clamp(length(lightpos - input.worldpos) / 5, 0, 1);
	float4 pointlightres = lightratio * lightcolor * baseColor * attenuation;
		pointlightres.a = 1;

	//spotlight
	float4 spotlightcolor = float4(1, 0, 0, 1);
		float4 spotlightdir = normalize(spotlightpos - input.worldpos);
		float surfaceRatio = clamp(dot(-spotlightdir, conedir), 0, 1);
	float spotfactor = (surfaceRatio > .8) ? 1 : 0;
	float spotlightratio = clamp(dot(spotlightdir, input.projectedNormal), 0, 1);
	float spotlightatten = 1 - clamp((.9 - surfaceRatio) / (.9 - .8), 0, 1);
	float4 spotlightResult = spotfactor * spotlightratio * spotlightcolor * baseColor * spotlightatten;
		spotlightResult.a = 1;

	//directional light
	float3 DirlightDirNorm = normalize(lightDir.xyz);
		float4 DirsurfaceNormal = float4(input.projectedNormal.xyz, 0);
		float Dirlightratio = clamp(dot(-DirlightDirNorm, DirsurfaceNormal), 0, 1);
	float4 DirlightColor = float4(1, 1, 1, 1);
		float4 Dirres = Dirlightratio * DirlightColor * baseColor;

		float4 finalTotalColor = saturate(pointlightres + spotlightResult + Dirres);
		return finalTotalColor;
}