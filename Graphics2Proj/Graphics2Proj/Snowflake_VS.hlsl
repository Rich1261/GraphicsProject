#pragma pack_matrix(row_major)

struct INPUT_VERTEX
{
	float4 coordinate : POSITION;
	float4 opos : ORIGPOS;
	float speed : SPEED;
	float dist : DISTANCE;
};
struct OUTPUT_VERTEX
{
	float4 coord : SV_POSITION;
	float4 opos : ORIGPOS;
	float speed : SPEED;
	float dist : DISTANCE;
};

OUTPUT_VERTEX main(INPUT_VERTEX input)
{
	OUTPUT_VERTEX output = (OUTPUT_VERTEX)0;
	output.coord = float4(input.coordinate.xyz, 1);
	output.opos = input.opos;
	output.speed = input.speed;
	output.dist = input.dist;
	return output;
}