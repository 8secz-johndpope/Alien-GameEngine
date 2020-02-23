#shader vertex
#version 330 core

layout(location = 0) in vec3 position;
layout(location = 1) in uint indices;
layout(location = 2) in vec3 uvs;
layout(location = 3) in vec3 normals;

uniform mat4 view; 
uniform mat4 model;
uniform mat4 projection;

out vec3 texcoords;

void main()
{
    texcoords = uvs;
    gl_Position = projection * view * model * vec4(position, 1.0f); 
};

#shader fragment
#version 330 core

in vec3 texcoords;

uniform sampler2d tex;

uniform vec4 custom_color = vec4(1.0, 1.0, 1.0, 1.0);

out vec4 color;

void main()
{
	//color = custom_color;
    color = texture(tex, texcoords);
}







