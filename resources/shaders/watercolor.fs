#version 410 core

in vec2 fragTexCoord;
in vec4 fragColor;

uniform sampler2D texture0;
uniform vec4 colDiffuse;
uniform float time;
uniform vec2 resolution;
uniform float intensity;

out vec4 finalColor;

// Noise function for organic watercolor effects
float noise(vec2 uv) {
    return fract(sin(dot(uv, vec2(12.9898, 78.233))) * 43758.5453);
}

// Smooth noise for organic patterns
float smoothNoise(vec2 uv) {
    vec2 i = floor(uv);
    vec2 f = fract(uv);
    f = smoothstep(0.0, 1.0, f);

    float a = noise(i);
    float b = noise(i + vec2(1.0, 0.0));
    float c = noise(i + vec2(0.0, 1.0));
    float d = noise(i + vec2(1.0, 1.0));

    return mix(mix(a, b, f.x), mix(c, d, f.x), f.y);
}

// Fractal noise with fixed iterations for Metal compatibility
float fractalNoise(vec2 uv) {
    float value = 0.0;
    value += 0.5 * smoothNoise(uv);
    value += 0.25 * smoothNoise(uv * 2.0);
    value += 0.125 * smoothNoise(uv * 4.0);
    value += 0.0625 * smoothNoise(uv * 8.0);
    return value;
}

// Paper texture simulation
float paperTexture(vec2 uv) {
    vec2 paperUV = uv * 120.0;
    float paper = fractalNoise(paperUV) * 0.3;

    // Add paper grain patterns
    paper += sin(paperUV.x * 0.4) * sin(paperUV.y * 0.6) * 0.05;
    paper += cos(paperUV.x * 0.7 + paperUV.y * 0.3) * 0.03;

    return clamp(paper + 0.75, 0.0, 1.0);
}

// Color bleeding effect - simplified for Metal
vec3 colorBleeding(vec2 uv, vec3 originalColor) {
    vec3 bleedColor = vec3(0.0);
    float totalWeight = 0.0;

    // Sample in 8 directions around the current pixel
    for (int i = 0; i < 8; i++) {
        float angle = float(i) * 0.78539816; // 45 degrees in radians
        float radius = 0.003 * intensity;

        // Add time-based variation for organic movement
        float timeOffset = time * 0.1 + float(i) * 0.5;
        radius *= (1.0 + sin(timeOffset) * 0.3);

        vec2 offset = vec2(cos(angle), sin(angle)) * radius;

        // Add noise-based displacement
        vec2 noiseOffset = vec2(
            fractalNoise(uv * 15.0 + timeOffset),
            fractalNoise(uv * 15.0 + timeOffset + 100.0)
        ) * 0.002 * intensity;

        vec2 sampleUV = uv + offset + noiseOffset;

        if (sampleUV.x >= 0.0 && sampleUV.x <= 1.0 &&
            sampleUV.y >= 0.0 && sampleUV.y <= 1.0) {

            vec3 sampleColor = texture(texture0, sampleUV).rgb;
            float weight = 1.0 / (1.0 + length(offset) * 200.0);

            bleedColor += sampleColor * weight;
            totalWeight += weight;
        }
    }

    if (totalWeight > 0.0) {
        bleedColor /= totalWeight;
        return mix(originalColor, bleedColor, intensity * 0.25);
    }

    return originalColor;
}

// Edge softening with organic variation
float edgeSoftening(vec2 uv, float alpha) {
    if (alpha < 0.01) return alpha;

    // Create soft, irregular edges
    vec2 edgeUV = uv * 25.0 + time * 0.03;
    float edgeNoise = fractalNoise(edgeUV) * 0.5 + 0.5;

    // More aggressive softening at lower alpha values
    float softening = (1.0 - alpha) * intensity * 0.4;
    float edgeFactor = mix(0.6, 1.0, edgeNoise);

    return mix(alpha * edgeFactor, alpha, 1.0 - softening);
}

// Color adjustment for authentic watercolor appearance
vec3 watercolorColorAdjust(vec3 color) {
    // Desaturate slightly for watercolor look
    float luminance = dot(color, vec3(0.299, 0.587, 0.114));
    color = mix(vec3(luminance), color, 0.82);

    // Adjust contrast curve
    color = pow(color, vec3(0.95));

    // Add slight warm tint typical of watercolor
    color.r *= 1.03;
    color.g *= 1.01;
    color.b *= 0.98;

    // Subtle color variation
    vec2 colorNoiseUV = fragTexCoord * 12.0 + time * 0.02;
    float colorNoise = fractalNoise(colorNoiseUV) * 0.08 - 0.04;
    color += colorNoise * intensity * 0.5;

    return color;
}

// Watercolor pigment settling effect
vec3 pigmentSettling(vec2 uv, vec3 color, float alpha) {
    if (alpha < 0.1) return color;

    // Create areas where pigment "settles" darker
    vec2 settlementUV = uv * 8.0 + time * 0.01;
    float settlement = fractalNoise(settlementUV);

    // Darken edges and areas of high pigment concentration
    float darkenFactor = smoothstep(0.6, 0.9, settlement) * intensity * 0.15;
    color *= (1.0 - darkenFactor);

    return color;
}

void main()
{
    vec2 uv = fragTexCoord;

    // Get original texture color
    vec4 texColor = texture(texture0, uv);
    vec3 color = texColor.rgb;
    float alpha = texColor.a;

    // Only apply watercolor effects where there's content
    if (alpha > 0.01 && length(color) > 0.01) {

        // Apply color bleeding first
        color = colorBleeding(uv, color);

        // Adjust colors for watercolor appearance
        color = watercolorColorAdjust(color);

        // Apply pigment settling effects
        color = pigmentSettling(uv, color, alpha);

        // Apply paper texture
        float paper = paperTexture(uv);
        color = mix(color * 0.9, color * paper, intensity);

        // Soften edges organically
        alpha = edgeSoftening(uv, alpha);

        // Add subtle wet-on-wet bleeding at edges
        vec2 wetBleedUV = uv * 20.0 + time * 0.05;
        float wetBleed = fractalNoise(wetBleedUV) * 0.1;
        if (alpha > 0.5 && alpha < 0.9) {
            color += wetBleed * intensity * 0.3 * (vec3(0.1, 0.05, 0.15));
        }
    }

    // Ensure values stay in valid range
    color = clamp(color, 0.0, 1.0);
    alpha = clamp(alpha, 0.0, 1.0);

    finalColor = vec4(color, alpha) * colDiffuse * fragColor;
}