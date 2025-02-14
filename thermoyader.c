#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <ft2build.h>
#include FT_FREETYPE_H
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// Ширина и высота окна
#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600

// Максимальная длина строки
#define MAX_INPUT_LENGTH 255

// Структура для хранения символа
typedef struct {
    GLuint textureID;   // ID текстуры символа
    int width, height;  // Размеры символа
    int bearingX;       // Смещение по X
    int bearingY;       // Смещение по Y
    GLuint advance;     // Расстояние до следующего символа
} Character;

// Глобальные переменные
Character characters[128]; // Хранит данные для всех ASCII символов
GLuint shaderProgram;      // Шейдерная программа
GLuint VAO, VBO;           // Вершинный массив и буфер
GLFWwindow* window;        // Окно GLFW
char inputBuffer[MAX_INPUT_LENGTH] = ""; // Буфер для ввода
int inputLength = 0;       // Длина ввода

// Функция для компиляции шейдера
GLuint compileShader(const char* source, GLenum type) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);

    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        fprintf(stderr, "Ошибка компиляции шейдера: %s\n", infoLog);
        exit(EXIT_FAILURE);
    }

    return shader;
}

// Функция для создания шейдерной программы
GLuint createShaderProgram(const char* vertexSource, const char* fragmentSource) {
    GLuint vertexShader = compileShader(vertexSource, GL_VERTEX_SHADER);
    GLuint fragmentShader = compileShader(fragmentSource, GL_FRAGMENT_SHADER);

    GLuint program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);

    GLint success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(program, 512, NULL, infoLog);
        fprintf(stderr, "Ошибка линковки шейдерной программы: %s\n", infoLog);
        exit(EXIT_FAILURE);
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return program;
}

// Функция для загрузки шрифта
void loadFont(const char* fontPath) {
    FT_Library ft;
    if (FT_Init_FreeType(&ft)) {
        fprintf(stderr, "Ошибка инициализации FreeType\n");
        exit(EXIT_FAILURE);
    }

    FT_Face face;
    if (FT_New_Face(ft, fontPath, 0, &face)) {
        fprintf(stderr, "Ошибка загрузки шрифта\n");
        exit(EXIT_FAILURE);
    }

    FT_Set_Pixel_Sizes(face, 0, 48); // Установка размера шрифта

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1); // Отключение выравнивания байт

    for (unsigned char c = 0; c < 128; c++) {
        if (FT_Load_Char(face, c, FT_LOAD_RENDER)) {
            fprintf(stderr, "Ошибка загрузки символа: %c\n", c);
            continue;
        }

        GLuint texture;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(
            GL_TEXTURE_2D,
            0,
            GL_RED,
            face->glyph->bitmap.width,
            face->glyph->bitmap.rows,
            0,
            GL_RED,
            GL_UNSIGNED_BYTE,
            face->glyph->bitmap.buffer
        );

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        Character character = {
            texture,
            face->glyph->bitmap.width,
            face->glyph->bitmap.rows,
            face->glyph->bitmap_left,
            face->glyph->bitmap_top,
            face->glyph->advance.x
        };
        characters[c] = character;
    }

    FT_Done_Face(face);
    FT_Done_FreeType(ft);
}

// Функция для отрисовки текста
void renderText(const char* text, float x, float y, float scale, float color[3]) {
    glUseProgram(shaderProgram);
    glUniform3f(glGetUniformLocation(shaderProgram, "textColor"), color[0], color[1], color[2]);
    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(VAO);

    for (const char* c = text; *c; c++) {
        Character ch = characters[*c];

        float xpos = x + ch.bearingX * scale;
        float ypos = y - (ch.height - ch.bearingY) * scale;

        float w = ch.width * scale;
        float h = ch.height * scale;

        float vertices[6][4] = {
            { xpos,     ypos + h,   0.0f, 0.0f },
            { xpos,     ypos,       0.0f, 1.0f },
            { xpos + w, ypos,       1.0f, 1.0f },

            { xpos,     ypos + h,   0.0f, 0.0f },
            { xpos + w, ypos,       1.0f, 1.0f },
            { xpos + w, ypos + h,   1.0f, 0.0f }
        };

        glBindTexture(GL_TEXTURE_2D, ch.textureID);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        glDrawArrays(GL_TRIANGLES, 0, 6);

        x += (ch.advance >> 6) * scale; // Сдвиг на ширину символа
    }

    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
}

// Функция для обработки ввода с клавиатуры
void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (action == GLFW_PRESS || action == GLFW_REPEAT) {
        if (key == GLFW_KEY_BACKSPACE && inputLength > 0) {
            inputBuffer[--inputLength] = '\0';
        } else if (key == GLFW_KEY_ENTER) {
            // Обработка команды
            if (strcmp(inputBuffer, "Joshua") == 0) {
                renderText("OK", 10.0f, 50.0f, 1.0f, (float[3]){1.0f, 1.0f, 1.0f});
            } else if (strcmp(inputBuffer, "Help Games") == 0) {
                renderText("'GAMES' REFERS TO MODELS, SIMULATIONS AND GAMES", 10.0f, 50.0f, 1.0f, (float[3]){1.0f, 1.0f, 1.0f});
                renderText("WHICH HAVE TACTICAL AND STRATEGIC APPLICATIONS", 10.0f, 70.0f, 1.0f, (float[3]){1.0f, 1.0f, 1.0f});
            } else if (strstr(inputBuffer, "help") != NULL) {
                renderText("HELP NOT AVAILABLE", 10.0f, 50.0f, 1.0f, (float[3]){1.0f, 1.0f, 1.0f});
            } else {
                renderText("INDENTIFICATION NOT RECOGNIZED BY SYSTEM", 10.0f, 50.0f, 1.0f, (float[3]){1.0f, 1.0f, 1.0f});
                renderText("--CONNECTION TERMINATED--", 10.0f, 70.0f, 1.0f, (float[3]){1.0f, 1.0f, 1.0f});
            }
            inputLength = 0;
            memset(inputBuffer, 0, MAX_INPUT_LENGTH);
        } else if (inputLength < MAX_INPUT_LENGTH - 1) {
            inputBuffer[inputLength++] = (char)key;
            inputBuffer[inputLength] = '\0';
        }
    }
}

int main() {
    // Инициализация GLFW
    if (!glfwInit()) {
        fprintf(stderr, "Ошибка инициализации GLFW\n");
        return -1;
    }

    // Создание окна
    window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "OpenGL Terminal", NULL, NULL);
    if (!window) {
        fprintf(stderr, "Ошибка создания окна GLFW\n");
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    // Инициализация GLEW
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        fprintf(stderr, "Ошибка инициализации GLEW\n");
        return -1;
    }

    // Настройка OpenGL
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Загрузка шрифта
    loadFont("path/to/your/font.ttf");

    // Создание шейдерной программы
    const char* vertexShaderSource = R"(
        #version 330 core
        layout(location = 0) in vec4 vertex;
        out vec2 TexCoords;
        uniform mat4 projection;
        void main() {
            gl_Position = projection * vec4(vertex.xy, 0.0, 1.0);
            TexCoords = vertex.zw;
        }
    )";

    const char* fragmentShaderSource = R"(
        #version 330 core
        in vec2 TexCoords;
        out vec4 color;
        uniform sampler2D text;
        uniform vec3 textColor;
        void main() {
            vec4 sampled = vec4(1.0, 1.0, 1.0, texture(text, TexCoords).r);
            color = vec4(textColor, 1.0) * sampled;
        }
    )";

    shaderProgram = createShaderProgram(vertexShaderSource, fragmentShaderSource);

    // Настройка проекции
    glUseProgram(shaderProgram);
    glm::mat4 projection = glm::ortho(0.0f, (float)WINDOW_WIDTH, 0.0f, (float)WINDOW_HEIGHT);
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

    // Настройка VAO и VBO
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // Установка callback для ввода с клавиатуры
    glfwSetKeyCallback(window, keyCallback);

    // Основной цикл
    while (!glfwWindowShouldClose(window)) {
        // Очистка экрана
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // Отрисовка текста
        renderText("LOGON: ", 10.0f, 10.0f, 1.0f, (float[3]){1.0f, 1.0f, 1.0f});
        renderText(inputBuffer, 70.0f, 10.0f, 1.0f, (float[3]){1.0f, 1.0f, 1.0f});

        // Обмен буферов и обработка событий
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Завершение работы GLFW
    glfwTerminate();
    return 0;
}