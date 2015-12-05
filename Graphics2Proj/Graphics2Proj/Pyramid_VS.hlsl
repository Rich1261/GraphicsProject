#pragma pack_matrix(row_major)

struct INPUT_VERTEX
{
	float3 coordinate : POSITION;
	float3 uv : UV;
	float3 norm : NORMAL;
};
struct OUTPUT_VERTEX
{
	float4 projectedCoordinate : SV_POSITION;
	float2 projectedUv : TEXCOORD;
	float3 projectedNormal : NORMAL;
};
cbuffer OBJECT : register(b0){
	float4x4 worldMatrix;
	float faceNum;
	float3 padding;
}
cbuffer SCENE : register(b1){
	float4x4 viewMatrix;
	float4x4 projMatrix;
}
OUTPUT_VERTEX main(INPUT_VERTEX input)
{
	OUTPUT_VERTEX output = (OUTPUT_VERTEX)0;
	float4 localH = float4(input.coordinate, 1);
	localH = mul(localH, worldMatrix);
	localH = mul(localH, viewMatrix);
	localH = mul(localH, projMatrix);
	output.projectedCoordinate = localH;
	output.projectedNormal = input.norm;
	output.projectedUv = float2(input.uv.xy);
	return output;
}