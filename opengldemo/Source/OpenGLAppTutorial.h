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

        float margin = 0.f;
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
            uniforms->iTime->set((GLfloat)juce::Time::getMillisecondCounterHiRes() * 0.001f);

        openGLContext.extensions.glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
        openGLContext.extensions.glEnableVertexAttribArray(positionAttribute);
        openGLContext.extensions.glVertexAttribPointer(positionAttribute, 2, GL_FLOAT, GL_FALSE, 0, 0);

        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        openGLContext.extensions.glDisableVertexAttribArray(positionAttribute);
        openGLContext.extensions.glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    void paint(juce::Graphics& g) override
    {

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
                fragCoord = position * 0.5 + 0.5; // Transform to [0, 1] range
                gl_Position = vec4(position, 0.0, 1.0);
            }
        )";

        fragmentShader = R"(
            const float pi = 3.14159265359;
            const float tau = 2. * pi;

            uniform vec3 iResolution;
            uniform float iTime;
            varying vec2 fragCoord;

            float noise(vec3 p) {
                return fract(sin(dot(p, vec3(12.9898, 78.233, 45.164))) * 43758.5453);
            }

            float noise(vec3 p, float time) {
                return noise(p + vec3(time));
            }

            float Noise2d(in vec2 x) {
                float xhash = cos(x.x * 37.0);
                float yhash = cos(x.y * 57.0);
                return fract(415.92653 * (xhash + yhash));
            }

            float NoisyStarField(in vec2 vSamplePos, float fThreshhold) {
                float StarVal = Noise2d(vSamplePos);
                if (StarVal >= fThreshhold)
                    StarVal = pow((StarVal - fThreshhold) / (1.0 - fThreshhold), 6.0);
                else
                    StarVal = 0.0;
                return StarVal;
            }

            float StableStarField(in vec2 vSamplePos, float fThreshhold) {
                float fractX = fract(vSamplePos.x);
                float fractY = fract(vSamplePos.y);
                vec2 floorSample = floor(vSamplePos);
                float v1 = NoisyStarField(floorSample, fThreshhold);
                float v2 = NoisyStarField(floorSample + vec2(0.0, 1.0), fThreshhold);
                float v3 = NoisyStarField(floorSample + vec2(1.0, 0.0), fThreshhold);
                float v4 = NoisyStarField(floorSample + vec2(1.0, 1.0), fThreshhold);
                float StarVal = v1 * (1.0 - fractX) * (1.0 - fractY) + v2 * (1.0 - fractX) * fractY + v3 * fractX * (1.0 - fractY) + v4 * fractX * fractY;
                return StarVal;
            }

            vec3 trig_palette(vec3 a, vec3 b, vec3 c, vec3 d, float t) {
                return a + b * cos((tau * c * t + d));
            }

            float sphere_distance(vec3 p) {
                return length(p);
            }

            vec3 rotated_x(float a, vec3 p) {
                float ca = cos(a);
                float sa = sin(a);
                return vec3(p.x, ca * p.y + sa * p.z, -sa * p.y + ca * p.z);
            }

            vec3 rotated_y(float a, vec3 p) {
                float ca = cos(a);
                float sa = sin(a);
                return vec3(ca * p.x - sa * p.z, p.y, sa * p.x + ca * p.z);
            }

            vec3 sphere_0_position() {
                return vec3(0.);
            }

            float sphere_0_radius() {
                return 1.;
            }

            vec3 sphere_0_color(vec3 p) {
                vec2 xy = p.xy - sphere_0_position().xy;
                float ang = atan(xy.y, xy.x);
                vec3 pal = trig_palette(vec3(.8, .2, .2), vec3(.2, .2, .2), vec3(1., 1., 1.), vec3(.0, .33, .67), ang / tau);
                return mix(vec3(1., 0., 0.), pal, min(length(xy), 1.));
            }

            float sphere_0_density(vec3 p) {
                vec3 d = sphere_0_position() - p;
                float l = length(d);
                vec3 q = sphere_0_radius() * normalize(d);
                return abs(noise(q * 2., iTime / 5. - l));
            }

            float sphere_0_distance(vec3 p) {
                return sphere_distance(p - sphere_0_position()) - 1.;
            }

            vec3 sphere_1_position() {
                return rotated_y(iTime, vec3(3, 0., 0.));
            }

            float sphere_1_radius() {
                return 1.0;
            }

            vec3 sphere_1_color(vec3 p) {
                vec2 xy = p.xy - sphere_1_position().xy;
                float ang = atan(xy.y, xy.x);
                vec3 pal = trig_palette(vec3(.2, .6, .3), vec3(.2, .2, .3), vec3(1., 1., 1.), vec3(.0, .33, .67), ang / tau);
                return mix(vec3(0., 1., 0.), pal, min(length(xy), 1.));
            }

            float sphere_1_density(vec3 p) {
                vec3 d = sphere_1_position() - p;
                float l = length(d);
                vec3 q = sphere_1_radius() * normalize(d);
                return abs(noise(q * 1.5, iTime / 5. - l));
            }

            float sphere_1_distance(vec3 p) {
                return sphere_distance(p - sphere_1_position()) - 0.5f;
            }

            float scene_distance(vec3 p) {
                float s0 = sphere_0_distance(p);
                float s1 = sphere_1_distance(p);
                return min(s0, s1);
            }

            vec4 scene_color(vec3 p) {
                float s0 = sphere_0_distance(p);
                float s1 = sphere_1_distance(p);
                if (s0 < s1) {
                    return vec4(sphere_0_color(p), 0.5);  // 0.5 alpha for transparency
                } else {
                    return vec4(sphere_1_color(p), 1.0);  // Opaque
                }
            }

            float scene_density(vec3 p) {
                float s0 = sphere_0_distance(p);
                float s1 = sphere_1_distance(p);
                if (s0 < s1) {
                    return sphere_0_density(p);
                } else {
                    return sphere_1_density(p);
                }
            }

            vec4 scene_halo(vec3 p, vec3 d) {
                float sd0 = 2. * sphere_0_distance(p) / 1.8;
                vec3 v0 = p - sphere_0_position();
                float a0 = 2. / (1. + sd0 * sd0 * sd0);
                float sd1 = 2. * sphere_1_distance(p) / 1.8;
                vec3 v1 = p - sphere_1_position();
                float a1 = 2. / (1. + sd1 * sd1 * sd1);
                float sum_a = a0 + a1;
                vec3 rgb = (a0 * sphere_0_color(p) + a1 * sphere_1_color(p)) / sum_a;
                float a = 1. - (1. - a0) * (1. - a1);
                float density = .2 + .8 * scene_density(p);
                return vec4(rgb, density * a);
            }

            void mainImage(out vec4 rgba, in vec2 coords) {
                coords = (2. * coords.xy - iResolution.xy) / iResolution.x;
                vec3 ro = vec3(0., 3., -10.);
                vec3 rd = rotated_x(-0.2, normalize(vec3(coords.xy, 1.)));
                vec4 halo = vec4(0.);
                for (int i = 0; i < 150; ++i) {
                    float sd = min(scene_distance(ro), 0.25);
                    ro += sd * rd;
                    if (sd < 0.01) {
                        break;
                    } else {
                        vec4 halo_i = scene_halo(ro, rd);
                        halo += (1. - halo.a) * sd * vec4(halo_i.rgb * halo_i.a, halo_i.a);
                    }
                }
                vec3 rgb = vec3(StableStarField(1000. * (coords + vec2(1.)), .97));
                float d = length(ro);
                if (d < 20.) {
                    float noi = scene_density(ro);
                    noi = pow(noi, 1. / 5.);
                    rgb = mix(scene_color(ro).rgb, vec3(1.), noi);
                }
                rgba = vec4(halo.rgb + (1. - halo.a) * rgb, 1.);
            }

            void main()
            {
                mainImage(gl_FragColor, fragCoord.xy * iResolution.xy);
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

        std::unique_ptr<juce::OpenGLShaderProgram::Uniform> iResolution;
        std::unique_ptr<juce::OpenGLShaderProgram::Uniform> iTime;

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