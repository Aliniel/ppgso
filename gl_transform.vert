#version 150
// The inputs will be fed by the vertex buffer objects
in vec2 Position;
in vec2 TexCoord;

// This will be passed to the fragment shader
out vec2 fragTexCoord;

// Matrices as program attributes
uniform mat2 Transform;

void main() {
  // Copy the input to the fragment shader
  fragTexCoord = vec2(TexCoord);
  vec2 transformedPosition = Transform * Position;
  // Calculate the final position on screen
  gl_Position = vec4(transformedPosition, 0, 1.0);
}
