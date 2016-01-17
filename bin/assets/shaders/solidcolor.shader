return [[
        attribute vec3 aPosition;
        uniform mat4 uTransMat;
        uniform mat4 uProjMat;
        void main() {
            gl_Position = uProjMat * uTransMat * vec4(aPosition, 1.0);
        }
    ]], [[
        uniform vec4 uColor;
        void main() {
            gl_FragColor = uColor;
        }
    ]]