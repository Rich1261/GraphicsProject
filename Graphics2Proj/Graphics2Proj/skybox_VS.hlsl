#pragma pack_matrix(row_major)

struct INPUT_VERTEX
{
	float4 coordinate : POSITION;
};
struct OUTPUT_VERTEX
{
	float4 projectedCoordinate : SV_POSITION;
	float4 projectedUv : TEXCOORD;
};
cbuffer OBJECT : register(b0){
	float4x4 worldMatrix;
}
cbuffer SCENE : register(b1){
	float4x4 viewMatrix;
	float4x4 projMatrix;
}
OUTPUT_VERTEX main(INPUT_VERTEX input)
{
	OUTPUT_VERTEX output = (OUTPUT_VERTEX)0;
	output.projectedUv = input.coordinate;
	float4 localH = input.coordinate;
	localH = mul(localH, worldMatrix);
	localH = mul(localH, viewMatrix);
	localH = mul(localH, projMatrix);
	output.projectedCoordinate = localH;
	return output;
}