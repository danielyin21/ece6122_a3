/*
Author: Haoran Yin
Class: ECE6122 A
Last Date Modified: 2024/10/18
Description:
    This is the source code for lab 3. I approach this lab in a dumb but efficient way, here is the modification I made:
    1. This source code is mainly based on Tutorial 9 AssImp from the OpenGL tut. To make my life easier when it comes to the final project,
        I modified the loadAssImp function to load multiple models at once. I also created a separate model class per Dr/Hurley's suggestion
        In this way I was able to individually load in the chess pieces and potentially modify their position in the final project
    2. For the board obj file, I modified: a. the orientation and the scale of the board manually; b. I triangulated the board manually due to the fact that the
       aiProcess flag of triangulation is not working well with it
    3. For the chess obj file, I modified:
        - first, since the original obj's are large and way off the grid, I scale them individually to the correct offset (the origin of the world)
        and apply a correct scale that matches the board
        - second, this is where the dumb part comes in, I manually duplicated the pieces into 32 in total, and then based on the board,
        manually placed them on the board and scale their locations into a new obj file
    4. For the jpg file, I use the windows paint to convert it to the correct BMP file that loadBMP_custom can read (this is only for the board)
    5. I rewrite the shader files to include the functionality of specular and diffuse lighting, and I also added a flag to toggle between them
    Some of the modifications might not be necessary with respect to this lab, but I think they definitely will help me in the final project
*/
// Include standard headers
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <map>
#include <string>

// Include GLEW
#include <GL/glew.h>

// Include GLFW
#include <GLFW/glfw3.h>
GLFWwindow* window;
bool isSpecularAndDiffuseEnabled = true;  // Flag to toggle specular and diffuse lighting

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;

#include <common/shader.hpp>
#include <common/texture.hpp>
#include <common/controls.hpp>
#include <common/objloader.hpp>
#include <common/vboindexer.hpp>
#include <iostream>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <vector>
#include <string>
#include <cstring>
#include <glm/glm.hpp>

/*
 * This class is for each individual object from the obj file so that later I can manipulate each object individually
 * */
class Model {
public:
    // Constructor
    Model() : vertexbuffer(0), uvbuffer(0), normalbuffer(0), elementbuffer(0), VAO(0), textureID(0){}

    // Destructor
    ~Model() {
        glDeleteBuffers(1, &vertexbuffer);
        glDeleteBuffers(1, &uvbuffer);
        glDeleteBuffers(1, &normalbuffer);
        glDeleteBuffers(1, &elementbuffer);
        glDeleteVertexArrays(1, &VAO);
    }

    // Stores the model data
    std::vector<unsigned int> indices;
    std::vector<glm::vec3> indexedVertices;
    std::vector<glm::vec2> indexedUVs;
    std::vector<glm::vec3> indexedNormals;

    GLuint vertexbuffer;
    GLuint uvbuffer;
    GLuint normalbuffer;
    GLuint elementbuffer;
    GLuint textureID;
    std::string texturePath;
    GLuint VAO;

    // Function to load the model data into buffers
    // this function is from the tutorial 9
    void loadBuffers()
    {
        // initialize and bind VAO
        glGenVertexArrays(1, &VAO);
        glBindVertexArray(VAO);

        // Vertex buffer
        glGenBuffers(1, &vertexbuffer);
        glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
        glBufferData(GL_ARRAY_BUFFER, indexedVertices.size() * sizeof(glm::vec3), &indexedVertices[0], GL_STATIC_DRAW);

        // UV buffer
        glGenBuffers(1, &uvbuffer);
        glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
        glBufferData(GL_ARRAY_BUFFER, indexedUVs.size() * sizeof(glm::vec2), &indexedUVs[0], GL_STATIC_DRAW);

        // Normal buffer
        glGenBuffers(1, &normalbuffer);
        glBindBuffer(GL_ARRAY_BUFFER, normalbuffer);
        glBufferData(GL_ARRAY_BUFFER, indexedNormals.size() * sizeof(glm::vec3), &indexedNormals[0], GL_STATIC_DRAW);

        std::cout << "UV count: " << indexedUVs.size() << std::endl;
        std::cout << "Normal count: " << indexedNormals.size() << std::endl;
        std::cout << "Vertices count: " << indexedVertices.size() << std::endl;
        std::cout << "Triangle count: " << indices.size() / 3 << std::endl;

        // Element buffer (indices)
        glGenBuffers(1, &elementbuffer);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbuffer);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);
    }

    void loadTexture()
    {
        if (!texturePath.empty())
            textureID = loadBMP_custom(texturePath.c_str());  // 使用你的 loadBMP_custom 函数加载纹理
    }

    // Render the model
    // this is the rendering code from the tutorial as well
    void render()
    {
        glBindVertexArray(VAO);
        // Enable vertex attribute arrays
        if (textureID != 0)
            glBindTexture(GL_TEXTURE_2D, textureID);
//        printf("Reached 2\n");

        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

        glEnableVertexAttribArray(1);
        glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);

        glEnableVertexAttribArray(2);
        glBindBuffer(GL_ARRAY_BUFFER, normalbuffer);
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

        // Bind the element buffer
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbuffer);

        // Draw the triangles
        glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, (void*)0);
//        printf("Reached 3\n");

        // Disable the vertex attribute arrays
        glDisableVertexAttribArray(0);
        glDisableVertexAttribArray(1);
        glDisableVertexAttribArray(2);
//        glBindVertexArray(0);
//        std::cout << "Vertices count: " << indexedVertices.size() << std::endl;
//        std::cout << "Triangle count: " << indices.size() / 3 << std::endl;
    }
};

// this is basically the same code as the original loadAssImp, but I modified it to load multiple objects at once
bool loadAssImpModified(
        const char * path,
        std::vector<Model> & models,  // 存储多个 Model 对象
        bool isLoadBoard
) {
    Assimp::Importer importer;
//    Model model;
    const aiScene* scene = importer.ReadFile(path, 0);
    if (!scene)
    {
        fprintf(stderr, "%s", importer.GetErrorString());
        getchar();
        return false;
    }

    // parse all meshes from the obj file, make sure it will also work with single mesh obj file
    for (unsigned int m = 0; m < scene->mNumMeshes; ++m)
    {
        printf("Processing mesh %d\n", m);
//        int verticeCount = model.indexedVertices.size();
        const aiMesh* mesh = scene->mMeshes[m];

        // create and initialize a model class for whatever mesh we are processing
        Model model;

        // code from the original loadAssImp function
        model.indexedVertices.reserve(mesh->mNumVertices);
        for (unsigned int i = 0; i < mesh->mNumVertices; ++i)
        {
            aiVector3D pos = mesh->mVertices[i];
            model.indexedVertices.push_back(glm::vec3(pos.x, pos.y, pos.z));
        }

        if (mesh->HasTextureCoords(0))
        {
            model.indexedUVs.reserve(mesh->mNumVertices);
            for (unsigned int i = 0; i < mesh->mNumVertices; ++i)
            {
                aiVector3D UVW = mesh->mTextureCoords[0][i];
                model.indexedUVs.push_back(glm::vec2(UVW.x, UVW.y));
            }
        }

        if (mesh->HasNormals())
        {
            model.indexedNormals.reserve(mesh->mNumVertices);
            for (unsigned int i = 0; i < mesh->mNumVertices; ++i)
            {
                aiVector3D n = mesh->mNormals[i];
                model.indexedNormals.push_back(glm::vec3(n.x, n.y, n.z));
            }
        }

        model.indices.reserve(3 * mesh->mNumFaces);
        for (unsigned int i = 0; i < mesh->mNumFaces; ++i)
        {
            const aiFace& face = mesh->mFaces[i];
            model.indices.push_back(face.mIndices[0]);
            model.indices.push_back(face.mIndices[1]);
            model.indices.push_back(face.mIndices[2]);
        }

        // check the texture path and initialize the texture
        if (scene->HasMaterials())
        {
            aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex]; // 获取当前网格的材质
            aiString texturePath;

            if (material->GetTexture(aiTextureType_DIFFUSE, 0, &texturePath) == AI_SUCCESS)
            {
                std::string fullPath = std::string(texturePath.C_Str());

                // have this flag so that i can specifically load in those textures
                if (isLoadBoard)
                    fullPath = "Stone_Chess_Board/" + fullPath;
                else
                    fullPath = "Chess/" + fullPath;
                printf("Texture path: %s\n", fullPath.c_str());

                model.texturePath = fullPath;
            }
        }

        models.push_back(model);
    }

    return true;
}

int main() {
    // Initialize GLFW
    if (!glfwInit()) {
        fprintf(stderr, "Failed to initialize GLFW\n");
        getchar();
        return -1;
    }

    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make macOS happy; should not be needed
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Open a window and create its OpenGL context
    window = glfwCreateWindow(1024, 768, "Lab3", NULL, NULL);
    if (window == NULL) {
        fprintf(stderr,
                "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n");
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
    // Hide the mouse and enable unlimited movement
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // Set the mouse at the center of the screen
    glfwPollEvents();
    glfwSetCursorPos(window, 1024 / 2, 768 / 2);

    // Dark blue background
    glClearColor(0.2f, 0.2f, 0.2f, 0.0f);

    // Enable depth test
    glEnable(GL_DEPTH_TEST);
    // Accept fragment if it is closer to the camera than the former one
    glDepthFunc(GL_LESS);

    // Cull triangles which normal is not towards the camera
    glEnable(GL_CULL_FACE);

/******************************************* Actual Lab3 starts here *******************************************/

    GLuint VertexArrayID;
    glGenVertexArrays(1, &VertexArrayID);
    glBindVertexArray(VertexArrayID);

    // Create and compile our GLSL program from the shaders
    GLuint programID = LoadShaders("StandardShading.vertexshader", "StandardShading.fragmentshader");

    // Get a handle for our "MVP" uniform
    GLuint MatrixID = glGetUniformLocation(programID, "MVP");
    GLuint ViewMatrixID = glGetUniformLocation(programID, "V");
    GLuint ModelMatrixID = glGetUniformLocation(programID, "M");

    // Get a handle for our "myTextureSampler" uniform
    GLuint LightID = glGetUniformLocation(programID, "LightPosition_worldspace");
    GLuint SpecularDiffuseFlagID = glGetUniformLocation(programID, "isSpecularAndDiffuseEnabled");

    // actual loading of the objects
    std::vector<Model> chessBoard;
    if (loadAssImpModified("Stone_Chess_Board/stone_chess_board_triangulated.obj", chessBoard, true)) {
        for (auto &i: chessBoard) {
            i.loadBuffers();
            i.loadTexture();
        }

        std::vector<Model> chessPieces;
        if (loadAssImpModified("Chess/chess_correct_offset_individual.obj", chessPieces, false)) {
            for (auto &i: chessPieces) {
                i.loadBuffers();
                i.loadTexture();
            }
        }

        // Get a handle for our "LightPosition" uniform
        glUseProgram(programID);
        glm::vec3 lightPos = glm::vec3(1, 6, 1);

        // For speed computation
        double lastTime = glfwGetTime();
        int nbFrames = 0;
        bool isLKeyPressed = false;

        do {
            // Measure speed
            double currentTime = glfwGetTime();
            nbFrames++;
            if (currentTime - lastTime >= 1.0) { // If last prinf() was more than 1sec ago
                // printf and reset
                printf("%f ms/frame\n", 1000.0 / double(nbFrames));
                nbFrames = 0;
                lastTime += 1.0;
            }

            // Debounce mechanism for 'L' key toggle
            if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS && !isLKeyPressed) {
                // Toggle the lighting flag when 'L' is pressed and was not pressed before
                isSpecularAndDiffuseEnabled = !isSpecularAndDiffuseEnabled;
                isLKeyPressed = true;  // Mark that the key is pressed
            }

            // When the 'L' key is released, reset the state
            if (glfwGetKey(window, GLFW_KEY_L) == GLFW_RELEASE) {
                isLKeyPressed = false;  // Reset the state when the key is released
            }

            // Clear the screen
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            // Use our shader
            glUseProgram(programID);

            // Compute the MVP matrix from keyboard and mouse input
            computeMatricesFromInputs();
            glm::mat4 ProjectionMatrix = getProjectionMatrix();
            glm::mat4 ViewMatrix = getViewMatrix();
            glm::mat4 ModelMatrix = glm::mat4(1.0);
            glm::mat4 MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;

            // Send our transformation to the currently bound shader,
            // in the "MVP" uniform
            glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
            glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);
            glUniformMatrix4fv(ViewMatrixID, 1, GL_FALSE, &ViewMatrix[0][0]);

            // Pass lighting uniforms
            glUniform3f(LightID, lightPos.x, lightPos.y, lightPos.z);
            glUniform1i(SpecularDiffuseFlagID, isSpecularAndDiffuseEnabled);

            for (auto &i: chessBoard) {
//            printf("Reached 1");
                i.render();
            }

            for (auto &i: chessPieces) {
                i.render();
            }

            // Swap buffers
            glfwSwapBuffers(window);
            glfwPollEvents();

        } // Check if the ESC key was pressed or the window was closed
        while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS &&
               glfwWindowShouldClose(window) == 0);

        // Cleanup VBO and shader
        glDeleteProgram(programID);
        glDeleteVertexArrays(1, &VertexArrayID);

        // Close OpenGL window and terminate GLFW
        glfwTerminate();

        return 0;
    }
}