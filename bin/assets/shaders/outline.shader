local gles2 = require('system').platform('android', 'ios')

return [[
        attribute vec2 aPosition;
        attribute vec2 aTexCoords;
        uniform mat4 uTransMat;
        uniform mat4 uProjMat;
        uniform vec2 uTexSize;
        varying vec2 vTexCoords;
        void main() {
            vTexCoords  = aTexCoords / uTexSize;
            gl_Position = uProjMat * uTransMat * vec4(aPosition, 0.0, 1.0);
        }
    ]], ([[
        // Based on Toon Lines shader by Jose I. Romero (cyborg_ar)
        // http://blenderartists.org/forum/showthread.php?172282-cartoon-in-GE/page2
        uniform sampler2D uTexture0;
        uniform sampler2D uDepthBuf0;
        uniform vec4 uColor;
        uniform vec2 uUnit;
        varying vec2 vTexCoords;

        const float edgeThresh = 0.03;
        const float edgeForce  = 0.275;

        const float near = 0.100;
        const float far  = 100.0;

        float norm(in float depth) {
            return (2.0 * near) / (far + near - depth * (far - near));
        }

        void main() {
            float pixZ = gl_FragCoord.z / gl_FragCoord.w;

            // gets all neighboring fragments colors:
            // [0] [1] [2]
            // [3] [X] [4]
            // [5] [6] [7]
            float sample[8];
            sample[0] = texture2D(uDepthBuf0, vTexCoords - uUnit.x                 ).x;
            sample[1] = texture2D(uDepthBuf0, vTexCoords + vec2( 0.0,     -uUnit.y)).x;
            sample[2] = texture2D(uDepthBuf0, vTexCoords + vec2( uUnit.x, -uUnit.y)).x;
            sample[3] = texture2D(uDepthBuf0, vTexCoords + vec2(-uUnit.x,  0.0)    ).x;
            sample[4] = texture2D(uDepthBuf0, vTexCoords + vec2( uUnit.x,  0.0)    ).x;
            sample[5] = texture2D(uDepthBuf0, vTexCoords + vec2(-uUnit.x,  uUnit.y)).x;
            sample[6] = texture2D(uDepthBuf0, vTexCoords + vec2( 0.0,      uUnit.y)).x;
            sample[7] = texture2D(uDepthBuf0, vTexCoords + uUnit.x                 ).x;

            float depth = texture2D(uDepthBuf0, vTexCoords).x;
                
            // From all the neighbor fragments gets the one with the greatest and lowest colors
            // in a pair so a subtract can be made later. The check is huge, but GLSL built-in functions
            // are optimized for the GPU
            float areaMx = max(sample[0], max(sample[1], max(sample[2], max(sample[3], max(sample[4], max(sample[5], max(sample[6], sample[7])))))));
            float areaMn = min(sample[0], min(sample[1], min(sample[2], min(sample[3], min(sample[4], min(sample[5], min(sample[6], sample[7])))))));

            // The dot below is the same as a sum of the areaMx - areaMn result RGB components, but is more GPU efficient.
            // The result is the average difference amount of the RGB(note alpha was left alone) components group 
            // of the two select fragment samples above.
            float colDifForce = dot(norm(depth) - norm(areaMn), 1.0)*2.0;

            // Check for heavy RGB difference to darken the current fragment; 
            // we do not want to mess with transparency, so leave alpha alone
            // edgeForce can be changed below to make outline more transparent or opaque
            vec4 texcol = texture2D(uTexture0, vTexCoords);
            if (colDifForce > edgeThresh) {
                gl_FragColor = uColor * vec4(texcol.xyz*edgeForce, 1.0);
                %s
            }
            else {
                gl_FragColor = uColor * texcol;
                %s
            }
        }
    ]]):format(gles2 and '' or 'gl_FragDepth = areaMn;', gles2 and '' or 'gl_FragDepth = texture2D(uDepthBuf0, vTexCoords).x;')