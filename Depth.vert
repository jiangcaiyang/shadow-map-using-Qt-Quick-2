// Depth.vert
#ifdef GL_ES
precision highp float;
#endif

attribute vec3 position;

uniform mat4 modelMatrix;
uniform mat4 viewProjectionMatrix;

varying vec4 projectedPosition;

void main( void )
{
    vec3 finalPosition = position;

    projectedPosition =
            viewProjectionMatrix *
            modelMatrix *
            vec4( finalPosition, 1.0 );
    gl_Position = projectedPosition;
}
