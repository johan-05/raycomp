#include "include/glad/glad.h"

#include "glfw3.h"
#include <math.h>
#include <stdlib.h>
#include <stdio.h>

#include "cglm/vec3.h"
#include "cglm/mat4.h"
#include "cglm/types.h"
#include "cglm/affine-pre.h"

#include <string.h>
#include <strings.h>
#include <unistd.h>

#define GLFW_INCLUDE_NONE
#define CGLM_DEFINE_PRINTS
#define STB_IMAGE_IMPLEMENTATION

#define PI 3.1415

#define AIR 0
#define WATER 1
#define GLASS 2
#define OIL 3
#define MIRROR 4
#define UNCREATED 5

#define SPEED_IN_AIR 2.99
#define SPEED_IN_WATER 2.16
#define SPEED_IN_GLASS 2.02
#define SPEED_IN_OIL 1.99

#define TRIANGLE  3
#define RECTANGLE  4


const char *vertexShaderSource = "#version 330 core\n"
    "layout (location = 0) in float indx;\n"
    "uniform vec4 x_pos;\n"
    "uniform vec4 y_pos;\n"
    "void main()\n"
    "{\n"
  //  "   gl_Position = vec4(aPos.x, aPos.y, 0.0, 1.0);\n"
    "   if (indx == 0.0){\n"
    "       gl_Position = vec4(x_pos.x/1632, y_pos.x/918, 0.0, 1.0);\n"
    "   } else if (indx == 1.0){\n"
    "       gl_Position = vec4(x_pos.y/1632, y_pos.y/918, 0.0, 1.0);\n"
    "   } else if (indx == 2.0){\n"
    "       gl_Position = vec4(x_pos.z/1632, y_pos.z/918, 0.0, 1.0);\n"
    "   } else{\n"
    "       gl_Position = vec4(x_pos.w/1632, y_pos.w/918, 0.0, 1.0);\n"
    "   }\n"
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
    "       FragColor = vec4(0.60f, 0.60f, 0.0f, 1.0f);\n"
    "   } else if (material == 4){\n"
    "       FragColor = vec4(0.45f, 0.45f, 0.45f, 1.0f);\n"
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
    int type;
    int material;
    float points[8];
}polygon;

typedef struct{
    polygon* poly_ptr;
    void* next_poly;
}polygon_link;

typedef struct{
    vec2 link_point;
    float freq;
}laser;

typedef struct{
    laser* laser_ptr;
    void* next_line;
}laser_link;


polygon_link* poly_link_head = NULL;
laser_link* laser_link_head = NULL;

//polygon create_sudo_element(int posX, int posY);

void push_polygon(polygon_link** head_ref, polygon* poly_ptr)
{
    polygon_link* new_link = (polygon_link*)malloc(sizeof(polygon_link));
 
    new_link->poly_ptr = poly_ptr;
    new_link->next_poly = (struct polygon_link*)(*head_ref);
    (*head_ref) = new_link;
}

void push_laser(laser_link** head_ref, laser* laser_ptr)
{
    laser_link* new_link = (laser_link*)malloc(sizeof(laser_link));

    new_link->laser_ptr = laser_ptr;
    new_link->next_line = (struct laser_link*)(*head_ref);
}

int WINDOW_HEIGHT = 918;
int WINDOW_WIDTH = 1632;

int posX = 0;
int posY = 0;

int sudo_element_active = 0;

vec3 Z_AXIS = {0.0, 0.0, 1.0};

polygon *sudo_element;

polygon *create_sudo_element(int type);


void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

void cursor_pos_callback(GLFWwindow* window, double xpos, double ypos)
{
    posX = 2*xpos-WINDOW_WIDTH;
    posY = -(2*ypos-WINDOW_HEIGHT);
    if (sudo_element_active)
    {
        if (sudo_element->points[2] == 69420.0)
        {
            sudo_element->points[0] = posX;
            sudo_element->points[1] = posY;
        }else if (sudo_element->points[4] == 69420.0)
        {
            sudo_element->points[2] = posX;
            sudo_element->points[3] = posY;
        } else if (sudo_element->points[6] == 69420.0 || sudo_element->points[6] == 0.0)
        {
            sudo_element->points[4] = posX;
            sudo_element->points[5] = posY;
        } else if (sudo_element->type == RECTANGLE)
        {
            sudo_element->points[6] = posX;
            sudo_element->points[7] = posY;
        }
    }
}

void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, 1);
    } 

    if (glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_PRESS)
    {
        sudo_element_active = 1;
        printf("polygon sides\n");
        int buff;
        scanf("%1d", &buff);
        printf("beofre\n");
        sudo_element = create_sudo_element(buff);
        printf("after\n");
    }
}

void mouse_button_callback(GLFWwindow *window, int button, int action, int mod)
{
    if (action == 0 && sudo_element_active == true)
    {
        printf("%f, %f, %f, %f\n", 
            sudo_element->points[0], 
            sudo_element->points[1], 
            sudo_element->points[2],
            sudo_element->points[3]
            );

        if (sudo_element->points[2] == 69420.0)
        {
            sudo_element->points[2] = 0;
        }else if (sudo_element->points[4] == 69420.0)
        {
            sudo_element->points[4] = 0;
        }else if (sudo_element->points[6] == 69420.0)
        {
            sudo_element->points[6] = 5;
        }else
        {
            sudo_element_active = false;
            int material = 5;
            while (material > 4 || material < 1)
            {
                printf("select material\n 1:Water, 2:Glass, 3:Oil, 4:Mirror\n");
                int result = scanf("%1d", &material);
            }
            sudo_element->material = material;
            push_polygon(&poly_link_head, sudo_element);
        }

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



void render_polygons(unsigned int polygon_shader_program, polygon_link* poly_link_head)
{
    polygon_link* current = poly_link_head;

    while (current != NULL)
    {
        vec4 x_pos = {
            current->poly_ptr->points[0],
            current->poly_ptr->points[2],
            current->poly_ptr->points[4],
            current->poly_ptr->points[6],
        };
        vec4 y_pos = {
            current->poly_ptr->points[1],
            current->poly_ptr->points[3],
            current->poly_ptr->points[5],
            current->poly_ptr->points[7],
        };


        unsigned int xlocation = glGetUniformLocation(polygon_shader_program, "x_pos");
        glUniform4fv(xlocation, 1, x_pos);

        unsigned int ylocation = glGetUniformLocation(polygon_shader_program, "y_pos");
        glUniform4fv(ylocation, 1, y_pos);

        int materialLocation = glGetUniformLocation(polygon_shader_program, "material");
        glUniform1i(materialLocation, current->poly_ptr->material);

        if (current->poly_ptr->type == TRIANGLE)
        {
            glDrawArrays(GL_TRIANGLES, 0, 3);
        } else{
            glDrawArrays(GL_TRIANGLES, 0, 6);
        }
        current = current->next_poly;
    }
}

polygon *create_sudo_element(int type)
{
    polygon *sudo_element_thing = (polygon*)malloc(sizeof(polygon));
    if (type == TRIANGLE)
    {
        polygon element = {
            TRIANGLE,
            UNCREATED,
            {
                69420.0, 69420.0,
                69420.0, 69420.0,
                69420.0, 69420.0,
                0.0, 0.0
            },
        };
        memcpy(sudo_element_thing, &element, sizeof(polygon));
    } else{
        polygon element = {
            RECTANGLE,
            UNCREATED,
            {
                69420.0, 69420.0,
                69420.0, 69420.0,
                69420.0, 69420.0,
                69420.0, 69420.0
            }
        };
        memcpy(sudo_element_thing, &element, sizeof(polygon));

    }

    return sudo_element_thing;
}

void render_sudo_element(unsigned int shaderProgram, polygon *sudo_element)
{

    unsigned int posLocation = glGetUniformLocation(shaderProgram, "rPos");

    for (int i = 0; i<3; ++i)
    {
        if (sudo_element->points[2*i+2] != 69420.0 && sudo_element->points[2*i+2] != 0.0)
        {
            vec4 rPos = {
                sudo_element->points[2*i],
                sudo_element->points[2*i+1],
                sudo_element->points[2*i+2],
                sudo_element->points[2*i+3],
            };
            glUniform4fv(posLocation, 1, rPos);
            glDrawArrays(GL_LINES, 0, 2);
            //printf("render %f %f,   %f %f\n", rPos[0], rPos[1], rPos[2], rPos[3]);
        }
    }


}

int check_laser_collision(vec3 position, vec3 velocity, int *current_material, int *current_vertex, polygon **current_polygon)
{
    polygon_link* current = poly_link_head;

    while (current != NULL)
    {

        int indx_array[8];
        if (current->poly_ptr->type == TRIANGLE)
        {
            int new_arr[] = {0, 2, 4, 0, 2, 4, 0, 0};
            memcpy(indx_array, new_arr, sizeof(indx_array));
            
        }else{
            int new_arr[] = {0, 2, 4, 6, 2, 4, 6, 0};
            memcpy(indx_array, new_arr, sizeof(indx_array));
        }

        for (int i = 0; i<current->poly_ptr->type; ++i)
        {
            // if vector from vertex a to position is on left, and vector
            // from vertex a to to position + velcity is on right
            // and position to vectex a and positon to vertex b are on different 
            // sides from position + velocity. 
            vec3 vrtx = {
                current->poly_ptr->points[indx_array[i+4]] - current->poly_ptr->points[indx_array[i]],
                current->poly_ptr->points[indx_array[i+4]+1] - current->poly_ptr->points[indx_array[i]+1],
                0.0
            };

            vec3 vrtx_a_to_pos = {
                position[0] - current->poly_ptr->points[indx_array[i]],
                position[1] - current->poly_ptr->points[indx_array[i]+1],
                0.0,
            };

            vec3 vrtx_a_to_new_pos;
            glm_vec3_add(vrtx_a_to_pos, velocity, vrtx_a_to_new_pos);

            vec3 cross_old;
            vec3 cross_new;
            glm_vec3_cross(vrtx_a_to_pos, vrtx, cross_old);
            glm_vec3_cross(vrtx_a_to_new_pos, vrtx, cross_new);

            int switched_side_of_vertex = cross_old[2]*cross_new[2]<0;

            vec3 pos_to_vrtx_a;
            glm_vec3_scale(vrtx_a_to_pos, -1, pos_to_vrtx_a);
            vec3 pos_to_vrtx_b = {
                current->poly_ptr->points[indx_array[i+4]] - position[0],
                current->poly_ptr->points[indx_array[i+4]+1] - position[1],
            };

            glm_vec3_cross(pos_to_vrtx_a, velocity, cross_old);
            glm_vec3_cross(pos_to_vrtx_b, velocity, cross_new);

            int two_sides_of_vel = cross_old[2]*cross_new[2]<0;

            if (switched_side_of_vertex && two_sides_of_vel){
                *current_polygon = current->poly_ptr;
                *current_material = current->poly_ptr->material;
                *current_vertex = i;
                return 1;
            }
        }

        current = current->next_poly;
    }
    return 0;
}

int is_in_polygon(vec3 position, vec3 velocity, int *current_vertex, polygon *current_polygon)
{

    for (int i = 0; i<current_polygon->type; ++i)
    {
        // if vector from vertex a to position is on left, and vector
        // from vertex a to to position + velcity is on right
        // and position to vectex a and positon to vertex b are on different 
        // sides from position + velocity. 

        int indx_array[8];
        if (current_polygon->type == TRIANGLE)
        {
            int new_arr[] = {0, 2, 4, 0, 2, 4, 0, 0};
            memcpy(indx_array, new_arr, sizeof(indx_array));
            
        }else{
            int new_arr[] = {0, 2, 4, 6, 2, 4, 6, 0};
            memcpy(indx_array, new_arr, sizeof(indx_array));
        }


        vec3 vrtx = {
            current_polygon->points[indx_array[i+4]] - current_polygon->points[indx_array[i]],
            current_polygon->points[indx_array[i+4]+1] - current_polygon->points[indx_array[i]+1],
            0.0
        };


        vec3 vrtx_a_to_pos = {
            position[0] - current_polygon->points[indx_array[i]],
            position[1] - current_polygon->points[indx_array[i]+1],
            0.0,
        };

        vec3 vrtx_a_to_new_pos;
        glm_vec3_add(vrtx_a_to_pos, velocity, vrtx_a_to_new_pos);

        vec3 cross_old;
        vec3 cross_new;
        glm_vec3_cross(vrtx_a_to_pos, vrtx, cross_old);
        glm_vec3_cross(vrtx_a_to_new_pos, vrtx, cross_new);

        int switched_side_of_vertex = cross_old[2]*cross_new[2]<0;

        vec3 pos_to_vrtx_a;
        glm_vec3_scale(vrtx_a_to_pos, -1, pos_to_vrtx_a);
        vec3 pos_to_vrtx_b = {
            current_polygon->points[indx_array[i+4]] - position[0],
            current_polygon->points[indx_array[i+4]+1] - position[1],
        };

        glm_vec3_cross(pos_to_vrtx_a, velocity, cross_old);
        glm_vec3_cross(pos_to_vrtx_b, velocity, cross_new);

        int two_sides_of_vel = cross_old[2]*cross_new[2]<0;

        if (switched_side_of_vertex && two_sides_of_vel){
            *current_vertex = i;
            return 0;
        }
    }

    return 1;
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
        case OIL:
            return SPEED_IN_OIL;
        case MIRROR:
            return -SPEED_IN_AIR;
        default:
            return 69420.0;
    }
}

void transition_vel_from_air(vec3 vel, int vrtx_indx, polygon *current_polygon)
{
    int indx_array[8];
    if (current_polygon->type == TRIANGLE)
    {
        int new_arr[] = {0, 2, 4, 0, 2, 4, 0, 0};
        memcpy(indx_array, new_arr, sizeof(indx_array));
        
    }else{
        int new_arr[] = {0, 2, 4, 6, 2, 4, 6, 0};
        memcpy(indx_array, new_arr, sizeof(indx_array));
    }

    vec3 vrtx = {
        current_polygon->points[indx_array[vrtx_indx+4]] - current_polygon->points[indx_array[vrtx_indx]],
        current_polygon->points[indx_array[vrtx_indx+4]+1] - current_polygon->points[indx_array[vrtx_indx]+1],
        0.0
    };

    vec3 norm;
    glm_vec3_dup(vrtx, norm);
    glm_vec3_rotate(norm, -PI/2, Z_AXIS);

    vec3 cross_result;
    glm_vec3_cross(vel, norm, cross_result);
    int sign = cross_result[2]<0?1:-1;

    float incidence_angle = sign*glm_vec3_angle(norm, vel);

    float speed_in_material = (float)speed_in_mat(current_polygon->material);
    float refraction_angle = asin(speed_in_material*sinf(incidence_angle)/SPEED_IN_AIR);

    glm_vec3_rotate(norm, refraction_angle, Z_AXIS);
    glm_normalize(norm);
    vel[0] = norm[0]; 
    vel[1] = norm[1];
    
    return;
}

void mirror_reflection(vec3 vel, int vrtx_indx, polygon *current_polygon);


int transition_vel_to_air(vec3 vel, int vrtx_indx, polygon *prev_polygon)
{

    int indx_array[8];
    if (prev_polygon->type == TRIANGLE)
    {
        int new_arr[] = {0, 2, 4, 0, 2, 4, 0, 0};
        memcpy(indx_array, new_arr, sizeof(indx_array));
        
    }else{
        int new_arr[] = {0, 2, 4, 6, 2, 4, 6, 0};
        memcpy(indx_array, new_arr, sizeof(indx_array));
    }


    vec3 vrtx = {
        prev_polygon->points[indx_array[vrtx_indx+4]] - prev_polygon->points[indx_array[vrtx_indx]],
        prev_polygon->points[indx_array[vrtx_indx+4]+1] - prev_polygon->points[indx_array[vrtx_indx]+1],
        0.0
    };

    vec3 norm;
    glm_vec3_dup(vrtx, norm);

    glm_vec3_rotate(norm, -PI/2, Z_AXIS);
    vec3 cross_result;
    glm_vec3_cross(vel, norm, cross_result);
    int sign = cross_result[2]<0?1:-1;
    glm_vec3_scale(norm, sign, norm);
    float incidence_angle = glm_vec3_angle(norm, vel);

    float speed_in_material = (float)speed_in_mat(prev_polygon->material);
    float refraction_angle = asin(SPEED_IN_AIR*sinf(incidence_angle)/speed_in_material);
    if (refraction_angle!=refraction_angle){
        mirror_reflection(vel, vrtx_indx, prev_polygon);
        return 0;
    }
    glm_vec3_rotate(norm, refraction_angle, Z_AXIS);
    glm_normalize(norm);
    vel[0] = norm[0];
    vel[1] = norm[1];

    return 1;
}

void mirror_reflection(vec3 vel, int vrtx_indx, polygon *current_polygon)
{
    int indx_array[8];
    if (current_polygon->type == TRIANGLE)
    {
        int new_arr[] = {0, 2, 4, 0, 2, 4, 0, 0};
        memcpy(indx_array, new_arr, sizeof(indx_array));
        
    }else{
        int new_arr[] = {0, 2, 4, 6, 2, 4, 6, 0};
        memcpy(indx_array, new_arr, sizeof(indx_array));
    }


    vec3 vrtx = {
        current_polygon->points[indx_array[vrtx_indx+4]] - current_polygon->points[indx_array[vrtx_indx]],
        current_polygon->points[indx_array[vrtx_indx+4]+1] - current_polygon->points[indx_array[vrtx_indx]+1],
        0.0
    };

    vec3 norm;
    glm_vec3_dup(vrtx, norm);
    glm_vec3_rotate(norm, -PI/2, Z_AXIS);

    vec3 out_vec;
    glm_vec3_scale(vel, -1, out_vec);
    glm_vec3_rotate(out_vec, PI, norm);
    vel[0] = out_vec[0];
    vel[1] = out_vec[1];
    return;
}

void compute_lasers(unsigned int laserShaderProgram)
{
    vec3 last_poition = {-1300.0f, -918.0f, 0.0f};
    vec3 position = {-1300.0f, -918.0f, 0.0f};
    vec3 velocity = {0.0f, 1.0f, 0.0};
    int current_material = AIR;
    int current_vertex = 0;
    polygon* current_polygon = (polygon*)malloc(sizeof(polygon));
    while ((position[0] >= -1632.0f && position[0] <= 1632)&&(position[1] >= -918.0f && position[1] <= 918.0f))
    {
        position[0] += velocity[0];
        position[1] += velocity[1];

        if (current_material == AIR)
        {
            if (check_laser_collision(position, velocity, &current_material, &current_vertex, &current_polygon))
            {
                render_laser(last_poition, position, laserShaderProgram);
                if (current_material == MIRROR)
                {
                    mirror_reflection(velocity, current_vertex, current_polygon);
                    current_material = AIR;
                }else{
                    transition_vel_from_air(velocity, current_vertex, current_polygon);

                }
                last_poition[0] = position[0];
                last_poition[1] = position[1];
            }
        } else
        {
            if (!is_in_polygon(position, velocity, &current_vertex, current_polygon))
            {
                render_laser(last_poition, position, laserShaderProgram);
                if (transition_vel_to_air(velocity, current_vertex, current_polygon)){;
                    current_material = AIR;
                }
                last_poition[0] = position[0];
                last_poition[1] = position[1];
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
        0.0, 1.0, 2.0, 
        0.0, 2.0, 3.0,
    }; 


    unsigned int VBO, VAO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    // bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 1, GL_FLOAT, GL_FALSE, 1 * sizeof(float), (void*)0);
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

    polygon test = {
        TRIANGLE,
        GLASS,
        {
            -1400, -500,
            -1100, 200,
            -700, -750,
            0, 0,
        }
    };

    push_polygon(&poly_link_head, &test);


    while (!glfwWindowShouldClose(window))
    {
        glfwGetFramebufferSize(window, &WINDOW_WIDTH, &WINDOW_HEIGHT);

        processInput(window);

        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glBindVertexArray(VAO); 
        glUseProgram(shaderProgram);
        render_polygons(shaderProgram, poly_link_head);



        glUseProgram(laserShaderProgram);
        if (sudo_element_active)
        {
            render_sudo_element(laserShaderProgram, sudo_element);
        }

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

