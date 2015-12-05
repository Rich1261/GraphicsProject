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
	switch (faceNum){
	case 1:
		output.projectedUv = float2(input.uv.r * .25f, input.uv.g);
		break;
	case 2:
		output.projectedUv = float2((input.uv.r * .25f) + .25f, input.uv.g);
		break;
	case 3:
		output.projectedUv = float2((input.uv.r * .25f) + .5f, input.uv.g);
		break;
	case 4:
		output.projectedUv = float2((input.uv.r * .25f) + .75f, input.uv.g);
		break;
	}
	output.projectedNormal = input.norm;
	return output;
}