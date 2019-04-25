#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <algorithm>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/norm.hpp>

#include <common/shader.hpp>
#include <common/texture.hpp>
#include <common/controls.hpp>

GLFWwindow* window;
// CPU representation of a particle
struct Particle {
    glm::vec3 pos, speed;
    unsigned char r,g,b,a; // Color
    float size;
    float cameradistance; // *Squared* distance to the camera. if dead : -1.0f
    float mass;
    float acc;
    Particle():r(0),g(0),b(0),a(255),size(0.5),cameradistance(-1),mass(1) {
    }
};
const float Hook = 5;
const float Damp = 0.5;
const float restLen = 0.1;
const glm::vec3 GRAVITY = glm::vec3(0,-9.8,0);
struct Cloth {
    enum {
        RowNum = 2,
        ColNum = 1,

        InitIntervel = 5,
    };
    Particle mPoints[RowNum][ColNum];
    void initPoints() {
        for(int i = 0; i< RowNum; i++) {
            for(int j = 0; j < ColNum; j++) {
                mPoints[i][j].pos  = glm::vec3(0,-i * InitIntervel,-20.0);
            }
        }
    }
    void update(float dt) {
        for(int i = 0; i < RowNum; i++) {
            for(int j = 0; j < ColNum; j++) {
                glm::vec3 force = GRAVITY;
                if(i != 0) {
                    glm::vec3 spring = mPoints[i][j].pos - mPoints[i-1][j].pos;
					float springLength = length(spring);
					spring *= 1/springLength;
					float v1 = glm::dot(spring,mPoints[i][j].speed);
					float v2 = glm::dot(spring,mPoints[i-1][j].speed);
					force += spring *(Damp*(v2-v1)-Hook*(springLength- restLen));
                }
                if(i != RowNum-1) {
                    glm::vec3 spring = mPoints[i][j].pos - mPoints[i+1][j].pos;
                    force += spring*(-Hook*(1- restLen/length(spring)));
                }

                if(i!=0) {
                    glm::vec3 acc = force * (1/mPoints[i][j].mass);
                    mPoints[i][j].speed += acc * dt;
//					printf("%f\n",mPoints[i][j].speed.z);
                    mPoints[i][j].pos += mPoints[i][j].speed * dt;
                }
            }
        }
        //TODO: collision
    }
    void fixFirstLine() {
        for(int i = 0; i < ColNum; i++) {
            mPoints[0][i].pos = glm::vec3(0,0,0);
        }
    }
};

const int MaxParticles = Cloth::RowNum * Cloth::ColNum;



int main( void ) {
    // Initialise GLFW
    if( !glfwInit() ) {
        fprintf( stderr, "Failed to initialize GLFW\n" );
        getchar();
        return -1;
    }

    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_RESIZABLE,GL_FALSE);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy; should not be needed
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Open a window and create its OpenGL context
    window = glfwCreateWindow( 1024, 768, "Tutorial 18 - Particules", NULL, NULL);
    if( window == NULL ) {
        fprintf( stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n" );
        getchar();
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    // Initialize GLEW
    glewExperimental = true; // Needed for core profile
    if (glewInit() != GLEW_OK) {
        fprintf(stderr, "Failed to initialize GLEW\n");
        getchar();
        glfwTerminate();
        return -1;
    }

    // Ensure we can capture the escape key being pressed below
    glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
    // Hide the mouse and enable unlimited mouvement
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // Set the mouse at the center of the screen
    glfwPollEvents();
    glfwSetCursorPos(window, 1024/2, 768/2);

    // Dark blue background
    glClearColor(1.0f, 1.0f, 1.0f, 0.0f);

    // Enable depth test
    glEnable(GL_DEPTH_TEST);
    // Accept fragment if it closer to the camera than the former one
    glDepthFunc(GL_LESS);

    GLuint VertexArrayID;
    glGenVertexArrays(1, &VertexArrayID);
    glBindVertexArray(VertexArrayID);


    // Create and compile our GLSL program from the shaders
    GLuint programID = LoadShaders( "Particle.vertexshader", "Particle.fragmentshader" );

    // Vertex shader
    GLuint CameraRight_worldspace_ID  = glGetUniformLocation(programID, "CameraRight_worldspace");
    GLuint CameraUp_worldspace_ID  = glGetUniformLocation(programID, "CameraUp_worldspace");
    GLuint ViewProjMatrixID = glGetUniformLocation(programID, "VP");

    // fragment shader
    GLuint TextureID  = glGetUniformLocation(programID, "myTextureSampler");


    static GLfloat* g_particule_position_size_data = new GLfloat[MaxParticles * 4];
    static GLubyte* g_particule_color_data         = new GLubyte[MaxParticles * 4];

    GLuint Texture = loadDDS("particle.DDS");

    // The VBO containing the 4 vertices of the particles.
    // Thanks to instancing, they will be shared by all particles.
    static const GLfloat g_vertex_buffer_data[] = {
        -0.5f, -0.5f, 0.0f,
            0.5f, -0.5f, 0.0f,
            -0.5f,  0.5f, 0.0f,
            0.5f,  0.5f, 0.0f,
        };
    GLuint billboard_vertex_buffer;
    glGenBuffers(1, &billboard_vertex_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, billboard_vertex_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_STATIC_DRAW);

    // The VBO containing the positions and sizes of the particles
    GLuint particles_position_buffer;
    glGenBuffers(1, &particles_position_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, particles_position_buffer);
    // Initialize with empty (NULL) buffer : it will be updated later, each frame.
    glBufferData(GL_ARRAY_BUFFER, MaxParticles * 4 * sizeof(GLfloat), NULL, GL_STREAM_DRAW);

    // The VBO containing the colors of the particles
    GLuint particles_color_buffer;
    glGenBuffers(1, &particles_color_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, particles_color_buffer);
    // Initialize with empty (NULL) buffer : it will be updated later, each frame.
    glBufferData(GL_ARRAY_BUFFER, MaxParticles * 4 * sizeof(GLubyte), NULL, GL_STREAM_DRAW);

    Cloth cloth;
    cloth.initPoints();

    double lastTime = glfwGetTime();
    do {
        // Clear the screen
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        double currentTime = glfwGetTime();
        double delta = currentTime - lastTime;
        cloth.update(delta);
        //cloth.fixFirstLine();
        lastTime = currentTime;

        computeMatricesFromInputs();
        glm::mat4 ProjectionMatrix = getProjectionMatrix();
        glm::mat4 ViewMatrix = getViewMatrix();

        // We will need the camera's position in order to sort the particles
        // w.r.t the camera's distance.
        // There should be a getCameraPosition() function in common/controls.cpp,
        // but this works too.
        glm::vec3 CameraPosition(glm::inverse(ViewMatrix)[3]);
        glm::mat4 ViewProjectionMatrix = ProjectionMatrix * ViewMatrix;

        // Simulate all particles
        for(int i = 0; i< Cloth::RowNum; i++) {
            for(int j = 0; j < Cloth::ColNum; j++) {
                Particle& p = cloth.mPoints[i][j]; // shortcut
                //TODO: simulate more
                //p.speed += glm::vec3(0.0f,-9.81f, 0.0f) * (float)delta * 0.5f;
                //p.pos += p.speed * (float)delta;
                p.cameradistance = glm::length2( p.pos - CameraPosition );
                g_particule_position_size_data[4*(i*Cloth::ColNum+j)+0] = p.pos.x;
                g_particule_position_size_data[4*(i*Cloth::ColNum+j)+1] = p.pos.y;
                g_particule_position_size_data[4*(i*Cloth::ColNum+j)+2] = p.pos.z;
                g_particule_position_size_data[4*(i*Cloth::ColNum+j)+3] = p.size;
                g_particule_color_data[4*(i*Cloth::ColNum+j)+0] = p.r;
                g_particule_color_data[4*(i*Cloth::ColNum+j)+1] = p.g;
                g_particule_color_data[4*(i*Cloth::ColNum+j)+2] = p.b;
                g_particule_color_data[4*(i*Cloth::ColNum+j)+3] = p.a;
            }
        }

        //printf("%d ",ParticlesCount);

        // Update the buffers that OpenGL uses for rendering.
        // There are much more sophisticated means to stream data from the CPU to the GPU,
        // but this is outside the scope of this tutorial.
        // http://www.opengl.org/wiki/Buffer_Object_Streaming

        glBindBuffer(GL_ARRAY_BUFFER, particles_position_buffer);
        glBufferData(GL_ARRAY_BUFFER, MaxParticles * 4 * sizeof(GLfloat), NULL, GL_STREAM_DRAW); // Buffer orphaning, a common way to improve streaming perf. See above link for details.
        glBufferSubData(GL_ARRAY_BUFFER, 0, MaxParticles* sizeof(GLfloat) * 4, g_particule_position_size_data);

        glBindBuffer(GL_ARRAY_BUFFER, particles_color_buffer);
        glBufferData(GL_ARRAY_BUFFER, MaxParticles * 4 * sizeof(GLubyte), NULL, GL_STREAM_DRAW); // Buffer orphaning, a common way to improve streaming perf. See above link for details.
        glBufferSubData(GL_ARRAY_BUFFER, 0, MaxParticles* sizeof(GLubyte) * 4, g_particule_color_data);


        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        // Use our shader
        glUseProgram(programID);

        // Bind our texture in Texture Unit 0
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, Texture);
        // Set our "myTextureSampler" sampler to use Texture Unit 0
        glUniform1i(TextureID, 0);

        // Same as the billboards tutorial
        glUniform3f(CameraRight_worldspace_ID, ViewMatrix[0][0], ViewMatrix[1][0], ViewMatrix[2][0]);
        glUniform3f(CameraUp_worldspace_ID, ViewMatrix[0][1], ViewMatrix[1][1], ViewMatrix[2][1]);

        glUniformMatrix4fv(ViewProjMatrixID, 1, GL_FALSE, &ViewProjectionMatrix[0][0]);

        // 1rst attribute buffer : vertices
        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, billboard_vertex_buffer);
        glVertexAttribPointer(
            0,                  // attribute. No particular reason for 0, but must match the layout in the shader.
            3,                  // size
            GL_FLOAT,           // type
            GL_FALSE,           // normalized?
            0,                  // stride
            (void*)0            // array buffer offset
        );

        // 2nd attribute buffer : positions of particles' centers
        glEnableVertexAttribArray(1);
        glBindBuffer(GL_ARRAY_BUFFER, particles_position_buffer);
        glVertexAttribPointer(
            1,                                // attribute. No particular reason for 1, but must match the layout in the shader.
            4,                                // size : x + y + z + size => 4
            GL_FLOAT,                         // type
            GL_FALSE,                         // normalized?
            0,                                // stride
            (void*)0                          // array buffer offset
        );

        // 3rd attribute buffer : particles' colors
        glEnableVertexAttribArray(2);
        glBindBuffer(GL_ARRAY_BUFFER, particles_color_buffer);
        glVertexAttribPointer(
            2,                                // attribute. No particular reason for 1, but must match the layout in the shader.
            4,                                // size : r + g + b + a => 4
            GL_UNSIGNED_BYTE,                 // type
            GL_TRUE,                          // normalized?    *** YES, this means that the unsigned char[4] will be accessible with a vec4 (floats) in the shader ***
            0,                                // stride
            (void*)0                          // array buffer offset
        );

        // These functions are specific to glDrawArrays*Instanced*.
        // The first parameter is the attribute buffer we're talking about.
        // The second parameter is the "rate at which generic vertex attributes advance when rendering multiple instances"
        // http://www.opengl.org/sdk/docs/man/xhtml/glVertexAttribDivisor.xml
        glVertexAttribDivisor(0, 0); // particles vertices : always reuse the same 4 vertices -> 0
        glVertexAttribDivisor(1, 1); // positions : one per quad (its center)                 -> 1
        glVertexAttribDivisor(2, 1); // color : one per quad                                  -> 1

        // Draw the particules !
        // This draws many times a small triangle_strip (which looks like a quad).
        // This is equivalent to :
        // for(i in ParticlesCount) : glDrawArrays(GL_TRIANGLE_STRIP, 0, 4),
        // but faster.
        glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, MaxParticles);

        glDisableVertexAttribArray(0);
        glDisableVertexAttribArray(1);
        glDisableVertexAttribArray(2);

        // Swap buffers
        glfwSwapBuffers(window);
        glfwPollEvents();

    } // Check if the ESC key was pressed or the window was closed
    while( glfwGetKey(window, GLFW_KEY_ESCAPE ) != GLFW_PRESS &&
            glfwWindowShouldClose(window) == 0 );

    delete[] g_particule_position_size_data;

    // Cleanup VBO and shader
    glDeleteBuffers(1, &particles_color_buffer);
    glDeleteBuffers(1, &particles_position_buffer);
    glDeleteBuffers(1, &billboard_vertex_buffer);
    glDeleteProgram(programID);
    glDeleteTextures(1, &Texture);
    glDeleteVertexArrays(1, &VertexArrayID);

    // Close OpenGL window and terminate GLFW
    glfwTerminate();

    return 0;
}
