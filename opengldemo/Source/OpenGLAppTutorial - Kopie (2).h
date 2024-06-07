#include <JuceHeader.h>
using namespace ::juce::gl;


//==============================================================================
class MainContentComponent : public juce::OpenGLAppComponent
{
public:
    MainContentComponent()
    {
        setSize(800, 600);
    }

    ~MainContentComponent() override
    {
        shutdownOpenGL();
    }

    void initialise() override
    {
        createShaders();
        openGLContext.extensions.glGenBuffers(1, &vertexBuffer);
        openGLContext.extensions.glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);

        float margin = 0.1f;
        float vertices[] = {
                -1.0f + margin,  1.0f - margin,
                -1.0f + margin, -1.0f + margin,
                1.0f - margin,  1.0f - margin,
                1.0f - margin, -1.0f + margin,
        };

        openGLContext.extensions.glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
        openGLContext.extensions.glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    void shutdown() override
    {
        shader.reset();
        uniforms.reset();
        openGLContext.extensions.glDeleteBuffers(1, &vertexBuffer);
    }

    void render() override
    {
        using namespace ::juce::gl;

        jassert(juce::OpenGLHelpers::isContextActive());

        auto desktopScale = (float)openGLContext.getRenderingScale();
        juce::OpenGLHelpers::clear(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));

        glViewport(0, 0, juce::roundToInt(desktopScale * (float)getWidth()), juce::roundToInt(desktopScale * (float)getHeight()));

        shader->use();

        if (uniforms->iResolution.get() != nullptr)
            uniforms->iResolution->set((GLfloat)getWidth(), (GLfloat)getHeight(), desktopScale);

        if (uniforms->iTime.get() != nullptr)
            uniforms->iTime->set((GLfloat)juce::Time::getMillisecondCounterHiRes() / 1000.0f);

        openGLContext.extensions.glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
        openGLContext.extensions.glEnableVertexAttribArray(positionAttribute);
        openGLContext.extensions.glVertexAttribPointer(positionAttribute, 2, GL_FLOAT, GL_FALSE, 0, 0);

        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        openGLContext.extensions.glDisableVertexAttribArray(positionAttribute);
        openGLContext.extensions.glBindBuffer(GL_ARRAY_BUFFER, 0);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    }

    void paint(juce::Graphics& g) override
    {
        g.setColour(getLookAndFeel().findColour(juce::Label::textColourId));
        g.setFont(20);
        g.drawText("OpenGL Example", 25, 20, 300, 30, juce::Justification::left);
    }

    void resized() override
    {
        // This is called when the MainContentComponent is resized.
    }

    void createShaders()
    {
        vertexShader = R"(
            attribute vec2 position;
            varying vec2 fragCoord;
            void main()
            {
                fragCoord = position;
                gl_Position = vec4(position, 0.0, 1.0);
            }
        )";
        fragmentShader = R"(
            uniform vec3 iResolution;
            uniform float iTime;
            varying vec2 fragCoord;

            void main()
            {
                gl_FragColor = vec4(0.0, 1.0, 0.0, 1.0); // Output green color
            }
        )";

        std::unique_ptr<juce::OpenGLShaderProgram> newShader(new juce::OpenGLShaderProgram(openGLContext));
        juce::String statusText;

        if (newShader->addVertexShader(juce::OpenGLHelpers::translateVertexShaderToV3(vertexShader)) &&
            newShader->addFragmentShader(juce::OpenGLHelpers::translateFragmentShaderToV3(fragmentShader)) &&
            newShader->link())
        {
            shader.reset(newShader.release());
            shader->use();

            uniforms.reset(new Uniforms(*shader));

            positionAttribute = glGetAttribLocation(shader->getProgramID(), "position");

            statusText = "GLSL: v" + juce::String(juce::OpenGLShaderProgram::getLanguageVersion(), 2);
        }
        else
        {
            auto vertexError = newShader->getLastError();
            DBG("Vertex Shader Error: " + vertexError);  // Output the error to the console

            auto fragmentError = newShader->getLastError();
            DBG("Fragment Shader Error: " + fragmentError);  // Output the error to the console

            statusText = newShader->getLastError();
            DBG("Linking Error: " + statusText);  // Output the error to the console

            jassertfalse;  // Break in debugger to inspect the issue
        }
    }

private:
    GLuint vertexBuffer;
    GLint positionAttribute;
    juce::String vertexShader;
    juce::String fragmentShader;

    std::unique_ptr<juce::OpenGLShaderProgram> shader;

    struct Uniforms
    {
        explicit Uniforms(juce::OpenGLShaderProgram& shaderProgram)
        {
            iResolution.reset(createUniform(shaderProgram, "iResolution"));
            iTime.reset(createUniform(shaderProgram, "iTime"));
        }

        std::unique_ptr<juce::OpenGLShaderProgram::Uniform> iResolution, iTime;

    private:
        static juce::OpenGLShaderProgram::Uniform* createUniform(juce::OpenGLShaderProgram& shaderProgram, const juce::String& uniformName)
        {
            using namespace ::juce::gl;
            auto location = glGetUniformLocation(shaderProgram.getProgramID(), uniformName.toRawUTF8());
            if (location < 0)
                return nullptr;

            return new juce::OpenGLShaderProgram::Uniform(shaderProgram, uniformName.toRawUTF8());
        }
    };

    std::unique_ptr<Uniforms> uniforms;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainContentComponent)
};
