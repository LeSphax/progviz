#version 330 core
in vec3 debugColor;  // Add this line
out vec4 FragColor;

void main() {
    FragColor = vec4(debugColor, 1.0);  // Use debugColor as the fragment color
}
