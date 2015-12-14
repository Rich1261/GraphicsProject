#pragma pack_matrix(row_major)

struct INPUT_VERTEX
{
	float4 pos : SV_POSITION;
	float4 opos : ORIGPOS;
	float speed : SPEED;
	float dist : DISTANCE;
};
struct GSOutput
{
	float4 pos : SV_POSITION;
	float2 uv : TEXCOORD;
	float3 norm : NORMAL;
};
cbuffer WORLD_MATRIX : register(b0){
	float4x4 worldMatrix;
	float down;
	float3 padding;
};
cbuffer PROJ_VIEW_MATRICIES : register(b1){
	float4x4 viewMatrix;
	float4x4 projMatrix;
};
cbuffer actualStats : register(b2)
{
	float4 Apos;
	float4 Aopos;
	float Aspeed;
	float Adist;
};

[maxvertexcount(4)]
void main(point INPUT_VERTEX input[1], inout TriangleStream< GSOutput > output)
{
	//input[0].pos.y += down;
	//input[0].pos.y -= input[0].speed;
	//if (input[0].pos.y < (input[0].opos.y - input[0].dist)) input[0].pos.y = input[0].opos.y;

	GSOutput verts[4];
	//float size = static_cast<float>(rand()) / (static_cast<float>(RAND_MAX));
	float size = .25;
	/*verts[0].pos = float4(input[0].pos.x - size, input[0].pos.y + size, input[0].pos.z, 1);
	verts[1].pos = float4(input[0].pos.x + size, input[0].pos.y + size, input[0].pos.z, 1);
	verts[2].pos = float4(input[0].pos.x - size, input[0].pos.y - size, input[0].pos.z, 1);
	verts[3].pos = float4(input[0].pos.x + size, input[0].pos.y - size, input[0].pos.z, 1);*/
	verts[0].pos = float4(Apos.x - size, Apos.y + size, Apos.z, 1);
	verts[1].pos = float4(Apos.x + size, Apos.y + size, Apos.z, 1);
	verts[2].pos = float4(Apos.x - size, Apos.y - size, Apos.z, 1);
	verts[3].pos = float4(Apos.x + size, Apos.y - size, Apos.z, 1);

	verts[0].uv = float2(0, 0);
	verts[1].uv = float2(1, 0);
	verts[2].uv = float2(0, 1);
	verts[3].uv = float2(1, 1);

	verts[0].norm = float3(0, 1, 0);
	verts[1].norm = float3(0, 1, 0);
	verts[2].norm = float3(0, 1, 0);
	verts[3].norm = float3(0, 1, 0);

	for (int i = 0; i < 4; i++){
		float4 localH = verts[i].pos;
		localH = mul(localH, worldMatrix);
		localH = mul(localH, viewMatrix);
		localH = mul(localH, projMatrix);
		verts[i].pos = localH;
		output.Append(verts[i]);
	}
	output.RestartStrip();
}