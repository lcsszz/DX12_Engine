#include "TransformBuffer.hlsli"

struct VS_INPUT
{
	float3 pos		: POSITION;
	float3 normal	: NORMAL;
	float2 uv		: TEXCOORD;
};

struct VS_OUTPUT
{
	float4 pos		: SV_POSITION;
	float3 normal	: NORMAL;
	float2 uv		: TEXCOORD;
};

VS_OUTPUT main(VS_INPUT input)
{
	VS_OUTPUT output;
	float4 pos = float4(input.pos, 1.f);
	float4 norm = mul(model, float4(input.normal, 1.f));

	// Transform the vertex position into projected space.
	pos = mul(pos, model);
	pos = mul(pos, view);
	pos = mul(pos, projection);

	// Return 
	output.pos = pos;
	output.uv = input.uv;
	output.normal = float3(normalize(norm.xyz));

	return output;
}