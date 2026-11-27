#version 330

// Interpolated inputs from dodge.vs
in vec2 fragTexCoord;
in vec4 fragColor;
in vec2 fragPosition;

// Raylib built-in texture sampler and tint diffuse uniform
uniform sampler2D texture0;
uniform vec4 colDiffuse;

// Gameplay uniforms uploaded by Level1::render() via ShaderProgram
uniform vec2  lightPosition;  // Current protagonist world position
uniform vec2  dodgeOrigin;    // World position where the dodge started
uniform vec2  dodgeDirection; // Normalized dash direction vector
uniform float dodgeProgress;  // 0.0 at dash start -> 1.0 at dash end
uniform int   isDodging;      // 1 while protagonist is in phantom-dodge i-frames
uniform int   isPlayer;       // 1 for protagonist draw pass, 0 for arena pass

out vec4 finalColor;

void main()
{
    vec4 texel = texture(texture0, fragTexCoord) * fragColor * colDiffuse;

    // Outside of a dodge, render the arena and protagonist normally
    if (isDodging == 0)
    {
        finalColor = texel;
        return;
    }

    // Fades from 1.0 at dash launch to 0.0 as invulnerability ends
    float fadeOut = clamp(1.0 - dodgeProgress, 0.0, 1.0);

    // Pass 1: Protagonist sprite — GPU afterimage smear + cyan phase-shift
    if (isPlayer == 1)
    {
        // All protagonist sheets have 8 frames across X (each frame is 1/8 = 0.125 wide).
        // Clamping ghost UV lookups to the current frame prevents sampling neighbor frames.
        float frameWidth = 0.125;
        float frameMinX  = floor(fragTexCoord.x / frameWidth) * frameWidth;
        float frameMaxX  = frameMinX + frameWidth - 0.001;

        vec2 ghostStep = vec2(
            -dodgeDirection.x * 0.010,
            -dodgeDirection.y * 0.080
        );

        vec2 uv1 = fragTexCoord + ghostStep * 1.0;
        vec2 uv2 = fragTexCoord + ghostStep * 2.0;
        vec2 uv3 = fragTexCoord + ghostStep * 3.0;

        uv1.x = clamp(uv1.x, frameMinX, frameMaxX);
        uv2.x = clamp(uv2.x, frameMinX, frameMaxX);
        uv3.x = clamp(uv3.x, frameMinX, frameMaxX);

        float trailAlpha = 0.0;
        if (uv1.y >= 0.0 && uv1.y <= 1.0)
            trailAlpha += texture(texture0, uv1).a * 0.55;
        if (uv2.y >= 0.0 && uv2.y <= 1.0)
            trailAlpha += texture(texture0, uv2).a * 0.35;
        if (uv3.y >= 0.0 && uv3.y <= 1.0)
            trailAlpha += texture(texture0, uv3).a * 0.20;

        // If this pixel is transparent behind the sprite, draw the trailing ghost streak
        if (texel.a < 0.1)
        {
            if (trailAlpha > 0.05)
            {
                finalColor = vec4(0.42, 0.92, 1.0, clamp(trailAlpha * fadeOut * 0.9, 0.0, 0.85));
                return;
            }
            discard;
        }

        // Solid protagonist pixels shift into a shimmering cyan-white phase form
        float scanline = mod(floor(fragPosition.y * 0.25), 2.0);
        vec3 phantomColor = vec3(0.55, 0.95, 1.0);
        if (scanline < 1.0)
            phantomColor = vec3(0.85, 1.0, 1.0);

        vec3 shifted = mix(texel.rgb, phantomColor, 0.55 + 0.30 * fadeOut);
        finalColor = vec4(shifted, texel.a * (0.78 + 0.22 * (1.0 - fadeOut)));
        return;
    }

    // Pass 2: Arena world — focus desaturation outside dash radius + shockwave ring
    float distToPlayer = distance(lightPosition, fragPosition);
    float distToOrigin = distance(dodgeOrigin, fragPosition);

    // Lecture 10 perceived luminance formula for monochrome draining
    float luminance = 0.2126 * texel.r + 0.7152 * texel.g + 0.0722 * texel.b;
    vec3 monochrome = vec3(luminance) * 0.78;

    vec3 worldColor = texel.rgb;

    // Drain color outside a 110 px focus bubble around the dashing protagonist
    if (distToPlayer > 110.0)
    {
        float edgeBlend = clamp((distToPlayer - 110.0) / 90.0, 0.0, 1.0);
        worldColor = mix(texel.rgb, monochrome, 0.68 * edgeBlend * fadeOut);
    }

    // Expanding cyan sonic shockwave ring radiating from where the dodge started
    float ringRadius = 24.0 + dodgeProgress * 220.0;
    float ringDiff   = abs(distToOrigin - ringRadius);
    if (ringDiff < 16.0)
    {
        float ringIntensity = (1.0 - ringDiff / 16.0) * 0.55 * fadeOut;
        worldColor = mix(worldColor, vec3(0.48, 0.94, 1.0), ringIntensity);
    }

    finalColor = vec4(worldColor, texel.a);
}
