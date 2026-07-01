struct VSInput {
    float3 position : POSITION;
    float4 color : COLOR;
};

struct VSOutput {
    float4 position : SV_POSITION;
    float4 color : COLOR;
};

cbuffer TransformData : register(b0) {
    matrix mvp;
};

VSOutput VSMain(VSInput input) {
    VSOutput output;
    output.position = mul(float4(input.position, 1.0f), mvp);
    output.color = float4((input.position * 0.5f) + 0.5f, 1.0f);
    return output;
}

float4 PSMain(VSOutput input) : SV_TARGET {
    return input.color;
}