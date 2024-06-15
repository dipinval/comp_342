#include <GLFW/glfw3.h>
#include <vector>
#include <iostream>
#include <cmath>

// Define a simple Vector2 structure
struct Vector2 {
    float x, y;
};

// Constants
const float k = 0.015f;                 // Spring constant
const float spread = 0.005f;            // Spread factor (adjusted for realism)
const float springSeparation = 24.0f / 640.0f; // Spring separation in OpenGL coordinates
const float gravity = 0.002f;           // Gravity
const float buoyancyFactor = 0.003f;    // Buoyancy factor
const float dragFactor = 0.98f;         // Drag factor (adjusted for realism)
const float tensionFactor = 0.007f;     // Surface tension factor (adjusted for realism)
const float dampingFactor = 0.98f;      // Damping factor (adjusted for realism)
const float rippleFactor = 0.001f;      // Ripple effect factor
const float waveFactor = 0.01f;         // Wave motion factor
const float waterLevelIncrement = 0.001f; // Water level increment per particle

// WaterParticle class
class WaterParticle {
public:
    Vector2 position;
    Vector2 velocity;

    WaterParticle(float x, float y) {
        position = { x, y };
        velocity = { 0.0f, 0.0f };
    }

    void update(float waterLevel) {
        // Apply gravity
        velocity.y -= gravity;

        // Apply buoyancy
        if (position.y < waterLevel) {
            velocity.y += buoyancyFactor;
        }

        // Apply drag
        velocity.x *= dragFactor;
        velocity.y *= dragFactor;

        // Apply damping
        velocity.x *= dampingFactor;
        velocity.y *= dampingFactor;

        // Update position
        position.x += velocity.x;
        position.y += velocity.y;

        // Collision with window boundaries
        if (position.y < -1.0f) {
            position.y = -1.0f;
            velocity.y = 0.0f;
        }
        if (position.y > 1.0f) {
            position.y = 1.0f;
            velocity.y = 0.0f;
        }
        if (position.x < -1.0f) {
            position.x = -1.0f;
            velocity.x = 0.0f;
        }
        if (position.x > 1.0f) {
            position.x = 1.0f;
            velocity.x = 0.0f;
        }
    }

    void applySpringForce(const WaterParticle& other) {
        Vector2 direction = { other.position.x - position.x, other.position.y - position.y };
        float distance = std::sqrt(direction.x * direction.x + direction.y * direction.y);

        if (distance < springSeparation && distance > 0.0f) {
            float force = k * (distance - springSeparation);
            direction.x /= distance;
            direction.y /= distance;

            float mass = 1.0f;  // Assuming unit mass for simplicity
            velocity.x += (force * direction.x * spread) / mass;
            velocity.y += (force * direction.y * spread) / mass;
        }
    }

    void applySurfaceTension(const WaterParticle& other) {
        Vector2 direction = { other.position.x - position.x, other.position.y - position.y };
        float distance = std::sqrt(direction.x * direction.x + direction.y * direction.y);

        if (distance > springSeparation && distance < 2 * springSeparation) {
            float force = tensionFactor * (2 * springSeparation - distance);
            direction.x /= distance;
            direction.y /= distance;

            float mass = 1.0f;  // Assuming unit mass for simplicity
            velocity.x += (force * direction.x) / mass;
            velocity.y += (force * direction.y) / mass;
        }
    }

    void applyRippleEffect(const Vector2& rippleCenter) {
        Vector2 direction = { rippleCenter.x - position.x, rippleCenter.y - position.y };
        float distance = std::sqrt(direction.x * direction.x + direction.y * direction.y);

        if (distance < 0.1f && distance > 0.0f) {  // Ripple effect range
            float force = rippleFactor * (0.1f - distance);
            direction.x /= distance;
            direction.y /= distance;

            float mass = 1.0f;  // Assuming unit mass for simplicity
            velocity.x -= (force * direction.x) / mass;
            velocity.y -= (force * direction.y) / mass;
        }
    }

    void applyWaveMotion(const Vector2& waveSource) {
        Vector2 direction = { waveSource.x - position.x, waveSource.y - position.y };
        float distance = std::sqrt(direction.x * direction.x + direction.y * direction.y);

        if (distance < 0.3f && distance > 0.0f) {  // Wave motion range
            float force = waveFactor * std::sin(distance * 10.0f);
            velocity.x += force * direction.x;
            velocity.y += force * direction.y;
        }
    }

    void draw() const {
        // Draw water particle as a circle
        int numSegments = 20;
        float radius = 0.03f;
        glColor4f(0.3f, 0.7f, 0.9f, 0.8f);  // Slightly transparent light blue
        glBegin(GL_TRIANGLE_FAN);
        glVertex2f(position.x, position.y);
        for (int i = 0; i <= numSegments; ++i) {
            float angle = i * 2.0f * 3.14159f / numSegments;
            glVertex2f(position.x + std::cos(angle) * radius, position.y + std::sin(angle) * radius);
        }
        glEnd();
    }
};

// Function to draw water level with gradient
void drawWaterLevel(float waterLevel) {
    glBegin(GL_QUADS);
    glColor4f(0.3f, 0.7f, 0.9f, 0.8f); // Light blue color at the water level
    glVertex2f(-1.0f, waterLevel);
    glVertex2f(1.0f, waterLevel);
    glColor4f(0.3f, 0.5f, 0.9f, 0.5f); // Darker blue color below the water level
    glVertex2f(1.0f, -1.0f);
    glVertex2f(-1.0f, -1.0f);
    glEnd();
}

// Main function
int main(void) {
    GLFWwindow* window;
    std::vector<WaterParticle> waterParticles;
    float waterLevel = -1.0f; // Initial water level at the bottom

    /* Initialize the library */
    if (!glfwInit())
        return -1;

    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(1280, 720, "Water Simulation", NULL, NULL);
    if (!window) {
        glfwTerminate();
        return -1;
    }

    /* Make the window's context current */
    glfwMakeContextCurrent(window);

    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window)) {
        /* Render here */
        glClearColor(0.576f, 0.871f, 0.973f, 1.0f); // Light sky blue background
        glClear(GL_COLOR_BUFFER_BIT);

        // Draw bounding box
        glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
        glBegin(GL_LINE_LOOP);
        glVertex2f(-1.0f, -1.0f);
        glVertex2f(1.0f, -1.0f);
        glVertex2f(1.0f, 1.0f);
        glVertex2f(-1.0f, 1.0f);
        glEnd();

        // Draw water level with gradient
        drawWaterLevel(waterLevel);

        // Apply spring forces, surface tension, and wave motion between particles
        for (size_t i = 0; i < waterParticles.size(); ++i) {
            for (size_t j = i + 1; j < waterParticles.size(); ++j) {
                waterParticles[i].applySpringForce(waterParticles[j]);
                waterParticles[j].applySpringForce(waterParticles[i]);
                waterParticles[i].applySurfaceTension(waterParticles[j]);
                waterParticles[j].applySurfaceTension(waterParticles[i]);
            }
        }

        // Update and draw water particles
        for (auto& particle : waterParticles) {
            particle.update(waterLevel);
            particle.draw();
        }

        /* Swap front and back buffers */
        glfwSwapBuffers(window);

        /* Poll for and process events */
        glfwPollEvents();

        // Check for mouse clicks
        if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
            double xpos, ypos;
            glfwGetCursorPos(window, &xpos, &ypos);

            // Convert screen coordinates to OpenGL coordinates
            float x = (float)xpos / 640.0f - 1.0f;
            float y = 1.0f - (float)ypos / 360.0f;

            // Add new water particle
            waterParticles.emplace_back(x, y);

            // Increase the water level
            waterLevel += waterLevelIncrement;
            if (waterLevel > 1.0f) {
                waterLevel = 1.0f; // Cap water level to the top of the window
            }

            // Apply ripple effect to existing particles
            Vector2 rippleCenter = { x, y };
            for (auto& particle : waterParticles) {
                particle.applyRippleEffect(rippleCenter);
            }

            // Apply wave motion effect to existing particles
            for (auto& particle : waterParticles) {
                particle.applyWaveMotion(rippleCenter);
            }
        }
    }

    glfwTerminate();
    return 0;
}
