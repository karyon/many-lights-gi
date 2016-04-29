#version 330

in vec3 g_worldCoord;

uniform vec3 lightWorldPos;

layout (location = 0) out vec3 out_dist;

void main()
{
    float dist = length(g_worldCoord - lightWorldPos);
    float dx = dFdx(dist);
    float dy = dFdy(dist);

    float alpha = 1.0;
    out_dist = vec3(dist, dist * dist + 0.25 * (dx*dx + dy*dy), alpha);
}
