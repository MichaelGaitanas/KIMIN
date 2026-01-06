#version 450 core

void main()
{
    //Nothing to explicit do here. We DO need this seamingly empty shader however, because OpenGL is gonna automatically write
    //the depth of the current fragment to the depth attachment of the currently bound framebuffer (glBindFramebuffer(GL_FRAMEBUFFER, depth_fbo_id);).
    //It is the same as writing explicitely :
    //gl_FragDepth = gl_FragCoord.z;
}