#pragma pack_matrix(row_major)

struct INPUT_VERTEX
{
	float4 coordinate : POSITION; 
	float4 constantColor : COLOR;
};
struct OUTPUT_VERTEX
{
	float4 colorOut : COLOR;
	float4 projectedCoordinate : SV_POSITION;
};
//cbuffer THIS_IS_VRAM : register( b0 ){
//	//float4 constantPos;
//	float4 constantColor;
//	float2 constantOffset;
//	float2 padding;
//};
cbuffer OBJECT : register(b0){
	float4x4 worldMatrix;
}
cbuffer SCENE : register(b1){
	float4x4 viewMatrix;
	float4x4 projMatrix;
}
OUTPUT_VERTEX main( INPUT_VERTEX input )
{
	OUTPUT_VERTEX output = (OUTPUT_VERTEX)0;
	float4 localH = input.coordinate;
	localH = mul(localH, worldMatrix);
	localH = mul(localH, viewMatrix);
	localH = mul(localH, projMatrix);
	output.projectedCoordinate = localH;
	output.colorOut = input.constantColor;
	return output;
}