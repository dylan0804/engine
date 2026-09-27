#version 330
uniform vec3 fillColor;
out vec4 FragColor;
void main()
{
     FragColor = vec4(fillColor, 1.0);
}
