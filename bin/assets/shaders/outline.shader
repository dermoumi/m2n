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
    ]], [[
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

        float depth(in vec2 coord) {
            vec4 depth = texture2D(uDepthBuf0, coord);
            // return -near / ((depth.x - 1.0) * ((far - near) / far));
            return (2.0 * near) / (far + near - float(depth) * (far - near));
            // return (near * float(depth)) / (far - float(depth) * (far - near));
        }

        void main() {
            //assign these variables now because they will be used next
            vec3 sample[8];
            vec4 texcol = texture2D(uTexture0, vTexCoords);
            float pixZ = gl_FragCoord.z / gl_FragCoord.w;
            // vec2 uUnit = vec2(1.0, 1.0) / vec2(2048.0, 1024.0);

            //gets all neighboring fragments colors
            sample[0] = vec3(depth(vTexCoords - uUnit.x));
            sample[1] = vec3(depth(vTexCoords + vec2( 0.0,     -uUnit.y)));
            sample[2] = vec3(depth(vTexCoords + vec2( uUnit.x, -uUnit.y)));
            sample[3] = vec3(depth(vTexCoords + vec2(-uUnit.x,  0.0)));
            sample[4] = vec3(depth(vTexCoords + vec2( uUnit.x,  0.0)));
            sample[5] = vec3(depth(vTexCoords + vec2(-uUnit.x,  uUnit.y)));
            sample[6] = vec3(depth(vTexCoords + vec2( 0.0,      uUnit.y)));
            sample[7] = vec3(depth(vTexCoords + uUnit.x));
                
            //From all the neighbor fragments gets the one with the greatest and lowest colors
            //in a pair so a subtract can be made later. The check is huge, but GLSL built-in functions
            //are optimized for the GPU
            vec3 areaMx = max(sample[0], max(sample[1], max(sample[2], max(sample[3], max(sample[4], max(sample[5], max(sample[6], sample[7])))))));
            vec3 areaMn = min(sample[0], min(sample[1], min(sample[2], min(sample[3], min(sample[4], min(sample[5], min(sample[6], sample[7])))))));

            //The dot below is the same as a sum of the areaMx - areaMn result RGB components, but is more GPU efficient.
            //The result is the average difference amount of the RGB(note alpha was left alone) components group 
            //of the two select fragment samples above.
            float colDifForce = dot(areaMx - areaMn, vec3(1))*2.0;

            //Check for heavy RGB difference to darken the current fragment; 
            //we do not want to mess with transparency, so leave alpha alone
            //edgeForce can be changed below to make outline more transparent or opaque
            gl_FragColor = uColor * (colDifForce > edgeThresh ? vec4(vec3(texcol)*edgeForce, 1.0) : texcol);
            gl_FragDepth = texture2D(uDepthBuf0, vTexCoords).x;
            
            // float edge = clamp(colDifForce - edgeThresh, 0.0, 0.000001) * 1000000.0;
            // gl_FragColor = uColor * vec4(vec3(texcol)*(1.0-edge), texcol.a*edge);
        }
    ]]