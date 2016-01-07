return [[
        attribute vec3 aPosition;
        attribute vec2 aTexCoords;
        uniform mat4 uTransMat;
        uniform mat4 uProjMat;
        varying vec2 vTexCoords;
        void main() {
            vTexCoords  = aTexCoords;
            gl_Position = uProjMat * uTransMat * vec4(aPosition, 1.0);
        }
    ]], [[
        uniform sampler2D uTexture;
        uniform vec4 uColor;
        varying vec2 vTexCoords;
        void main() {
            gl_FragColor = texture2D(uTexture, vTexCoords) * uColor;
        }
    ]]