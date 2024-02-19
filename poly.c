// -------------Raycomp----------------
// Velkommen til den mystiske innsiden av simuleringen som jeg
// har valgt å kalle Raycomp (Ray-computing). Jeg håper å kunne forklare
// programmet på en slik måte at man kan forstå ideen selv uten kjennskap
// til C eller programmering. Fundamentalt har programmet to deler. 
// Selve simuleringen er ikke det du ser på skjermen. Simuleringen eksisterer
// kun i usynlig matte. Den andre delen av programmet er grafikken som viser 
// resultatet av simuleringen. For grafikk bruker jeg OpenGL. Alle funksjoner 
// som begynner med gl hører til OpenGL biblioteket. Det er ikke det
// mest ideelle valget, men det er det eneste grafikk-biblioteket jeg har erfaring
// med. Jeg turte ikke gamble på å lære meg SDL2 eller Raylib selv om de passer
// bedre til denne typen lav-intensitet grafikk. Grunnen til at OpenGL gjør
// ting komplisert er at det kommuniserer med grafikkortet for å gjøre grafikken
// mye mer effektiv, men komplisert å programmere. Med det ute av veien
// la oss sette i gang!



// ----------------- Seksjon 1: definisjoner -------------------
// Denne første seksjonen inneholder definisjoner og inkluderingen av filer.
// Alt som starter med #include henter inn OpenGL biblioteket eller andre 
// nyttige funksjoner som brukes i programmet.

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

// #define er nesten som en variabel men mer som et 
// alias for en verdi. De kan ikke endres og tar ikke noe
// plass i minnet. Når programmet kompileres bytter kompileren
// SPEED_IN_AIR ut med 2.99 overalt i koden. 

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


// Allerede her begynner kompleksiteten jeg lovte med grafikkortet
// Disse fire konstantene er små program kallt "shaders" som kjøres
// på grafikkortet. en VertexShader samler distinkte punkt til en form
// enten en trekant eller firkant. En FragmentShader definerer fargen.
// Vi har Vertex og Fragment for mangekantene og for laserene.

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


// Den siste vi må definere er de komplekse typene vi trenger
// Disse heter struct og er omtrent som objekt i andre språk.

// polygon definerer en mangekant, enten en trekant eller firkant.
// Det kan se merkelig ut at typen mangekant og materialet er en int,
// men det er fordi vi har definert TRIANGLE som 3 og RECTANGLE som 4
// materialt er også definert som tall. C liker ikke takst som navn så
// på denne måten kan vi som programmerer navngi ting som tekst mens
// programmet bare ser tall.

typedef struct{
    int type;
    int material;
    float points[8];
}polygon;

// Polygon link er et element i en såkalt linket liste.
// OK, vi kommer ikke utenom en grunnleggende forklaing av pointere.
// En pointer kan tenkes på som en pil som peker til noe. I virkeligheten
// er det en minne-addresse hvor verdien befinner seg. Pointere noteres
// på formen "type *variabel_navn". Dette vil si at polygon *poly_ptr
// som vi kan se i polygon_link betyr at poly_ptr er navnet på variabelen'
// og at variabelen har datatypen pointer til en polygon. 
// En polygon_link har to pointere, en til polygonet og en til nest element
// i den linkede listen. 

typedef struct{
    polygon *poly_ptr;
    void *next_poly;
}polygon_link;

// laser structen er den simpleste av de fire: Det er en laser
// én laser vil si en rett linje. Hver knekk og refleksjon gir
// en ny laser. Laseren har kunn en posisjon representert som en vektor

typedef struct{
    vec2 link_point;
}laser;

// laser_link er samme ide som polygon_link. Sentralt til programmet
// er to linkede lister: en for mangekanter og en for lasere. Disse 
// renderes hver frame.  

typedef struct{
    laser *laser_ptr;
    void *next_line;
}laser_link;


//------------------ Seksjon 2: Stille før stormen -----------------
// Før vi begynner med det virkelig saftige stoffet har vi en del initialisering å gjøre
// Det er viktig å huske på at C i motsetning til språk som Python
// ikke kjører fra topp til bunn. Startpunktet for programmet er funksjonen
// int main() som ironisk nok er i bunnen av filen. Husk på at funksjoner ikke 
// kalles i rekkefølgen de er skrevet.

// Her initialisere vi de to linkede listene. "head" betyr starten på en
// linked liste. NULL betyr ingenting, altå konseptet ingenting. De viser
// at de to listene foreløpig ikke har noen elementer. 

polygon_link* poly_link_head = NULL;
laser_link* laser_link_head = NULL;

// Disse to funksjonene brukes for å legge til elementer i de linkede listene.
// De tar in listes "head" og en polygon eller laser og legger det til i listen
// C har ikke noe funksjon ord som def i python eller function i JS, men starter
// med den returnerende datatypen. void push_polygon() betyr at funksjonen heter
// push_polygon og returnere void, som omtrent som NULL betyr ingenting.

void push_polygon(polygon_link **head_ref, polygon *poly_ptr)
{
    polygon_link *new_link = (polygon_link*)malloc(sizeof(polygon_link));
 
    new_link->poly_ptr = poly_ptr;
    new_link->next_poly = (struct polygon_link*)(*head_ref);
    (*head_ref) = new_link;
    return;
}

void push_laser(laser_link **head_ref, laser *laser_ptr)
{
    laser_link* new_link = (laser_link*)malloc(sizeof(laser_link));

    new_link->laser_ptr = laser_ptr;
    new_link->next_line = (struct laser_link*)(*head_ref);
    return;
}

// Her har vi en definisjon av alle de 7 globale variabelene i programmet
// Med unntak av disse er alle andre variabler lokale.
// z-aksen er z-aksen, simpelt nok. WINDOW_HEIGHT og WIDTH er dimensjonene
// av vinduet som programmet lager. posX og posY vil senere bli brukt til å
// lagre mus-posisjonen. sude_element_active definerer om vi holder på å tegne
// en mangekant. Mangekanten som holder på å tegnes er sudo_element. Den defineres,
// men får ingen verdi enda. 

vec3 Z_AXIS = {0.0, 0.0, 1.0};

int WINDOW_HEIGHT = 918;
int WINDOW_WIDTH = 1632;

int posX = 0;
int posY = 0;

int sudo_element_active = false;

polygon *sudo_element;

// Ikke tenkt på denne
polygon *create_sudo_element(int type);



//------------------ Seksjon 3: OpenGL relatert -----------------
// Dette er en rekke funksjoner som samhandler med OpenGL sitt event-system
// frambuffer_size_callback() passer på at programmet tilpasser seg endringer
// i vinduets størrelse

void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
    glViewport(0, 0, width, height);
}

// Denne funksjonene kalles av OpenGL hver gang musen beveger på seg
// I dette programmet brues den til å finne posisjonen til hjørnene
// av nye mangekanter. 
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


// Denne funksjonen tar seg av tastatur-input. Det brukes til å
// finne ut når vi skal lage en my mangekant

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


// Denne funksjonen kalles hver gang en av mus-knappene klikkes. 
// funksjonen jobber tett med mouse_pos_callback for å definere mangekanter

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

// Window_init() gjør omtrent det navnet tilsier. Funksjonen kalles tidlig
// i programmet og setter opp vinduet med dimensjoner og navn. Den fester også 
// de tidligere nevnte dunksjonene til "OpenGL conteksten" slik at de kan ta del
// i event-systemet.

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


//------------------ Seksjon 4: Rendering -----------------
// Så nå har vi definert en hel rekke datatyper, vi har en GL context
// og vi kan lytte til event-systemet så nå er det på tide å begynne
// å fylle de to linkede listene og rendere dem.

// Denne funksjonen initialiserer et nytt sudo_element
// Den kalles når brukeren trykker ENTER. 
// sude_elementet som skapes er fyllt av 69420 som bare betyr en
// uinitialisert verdi. posisjonsverdiene fylles ut av mouse_pos_callback()

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


// Denne funksjonen renderer sudo_elementet. 

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
        }
    }
}


// Denne funksjonene renderer mangekantene.
// Det som skjer her er at vi looper over alle mangekantene i den linkede
// listen, så lager vi to firedimensjonale vektorer med x og y kordinatene
// til punktene som definerer mangekanten, sender det over en data-bro
// til grafikkortet som gjør sin magi med Shaderene vi så tidligere. 

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


// Mye er likt som med mangekantene når vi skal rendere laserene
// Vi har en liste med lasere, vi bruker den til å definere en start og
// slittposisjon for laseren også sender vi den dataen til grafikkortet

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

//-------------------- Seksjon 5: Simulering ---------------------
// Nå er vi kommet til selve hjertet av programmet: Selve simuleringen
// Her er funksjonene lengre og mer kompliserte.
// Den første vi skal se på sjekker for kollisjoner mellom en laser og mangekantene.
// Funksjonen looper over alle sidekantene til alle mangekanatene og sjekker for
// kollisjoner med et smart triks. En kollisjon vil si at laserens posisjon
// bytter side av sidekante. Jeg bruker vektor kryssprodukt for å finne ut
// hvilken side av sidekanten laseren er på. 

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

// Denne funksjonen er veldig lik den forrige. 
// Den sjekker for kollisjoner men bar for en mangekant.
// Når vi har kollidert med en mangekant vet vi at neste kollisjon
// vil være laseren på vei ut av den samme mangekanten. Derfor trenger vi ikke
// sjekke for kollisjoner mellom sidekantene til alle de andre mangekantene.
// Funkjonen heter is_in_polygon fordi den returnerer true hvis vi fremdeles er inne
// i den samme mangekanten og false hvis vi har gått ut. 

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

// Dette er en kort funksjon som returnerer lysfarten i materialet
// som puttes in. 

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

void mirror_reflection(vec3 vel, int vrtx_indx, polygon *current_polygon);

// De to funksjonene som gjør selve lys-bøyingen. 
// Den første er vel_from_air altså bøying fra luft inn i et
// annet materiale. Den andre vel_to_air gjør bøyingen fra materiale
// tilbake til luft. (Det er store muligheter for bugs om du tegner
// mangekanter oppå hverandre)

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

    // Den første delen av funksjonen har satt sammen punkt til en sidekant vrtx
    // Det neste vi gjør er å lage en normalvektor til sidekanten ved å duplisere
    // sidekanten også rotere den rundt z-aksen 

    vec3 norm;
    glm_vec3_dup(vrtx, norm);
    glm_vec3_rotate(norm, -PI/2, Z_AXIS);

    // For å finne innfallsvinkelen bruker jeg en OpenGL funksjon som heter
    // glm_vec3_angle som gir vinkelen mellom to vektorer. Problemet er at vi kan
    // ha en 30° innfallsvinkel fra begge sider og funksjonen ikke skiller mellom 30° og -30°.
    // Derfor trenger jeg en "sign" skalar på enten 1 eller -1 for å skille mellom 
    // innfallsvinkel fra høyere og venstre.

    vec3 cross_result;
    glm_vec3_cross(vel, norm, cross_result);
    int sign = cross_result[2]<0?1:-1;

    float incidence_angle = sign*glm_vec3_angle(norm, vel);
    // Dermed får vi innfallsvinkelen som "sign" ganger med vinkelen mellom velocity og 
    // normal-vinkelen

    float speed_in_material = (float)speed_in_mat(current_polygon->material);
    float refraction_angle = asin(speed_in_material*sinf(incidence_angle)/SPEED_IN_AIR);

    // Med den kan vi kalkulere refraksjonsvinkelen og rotere normlvektoren
    // med refraksjonsvinkelen. Dette blir den nye verdien for velocity.

    glm_vec3_rotate(norm, refraction_angle, Z_AXIS);
    glm_normalize(norm);
    vel[0] = norm[0]; 
    vel[1] = norm[1];
    
    return;
}

// I motsetning til transistion_vel_from_air som returnerte void returnerer
// denne bool fordi bøyingen ut av et materiale kan feile. Hvis innfallsvinkelen
// er stor nok vil laseren reflektere internt og funksjonen returnere false. 
// med unntak av det er det mest likt som den forrige funksjonen

bool transition_vel_to_air(vec3 vel, int vrtx_indx, polygon *prev_polygon)
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
    // Ved intern refleksjon vil asin funksjonen returnere NaN (not a number) som har
    // en unik egenskap. Den er aldri lik noe. NaN er den eneste verdien som ikke er lik seg selv
    // Dette bruker jeg for å kontrollere om brytningen var vellykket
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

// Ved intern refleksjon eller kollisjon med et speil vil paseren bøyes med denne
// funksjonen. Den er en del simplere enn de to forrige.

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

    // Vi setter sammen sidekanten likt som før.
    // Alt vi gjør er å lage en normalvektor og speiler velocity rundt normalvektoren
    // og endrer retningen.

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

// Dette er selve loopen som simulerer laseren fram til den forlater sjermen
// Vi starter med å gi laseren en start posisjon og start fart

void compute_lasers(unsigned int laserShaderProgram)
{
    vec3 last_poition = {-1300.0f, -918.0f, 0.0f};
    vec3 position = {-1300.0f, -918.0f, 0.0f};
    vec3 velocity = {0.0f, 1.0f, 0.0};
    int current_material = AIR;
    int current_vertex = 0;
    polygon *current_polygon = (polygon*)malloc(sizeof(polygon));

    // Så looper vi fra til posisjoner er utenfor sjermen. 
    // Inne i loopen endrer vi posisjoen med farten og sjekker for
    // kollisjoner. 

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
    return;
}



//-------------------- Seksjon 6: Starten? wtf ---------------------
// yep, dette er starten av programmet. Alt fram til nå har ikke gjort noe som helst.


int main(){

    // Først er det hel haug med initialisering av OpenGL contexten og slikt.
    // Vi laster driverne og lager en context og en vindu.

    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = window_init();

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        printf("unalived\n");
        return -1;
    }
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);

        
    // Tidligere i programmet har jeg bare skrevet at jeg har
    // sendt data over mystiske data-broer til grafikk-kortet, vel
    // nå kommer vi til å faktisk måtte sette opp alt dette.
    // Vi kompilerer shaderene, linker dem og definerer data-broen
    // Vi er også nødt til å lage to Vertex Array Objekt og Vertex 
    // Buffer objekt for å lagre dataen i.

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

    float vertices[] = {
        0.0, 1.0, 2.0, 
        0.0, 2.0, 3.0,
    }; 


    unsigned int VBO, VAO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
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
    glBindVertexArray(laser_VAO);

    float lines[] = {0.0f, 1.0f};
    glBindBuffer(GL_ARRAY_BUFFER, laser_VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(lines), lines, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 1, GL_FLOAT, GL_FALSE, 1 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0); 
    glBindVertexArray(0);

    
    // Til slutt har vi endelig kommet til selve event-loopen
    // Denne kjører en gang for hver frame, omtrent hvert 16 ms

    while (!glfwWindowShouldClose(window))
    {
        // Første oppdatere vi vindu-størrelsen i tilfølge vinduet har endret
        // størrelse. Deretter lytter vi etter mus og tastatur events.
        // Så renser vi vinduet, setter det til bakgrunnsfargen og binder
        // programmet for mangekanter til GL contexten. 

        glfwGetFramebufferSize(window, &WINDOW_WIDTH, &WINDOW_HEIGHT);
        processInput(window);
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        glBindVertexArray(VAO); 
        glUseProgram(shaderProgram);

        // Fordelen med å ha gjort så mye før programmet starter er at å rendere
        // alle mangekantene blir så enkelt som å kalle en funksjon.

        render_polygons(shaderProgram, poly_link_head);

        // Når som vi er ferdig med mangekantene kan vi binde programmet for
        // lasere til GL contexten.

        glUseProgram(laserShaderProgram);
        glBindVertexArray(laser_VAO);

        // Hvis vi har et sudo element aktivt må vi rendere det

        if (sudo_element_active)
        {
            render_sudo_element(laserShaderProgram, sudo_element);
        }

        // compute lasers gjør selve laser-simuleringen og renderer laserne

        glLineWidth(5);
        compute_lasers(laserShaderProgram);

        // Helt til slutt bytter vi video-bufferene for å ferdigstille den nye framen

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Dette kjører når programmet holder på å avslutte.
    // Vi sletter ressurser og avslutter.


    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteProgram(shaderProgram);

    glfwTerminate();
    return 0;
}

// Hvis du kom helt hit vil jeg bare si: Beklager!