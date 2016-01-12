return [[
        attribute vec2 aPosition;
        attribute vec2 aTexCoords;
        uniform mat4 uTransMat;
        uniform mat4 uProjMat;
        varying vec2 vTexCoords;
        void main() {
            vTexCoords  = aTexCoords;
            gl_Position = uProjMat * uTransMat * vec4(aPosition, 0.0, 1.0);
        }
    ]], [[
        uniform sampler2D uTexture0;
        uniform vec2 uTexSize;
        uniform vec4 uColor;
        varying vec2 vTexCoords;

        const float near = 0.100;
        const float far  = 100.0;

        float depth(in vec2 coord) {
            vec4 depth = texture2D(uTexture0, vTexCoords / uTexSize);
            // return (near * float(depth)) / (far - float(depth) * (far - near));
            return (2.0 * near) / (far + near - float(depth) * (far - near));
        }

        void main() {
            float d = 1.0 - depth(vTexCoords);
            gl_FragColor = vec4(d, d, d, 1.0) * uColor;
        }
    ]]