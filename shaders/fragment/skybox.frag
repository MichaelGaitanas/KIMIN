#version 450 core

in vec3 uv;

out vec4 frag_col;

uniform samplerCube skybox_stars;
uniform samplerCube skybox_starmap;
uniform samplerCube skybox_galaxy;

uniform int render_stars;
uniform int render_starmap;
uniform int render_galaxy;

void main()
{
    vec3 col = vec3(0.0);

    //Now we add the individual skybox contributions to the final fragment color.
    if (render_stars == 1)
        col += texture(skybox_stars, uv).rgb;
    if (render_starmap == 1)
    {
        //For the constellations, we highlight blue a little bit. It makes it more appealing I think...
        vec3 temp = texture(skybox_starmap, uv).rgb;
        col += vec3(0.5*temp.r, 0.5*temp.g, 2.0*temp.b);
    }
    if (render_galaxy == 1)
        col += 0.5*texture(skybox_galaxy, uv).rgb; //Decrease overall gain of the galaxy a little bit.


    //Actually there are a lot of cheap image processing tricks one can do to make the skybox impressive. Maybe another time. Going to bed now...

    frag_col = vec4(col, 1.0);
}