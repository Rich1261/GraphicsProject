#pragma pack_matrix(row_major)

struct INPUT_VERTEX
{
	float3 coordinate : POSITION;
	float3 uv : UV;
	float3 norm : NORMAL;
	float4 tan : TANGENT;
};
struct OUTPUT_VERTEX
{
	float4 projectedCoordinate : SV_POSITION;
	float2 projectedUv : TEXCOORD;
	float3 projectedNormal : NORMAL;
	float4 worldPos : WORLDPOS;
	float3 Tangent : TANGENT;
	float3 BiTangent : BITAN;
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
	float4 lightpos = float4(0, -8, 0, 0);
	OUTPUT_VERTEX output = (OUTPUT_VERTEX)0;
	float4 localH = float4(input.coordinate, 1);
	localH = mul(localH, worldMatrix);
	output.worldPos = localH;
	localH = mul(localH, viewMatrix);
	localH = mul(localH, projMatrix);
	output.projectedCoordinate = localH;
	output.projectedNormal = mul(float4(input.norm,0), worldMatrix).xyz;
	output.Tangent = mul(float4(input.tan.xyz * input.tan.w, 0.0f), worldMatrix);
	output.BiTangent = mul(float4(cross(output.projectedNormal.xyz, output.Tangent.xyz), 0.0f), worldMatrix);
	output.projectedUv = float2(input.uv.xy);
	return output;
}