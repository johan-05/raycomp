#include "include/glad/glad.h"
#include <fenv.h>
#include <stdlib.h>
#include <stdio.h>
#include <GLFW/glfw3.h>
#include <cglm/affine-pre.h>
#include <cglm/vec3.h>
#include <cglm/vec4.h>
#include <string.h>
#include <strings.h>
#include<unistd.h>

#define GLFW_INCLUDE_NONE
#define CGLM_DEFINE_PRINTS
#define STB_IMAGE_IMPLEMENTATION

#include <cglm/mat4.h>
#include <cglm/types.h>

#define AIR 0
#define WATER 1
#define GLASS 2
#define MIRROR 3
#define UNCREATED 4

#define SPEED_IN_AIR 1
#define SPEED_IN_WATER 0.66
#define SPEED_IN_GLASS 0.66


const char *vertexShaderSource = "#version 330 core\n"
    "layout (location = 0) in vec3 aPos;\n"
    "uniform vec2 size;\n"
    "uniform mat4 rotation;\n"
    "uniform mat4 model;\n"
    "void main()\n"
    "{\n"
    "   vec4 test = model*rotation*vec4(aPos.x*size.x, aPos.y*size.y, aPos.z, 1.0);\n"
    "   gl_Position = vec4(test.x/1632, test.y/918, 0.0, 1.0);\n"
    "}\0";
const char *fragmentShaderSource = "#version 330 core\n"
    "out vec4 FragColor;\n"
    "uniform int material;\n"
    "void main()\n"
    "{\n"
    "   if (material == 0){\n"
    "       FragColor = vec4(0.3f, 0.3f, 0.3f, 1.0f);\n"
    "   } else if(material == 1){\n"
    "       FragColor = vec4(0.15f, 0.2f, 1.0f, 1.0f);\n"
    "   } else if(material == 2){\n"
    "       FragColor = vec4(0.67f, 0.84f, 0.9f, 1.0f);\n"
    "   } else if (material == 3){\n"
    "       FragColor = vec4(0.85f, 0.85f, 0.85f, 1.0f);\n"
    "   } else{\n"
    "       FragColor = vec4(0.7f, 0.2f, 0.4f, 1.0f);\n"
    "   }\n"
    "}\n\0";

const char *laserVertexShaderSource = "#version 330 core\n"
    "layout (location = 0) in float check;\n"
    "uniform vec4 rPos;\n"
    "void main()\n"
    "{\n"
    "   if (check == 0)\n"
    "   {\n"
    "       gl_Position = vec4(rPos.x/1632, rPos.y/918, 0.0, 1.0f);\n"
    "   }else{\n"
    "       gl_Position = vec4(rPos.z/1632, rPos.w/918, 0.0, 1.0f);\n"
    "   }\n"
    "}\n\0";

const char *laserFragmentShaderSource = "#version 330 core\n"
    "out vec4 FragColor;\n"
    "void main()\n"
    "{\n"
    "   FragColor = vec4(1.0f, 0.1f, 0.2f, 1.0f);\n"
    "}\n\0";

typedef struct{
    vec3 pos;
    float width;
    float height;
    float rot;
    int material;
}rectangle;

typedef struct{
    rectangle* rect_ptr;
    void* next_rect;
}rectangle_link;

typedef struct{
    vec2 link_point;
    float freq;
}laser;

typedef struct{
    laser* laser_ptr;
    void* next_line;
}laser_link;


rectangle_link* rect_link_head = NULL;
laser_link* laser_link_head = NULL;

rectangle create_sudo_element(int posX, int posY);

void push_rectangle(rectangle_link** head_ref, rectangle* rect_ptr)
{
    rectangle_link* new_link = (rectangle_link*)malloc(sizeof(rectangle_link));
 
    new_link->rect_ptr = rect_ptr;
    new_link->next_rect = (struct rectangle_link*)(*head_ref);
    (*head_ref) = new_link;
}

void push_laser(rectangle_link** head_ref, laser* laser_ptr)
{
    laser_link* new_link = (laser_link*)malloc(sizeof(laser_link));

    new_link->laser_ptr = laser_ptr;
    new_link->next_line = (struct laser_link*)(*head_ref);
}

int WINDOW_HEIGHT = 918;
int WINDOW_WIDTH = 1632;

int posX = 0;
int posY = 0;

int sudo_item_active = 0;

vec3 Z_AXIS = {0.0, 0.0, 1.0};

rectangle sudo_element;

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}

void cursor_pos_callback(GLFWwindow* window, double xpos, double ypos)
{
    posX = xpos;
    posY = ypos;
    if (sudo_item_active)
    {
        int width = 2*((2*posX-WINDOW_WIDTH)-sudo_element.pos[0]);
        int height = 2*((-(2*posY-WINDOW_HEIGHT))-sudo_element.pos[1]);
        sudo_element.width = width;
        sudo_element.height = height;
    }

        vec3 new_pos = {
        2*sudo_element.pos[0]-WINDOW_WIDTH,
        -(2*sudo_element.pos[1]-918),
        0.0,
    };
}

void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, 1);
    } 
}

void mouse_button_callback(GLFWwindow *window, int button, int action, int mod)
{
    if (action == 0)
    {
        sudo_item_active = 0;

        rectangle* sudo_element_copy = malloc(sizeof(rectangle));
        memcpy(sudo_element_copy, &sudo_element, sizeof(rectangle));

        int buffer;
        printf("select rotation in degrees\n");
        scanf("%4d", &buffer);
        float rotation = -buffer*6.26/360;
        sudo_element_copy->rot = rotation;

        int material = 4;
        while (material > 3 || material < 1)
        {
            printf("select material\n 1:Water, 2:Glass, 3:Mirror\n");
            scanf("%1d", &material);
        }
        sudo_element_copy->material = material;
        push_rectangle(&rect_link_head, sudo_element_copy);

    }else if (action == 1)
    {
        sudo_item_active = 1;
        sudo_element = create_sudo_element(posX, posY);
    }
}

GLFWwindow* window_init()
{
    GLFWwindow* window = glfwCreateWindow(1632, 918, "Raycomp", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
    }
    
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetCursorPosCallback(window, cursor_pos_callback);
    gladLoadGL();
    glfwSwapInterval(1);
    return window;
}



void render_rectangles(unsigned int rectangle_shader_program, rectangle_link* rect_link_head)
{
    rectangle_link* current = rect_link_head;

    while (current != NULL)
    {

        vec2 size = {
            current->rect_ptr->width,
            current->rect_ptr->height,
        };
        unsigned int sizeLocation = glGetUniformLocation(rectangle_shader_program, "size");
        glUniform2fv(sizeLocation, 1, size);

        mat4 rotation;
        glm_mat4_identity(rotation);
        glm_rotate_z(rotation, current->rect_ptr->rot, rotation);
        int rotationLocation = glGetUniformLocation(rectangle_shader_program, "rotation");
        glUniformMatrix4fv(rotationLocation, 1, GL_FALSE, *rotation);

        mat4 model;
        glm_mat4_identity(model);
        glm_translate(model, current->rect_ptr->pos);
        int modelLocation = glGetUniformLocation(rectangle_shader_program, "model");
        glUniformMatrix4fv(modelLocation, 1, GL_FALSE, *model);

        int materialLocation = glGetUniformLocation(rectangle_shader_program, "material");
        glUniform1i(materialLocation, current->rect_ptr->material);

        glDrawArrays(GL_TRIANGLES, 0, 6);
        current = current->next_rect;
    }
}

rectangle create_sudo_element(int posX, int posY)
{
    rectangle sudo_element = {
        {2*posX-WINDOW_WIDTH, -(2*posY-WINDOW_HEIGHT), 0.0},
        0,
        0,
        0,
        UNCREATED
    };
    return sudo_element;
}

void render_sudo_item(unsigned int shaderProgram, rectangle sudo_element)
{
    vec2 size = {
        sudo_element.width,
        sudo_element.height,
    };
    int sizeLocation = glGetUniformLocation(shaderProgram, "size");
    glUniform2fv(sizeLocation, 1, size);

    mat4 rotation;
    glm_mat4_identity(rotation);
    glm_rotate_z(rotation, sudo_element.rot, rotation);
    int rotationLocation = glGetUniformLocation(shaderProgram, "rotation");
    glUniformMatrix4fv(rotationLocation, 1, GL_FALSE, *rotation);

    mat4 model;
    glm_mat4_identity(model);
    glm_translate(model, sudo_element.pos);
    int modelLocation = glGetUniformLocation(shaderProgram, "model");
    glUniformMatrix4fv(modelLocation, 1, GL_FALSE, *model);

    int materialLocation = glGetUniformLocation(shaderProgram, "material");
    glUniform1i(materialLocation, sudo_element.material);

    glDrawArrays(GL_TRIANGLES, 0, 6);
}

int check_laser_collision(vec3 position, int *current_material, rectangle **current_rectangle)
{
    rectangle_link* current = rect_link_head;

    while (current != NULL)
    {
        mat4 inv_rot;
        glm_mat4_identity(inv_rot);
        glm_rotate_z(inv_rot, -current->rect_ptr->rot, inv_rot);

        vec4 position_relative = {
            position[0]-current->rect_ptr->pos[0],
            position[1]-current->rect_ptr->pos[1],
            0.0,
            0.0,
        };
        vec4 result;
        glm_mat4_mulv(inv_rot, position_relative, result); 


        int collision_x = fabs(result[0])<=fabs(current->rect_ptr->width/2)?1:0;
        int collision_y = fabs(result[1])<=fabs(current->rect_ptr->height/2)?1:0;

        if (collision_x && collision_y)
        {
            *current_material = current->rect_ptr->material;
            *current_rectangle = current->rect_ptr;
            return 1;
        }
        current = current->next_rect;
    }
    return 0;
}

int is_in_rectangle(rectangle *current_rectangle, vec3 position)
{
    mat4 inv_rot;
    glm_mat4_identity(inv_rot);
    glm_rotate_z(inv_rot, -current_rectangle->rot, inv_rot);

    vec4 position_relative = {
        position[0]-current_rectangle->pos[0],
        position[1]-current_rectangle->pos[1],
        0.0,
        0.0,
    };
    vec4 result;
    glm_mat4_mulv(inv_rot, position_relative, result); 

    int collision_x = fabs(result[0])<=fabs(current_rectangle->width/2)?1:0;
    int collision_y = fabs(result[1])<=fabs(current_rectangle->height/2)?1:0;

    return (collision_x && collision_y)?1:0;
}

void render_laser(vec3 last_pos, vec3 pos, unsigned int shaderProgram)
{
    vec4 comp_pos = {
        last_pos[0],
        last_pos[1],
        pos[0],
        pos[1],
    };
    unsigned int posLocation = glGetUniformLocation(shaderProgram, "rPos");
    glUniform4fv(posLocation, 1, comp_pos);

    glDrawArrays(GL_LINES, 0, 2);
}

float speed_in_mat(int material)
{
    switch (material){
        case AIR:
            return SPEED_IN_AIR;
        case WATER:
            return SPEED_IN_WATER;
        case GLASS:
            return SPEED_IN_GLASS;
        case MIRROR:
            return -SPEED_IN_AIR;
    }
}

void transition_vel_from_air(vec3 vel, rectangle *prev_material)
{
    float rotation_radians = prev_material->rot;
    int signY = vel[1]>0?1:-1;
    vec3 up_vec = {0.0, signY*1.0, 0.0};
    vec3 z_axis = {0.0, 0.0, 1.0};
    glm_vec3_rotate(up_vec, rotation_radians, Z_AXIS);
    int signX = up_vec[0]<vel[0]?-1:1;
    float incidenc_angle = glm_vec3_angle(vel, up_vec);
    float speed_in_material = (float)speed_in_mat(prev_material->material);
    float refraction_angle = asin(speed_in_material*sinf(incidenc_angle));
    glm_vec3_rotate(up_vec, signY*signX*refraction_angle, Z_AXIS);
    vel[0] = up_vec[0];
    vel[1] = up_vec[1];
    return;
}

void transition_vel_to_air(vec3 vel, rectangle *current_rectangle)
{
    float rotation_radians = current_rectangle->rot;
    int signY = vel[1]>0?1:-1;
    vec3 up_vec = {0.0, signY*1.0, 0.0};
    glm_vec3_rotate(up_vec, rotation_radians, Z_AXIS);
    float incidenc_angle = glm_vec3_angle(up_vec, vel);
    float speed_in_material = (float)speed_in_mat(current_rectangle->material);
    float refraction_angle = asin(sinf(incidenc_angle)/speed_in_material);
    int signX = up_vec[0]<vel[0]?-1:1;
    glm_vec3_rotate(up_vec, signY*signX*refraction_angle, Z_AXIS);
    vel[0] = up_vec[0];
    vel[1] = up_vec[1];
    return;
}

void mirror_reflection(vec3 vel, rectangle *current_rectangle){
    vec3 up_vec = {0.0, 1.0, 0.0};
    vec3 z_axis = {0.0, 0.0, 1.0};
    glm_vec3_rotate(up_vec, current_rectangle->rot, z_axis);
    vel[0] = -vel[0];
    vel[1] = -vel[1];
    glm_vec3_rotate(vel, 3.1415, up_vec);
    return;
}

void compute_lasers(unsigned int laserShaderProgram)
{
    vec3 last_poition = {-1300.0f, -918.0f, 0.0f};
    vec3 position = {-1300.0f, -918.0f, 0.0f};
    vec3 velocity = {0.0f, 1.0f, 0.0};
    int current_material = AIR;
    rectangle* current_rectangle = (rectangle*)malloc(sizeof(rectangle));
    while ((position[0] >= -1632.0f && position[0] <= 1632)&&(position[1] >= -918.0f && position[1] <= 918.0f))
    {
        position[0] += velocity[0];
        position[1] += velocity[1];

        if (current_material == AIR)
        {
            if (check_laser_collision(position, &current_material, &current_rectangle))
            {
                render_laser(last_poition, position, laserShaderProgram);
                if (current_material == MIRROR){
                    mirror_reflection(velocity, current_rectangle);
                    current_material = AIR;
                }else{
                    transition_vel_from_air(velocity, current_rectangle);

                }
                last_poition[0] = position[0];
                last_poition[1] = position[1];
            }
        } else
        {
            if (!is_in_rectangle(current_rectangle, position))
            {
                render_laser(last_poition, position, laserShaderProgram);
                transition_vel_to_air(velocity, current_rectangle);
                last_poition[0] = position[0];
                last_poition[1] = position[1];
                current_material = AIR;
            }
        }
    }
    render_laser(last_poition, position, laserShaderProgram);
}

int main(){

    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // glfw window creation
    // --------------------
    GLFWwindow* window = window_init();

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        printf("unalived\n");
        return -1;
    }
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);

    // Shaders
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);

    int success;
    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        printf("ERROR::SHADER::VERTEX::COMPILATION_FAILED\n");
    }

    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);

    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        printf("ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n");
    }

    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        printf("ERROR::SHADER::PROGRAM::LINKING_FAILED\n");
    }
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // set up vertex data and configure vertex attributes
    float vertices[] = {
        -0.5f, -0.5f, 0.0f,
        0.5f, -0.5f, 0.0f,
        -0.5f, 0.5f, 0.0f,
        -0.5f, 0.5f, 0.0f,
        0.5f, 0.5f, 0.0f,
        0.5f, -0.5f, 0.0f, 
    }; 


    unsigned int VBO, VAO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    // bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0); 
    glBindVertexArray(0);

    // Laser Shaders
    unsigned int laserVertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(laserVertexShader, 1, &laserVertexShaderSource, NULL);
    glCompileShader(laserVertexShader);

    glGetShaderiv(laserVertexShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(laserVertexShader, 512, NULL, infoLog);
        printf("ERROR::SHADER::VERTEX::COMPILATION_FAILED\n");
    }

    unsigned int laserFragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(laserFragmentShader, 1, &laserFragmentShaderSource, NULL);
    glCompileShader(laserFragmentShader);

    glGetShaderiv(laserFragmentShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(laserFragmentShader, 512, NULL, infoLog);
        printf("ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n");
    }

    unsigned int laserShaderProgram = glCreateProgram();
    glAttachShader(laserShaderProgram, laserVertexShader);
    glAttachShader(laserShaderProgram, laserFragmentShader);
    glLinkProgram(laserShaderProgram);

    glGetProgramiv(laserShaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(laserShaderProgram, 512, NULL, infoLog);
        printf("ERROR::SHADER::PROGRAM::LINKING_FAILED\n");
    }
    glDeleteShader(laserVertexShader);
    glDeleteShader(laserFragmentShader);

    unsigned int laser_VBO, laser_VAO;
    glGenVertexArrays(1, &laser_VAO);
    glGenBuffers(1, &laser_VBO);
    // bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
    glBindVertexArray(laser_VAO);

    float lines[] = {0.0f, 1.0f};
    glBindBuffer(GL_ARRAY_BUFFER, laser_VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(lines), lines, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 1, GL_FLOAT, GL_FALSE, 1 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0); 
    glBindVertexArray(0);

    rectangle test = {
        {-1250, 0, 0},
        500,
        300,
        -0.55,
        WATER,
    };
    push_rectangle(&rect_link_head, &test);

    while (!glfwWindowShouldClose(window))
    {
        glfwGetFramebufferSize(window, &WINDOW_WIDTH, &WINDOW_HEIGHT);

        processInput(window);

        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glBindVertexArray(VAO); 
        glUseProgram(shaderProgram);
        render_rectangles(shaderProgram, rect_link_head);

        if (sudo_item_active)
        {
            render_sudo_item(shaderProgram, sudo_element);
        }


        glUseProgram(laserShaderProgram);
        glBindVertexArray(laser_VAO);

        compute_lasers(laserShaderProgram);

        glLineWidth(5);

        glfwSwapBuffers(window);
        glfwPollEvents();
        //sleep(2);
    }

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteProgram(shaderProgram);

    glfwTerminate();
    return 0;

}

