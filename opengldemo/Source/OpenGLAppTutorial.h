/*
  ==============================================================================

   This file is part of the JUCE tutorials.
   Copyright (c) 2020 - Raw Material Software Limited

   The code included in this file is provided under the terms of the ISC license
   http://www.isc.org/downloads/software-support-policy/isc-license. Permission
   To use, copy, modify, and/or distribute this software for any purpose with or
   without fee is hereby granted provided that the above copyright notice and
   this permission notice appear in all copies.

   THE SOFTWARE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES,
   WHETHER EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR
   PURPOSE, ARE DISCLAIMED.

  ==============================================================================
*/

/*******************************************************************************
 The block below describes the properties of this PIP. A PIP is a short snippet
 of code that can be read by the Projucer and used to generate a JUCE project.

 BEGIN_JUCE_PIP_METADATA

 name:             OpenGLAppTutorial
 version:          1.0.0
 vendor:           JUCE
 website:          http://juce.com
 description:      Explores the OpenGL features.

 dependencies:     juce_core, juce_data_structures, juce_events, juce_graphics,
                   juce_gui_basics, juce_gui_extra, juce_opengl
 exporters:        xcode_mac, vs2019, xcode_iphone

 type:             Component
 mainClass:        MainContentComponent

 useLocalCopy:     1

 END_JUCE_PIP_METADATA

*******************************************************************************/


#pragma once

#include <map>

//==============================================================================
/**
    This is a quick-and-dirty parser for the 3D OBJ file format.

    Just call load() and if there aren't any errors, the 'shapes' array should
    be filled with all the shape objects that were loaded from the file.
*/
class WavefrontObjFile
{
public:
    WavefrontObjFile() {}

    juce::Result load (const juce::String& objFileContent)
    {
        shapes.clear();
        return parseObjFile (juce::StringArray::fromLines (objFileContent));
    }

    juce::Result load (const juce::File& file)
    {
        sourceFile = file;
        return load (file.loadFileAsString());
    }

    //==============================================================================
    typedef juce::uint32 Index;

    struct Vertex        { float x, y, z; };
    struct TextureCoord  { float x, y;    };

    struct Mesh
    {
        juce::Array<Vertex> vertices, normals;
        juce::Array<TextureCoord> textureCoords;
        juce::Array<Index> indices;
    };

    struct Material
    {
        Material() noexcept
        {
            juce::zerostruct (ambient);
            juce::zerostruct (diffuse);
            juce::zerostruct (specular);
            juce::zerostruct (transmittance);
            juce::zerostruct (emission);
        }

        juce::String name;

        Vertex ambient, diffuse, specular, transmittance, emission;
        float shininess = 1.0f, refractiveIndex = 0.0f;

        juce::String ambientTextureName, diffuseTextureName,
                     specularTextureName, normalTextureName;

        juce::StringPairArray parameters;
    };

    struct Shape
    {
        juce::String name;
        Mesh mesh;
        Material material;
    };

    juce::OwnedArray<Shape> shapes;

private:
    //==============================================================================
    juce::File sourceFile;

    struct TripleIndex
    {
        TripleIndex() noexcept {}

        bool operator< (const TripleIndex& other) const noexcept
        {
            if (this == &other)
                return false;

            if (vertexIndex != other.vertexIndex)
                return vertexIndex < other.vertexIndex;

            if (textureIndex != other.textureIndex)
                return textureIndex < other.textureIndex;

            return normalIndex < other.normalIndex;
        }

        int vertexIndex = -1, textureIndex = -1, normalIndex = -1;
    };

    struct IndexMap
    {
        std::map<TripleIndex, Index> map;

        Index getIndexFor (TripleIndex i, Mesh& newMesh, const Mesh& srcMesh)
        {
            const std::map<TripleIndex, Index>::iterator it (map.find (i));

            if (it != map.end())
                return it->second;

            auto index = (Index) newMesh.vertices.size();

            if (juce::isPositiveAndBelow (i.vertexIndex, srcMesh.vertices.size()))
                newMesh.vertices.add (srcMesh.vertices.getReference (i.vertexIndex));

            if (juce::isPositiveAndBelow (i.normalIndex, srcMesh.normals.size()))
                newMesh.normals.add (srcMesh.normals.getReference (i.normalIndex));

            if (juce::isPositiveAndBelow (i.textureIndex, srcMesh.textureCoords.size()))
                newMesh.textureCoords.add (srcMesh.textureCoords.getReference (i.textureIndex));

            map[i] = index;
            return index;
        }
    };

    static float parseFloat (juce::String::CharPointerType& t)
    {
        t = t.findEndOfWhitespace();
        return (float) juce::CharacterFunctions::readDoubleValue (t);
    }

    static Vertex parseVertex (juce::String::CharPointerType t)
    {
        Vertex v;
        v.x = parseFloat (t);
        v.y = parseFloat (t);
        v.z = parseFloat (t);
        return v;
    }

    static TextureCoord parseTextureCoord (juce::String::CharPointerType t)
    {
        TextureCoord tc;
        tc.x = parseFloat (t);
        tc.y = parseFloat (t);
        return tc;
    }

    static bool matchToken (juce::String::CharPointerType& t, const char* token)
    {
        auto len = (int) strlen (token);

        if (juce::CharacterFunctions::compareUpTo (juce::CharPointer_ASCII (token), t, len) == 0)
        {
            auto end = t + len;

            if (end.isEmpty() || end.isWhitespace())
            {
                t = end.findEndOfWhitespace();
                return true;
            }
        }

        return false;
    }

    struct Face
    {
        Face (juce::String::CharPointerType t)
        {
            while (! t.isEmpty())
                triples.add (parseTriple (t));
        }

        juce::Array<TripleIndex> triples;

        void addIndices (Mesh& newMesh, const Mesh& srcMesh, IndexMap& indexMap)
        {
            TripleIndex i0 (triples[0]), i1, i2 (triples[1]);

            for (auto i = 2; i < triples.size(); ++i)
            {
                i1 = i2;
                i2 = triples.getReference (i);

                newMesh.indices.add (indexMap.getIndexFor (i0, newMesh, srcMesh));
                newMesh.indices.add (indexMap.getIndexFor (i1, newMesh, srcMesh));
                newMesh.indices.add (indexMap.getIndexFor (i2, newMesh, srcMesh));
            }
        }

        static TripleIndex parseTriple (juce::String::CharPointerType& t)
        {
            TripleIndex i;

            t = t.findEndOfWhitespace();
            i.vertexIndex = t.getIntValue32() - 1;
            t = findEndOfFaceToken (t);

            if (t.isEmpty() || t.getAndAdvance() != '/')
                return i;

            if (*t == '/')
            {
                ++t;
            }
            else
            {
                i.textureIndex = t.getIntValue32() - 1;
                t = findEndOfFaceToken (t);

                if (t.isEmpty() || t.getAndAdvance() != '/')
                    return i;
            }

            i.normalIndex = t.getIntValue32() - 1;
            t = findEndOfFaceToken (t);
            return i;
        }

        static juce::String::CharPointerType findEndOfFaceToken (juce::String::CharPointerType t) noexcept
        {
            return juce::CharacterFunctions::findEndOfToken (t, juce::CharPointer_ASCII ("/ \t"), juce::String().getCharPointer());
        }
    };

    static Shape* parseFaceGroup (const Mesh& srcMesh,
                                  juce::Array<Face>& faceGroup,
                                  const Material& material,
                                  const juce::String& name)
    {
        if (faceGroup.size() == 0)
            return nullptr;

        std::unique_ptr<Shape> shape (new Shape());
        shape->name = name;
        shape->material = material;

        IndexMap indexMap;

        for (auto& f : faceGroup)
            f.addIndices (shape->mesh, srcMesh, indexMap);

        return shape.release();
    }

    juce::Result parseObjFile (const juce::StringArray& lines)
    {
        Mesh mesh;
        juce::Array<Face> faceGroup;

        juce::Array<Material> knownMaterials;
        Material lastMaterial;
        juce::String lastName;

        for (auto lineNum = 0; lineNum < lines.size(); ++lineNum)
        {
            auto l = lines[lineNum].getCharPointer().findEndOfWhitespace();

            if (matchToken (l, "v"))    { mesh.vertices.add (parseVertex (l));            continue; }
            if (matchToken (l, "vn"))   { mesh.normals.add (parseVertex (l));             continue; }
            if (matchToken (l, "vt"))   { mesh.textureCoords.add (parseTextureCoord (l)); continue; }
            if (matchToken (l, "f"))    { faceGroup.add (Face (l));                       continue; }

            if (matchToken (l, "usemtl"))
            {
                auto name = juce::String (l).trim();

                for (auto i = knownMaterials.size(); --i >= 0;)
                {
                    if (knownMaterials.getReference(i).name == name)
                    {
                        lastMaterial = knownMaterials.getReference(i);
                        break;
                    }
                }

                continue;
            }

            if (matchToken (l, "mtllib"))
            {
                juce::Result r = parseMaterial (knownMaterials, juce::String (l).trim());
                continue;
            }

            if (matchToken (l, "g") || matchToken (l, "o"))
            {
                if (Shape* shape = parseFaceGroup (mesh, faceGroup, lastMaterial, lastName))
                    shapes.add (shape);

                faceGroup.clear();
                lastName = juce::StringArray::fromTokens (l, " \t", "")[0];
                continue;
            }
        }

        if (auto* shape = parseFaceGroup (mesh, faceGroup, lastMaterial, lastName))
            shapes.add (shape);

        return juce::Result::ok();
    }

    juce::Result parseMaterial (juce::Array<Material>& materials, const juce::String& filename)
    {
        jassert (sourceFile.exists());
        auto f = sourceFile.getSiblingFile (filename);

        if (! f.exists())
            return juce::Result::fail ("Cannot open file: " + filename);

        auto lines = juce::StringArray::fromLines (f.loadFileAsString());

        materials.clear();
        Material material;

        for (auto line : lines)
        {
            auto l = line.getCharPointer().findEndOfWhitespace();

            if (matchToken (l, "newmtl"))   { materials.add (material); material.name = juce::String (l).trim(); continue; }

            if (matchToken (l, "Ka"))       { material.ambient         = parseVertex (l); continue; }
            if (matchToken (l, "Kd"))       { material.diffuse         = parseVertex (l); continue; }
            if (matchToken (l, "Ks"))       { material.specular        = parseVertex (l); continue; }
            if (matchToken (l, "Kt"))       { material.transmittance   = parseVertex (l); continue; }
            if (matchToken (l, "Ke"))       { material.emission        = parseVertex (l); continue; }
            if (matchToken (l, "Ni"))       { material.refractiveIndex = parseFloat (l);  continue; }
            if (matchToken (l, "Ns"))       { material.shininess       = parseFloat (l);  continue; }

            if (matchToken (l, "map_Ka"))   { material.ambientTextureName  = juce::String (l).trim(); continue; }
            if (matchToken (l, "map_Kd"))   { material.diffuseTextureName  = juce::String (l).trim(); continue; }
            if (matchToken (l, "map_Ks"))   { material.specularTextureName = juce::String (l).trim(); continue; }
            if (matchToken (l, "map_Ns"))   { material.normalTextureName   = juce::String (l).trim(); continue; }

            auto tokens = juce::StringArray::fromTokens (l, " \t", "");

            if (tokens.size() >= 2)
                material.parameters.set (tokens[0].trim(), tokens[1].trim());
        }

        materials.add (material);
        return juce::Result::ok();
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WavefrontObjFile)
};

//==============================================================================
/*
    This component lives inside our window, and this is where you should put all
    your controls and content.
*/
class MainContentComponent   : public juce::OpenGLAppComponent
{
public:
    //==============================================================================
    MainContentComponent()
    {
        setSize (800, 600);
    }

    ~MainContentComponent() override
    {
        shutdownOpenGL();
    }

    void initialise() override
    {
        createShaders();
    }

    void shutdown() override
    {
        shader    .reset();
        shape     .reset();
        attributes.reset();
        uniforms  .reset();
    }

    juce::Matrix3D<float> getProjectionMatrix() const
    {
        auto w = 1.0f / (0.5f + 0.1f);                                          // [1]
        auto h = w * getLocalBounds().toFloat().getAspectRatio (false);         // [2]

        return juce::Matrix3D<float>::fromFrustum (-w, w, -h, h, 4.0f, 30.0f);  // [3]
    }

    juce::Matrix3D<float> getViewMatrix() const
    {
        auto viewMatrix = juce::Matrix3D<float>::fromTranslation ({ 0.0f, 0.0f, -10.0f });  // [4]
        auto rotationMatrix = viewMatrix.rotation ({ -0.3f,
                                                      5.0f * std::sin ((float) getFrameCounter() * 0.01f),
                                                      0.0f });                        // [5]

        return viewMatrix * rotationMatrix;                                           // [6]
    }

    void render() override
    {
        using namespace ::juce::gl;

        jassert (juce::OpenGLHelpers::isContextActive());

        auto desktopScale = (float) openGLContext.getRenderingScale();          // [1]
        juce::OpenGLHelpers::clear (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId)); // [2]

        glEnable (GL_BLEND);                                                    // [3]
        glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        glViewport (0,
                    0,
                    juce::roundToInt (desktopScale * (float) getWidth()),
                    juce::roundToInt (desktopScale * (float) getHeight()));     // [4]

        shader->use();                                                          // [5]

        if (uniforms->projectionMatrix.get() != nullptr)                        // [6]
            uniforms->projectionMatrix->setMatrix4 (getProjectionMatrix().mat, 1, false);

        if (uniforms->viewMatrix.get() != nullptr)                              // [7]
            uniforms->viewMatrix->setMatrix4 (getViewMatrix().mat, 1, false);

        shape->draw (*attributes);                                              // [8]

        // Reset the element buffers so child Components draw correctly
        glBindBuffer (GL_ARRAY_BUFFER, 0);                                      // [9]
        glBindBuffer (GL_ELEMENT_ARRAY_BUFFER, 0);
    }

    void paint (juce::Graphics& g) override
    {
        // You can add your component specific drawing code here!
        // This will draw over the top of the openGL background.

        g.setColour (getLookAndFeel().findColour (juce::Label::textColourId));
        g.setFont (20);
        g.drawText ("OpenGL Example", 25, 20, 300, 30, juce::Justification::left);
        g.drawLine (20, 20, 170, 20);
        g.drawLine (20, 50, 170, 50);
    }

    void resized() override
    {
        // This is called when the MainContentComponent is resized.
        // If you add any child components, this is where you should
        // update their positions.
    }

    void createShaders()
    {
        vertexShader = R"(
            attribute vec4 position;
            attribute vec4 sourceColour;
            attribute vec2 textureCoordIn;

            uniform mat4 projectionMatrix;
            uniform mat4 viewMatrix;

            varying vec4 destinationColour;
            varying vec2 textureCoordOut;

            void main()
            {
                destinationColour = sourceColour;
                textureCoordOut = textureCoordIn;
                gl_Position = projectionMatrix * viewMatrix * position;
            })";

        fragmentShader =
           #if JUCE_OPENGL_ES
            R"(varying lowp vec4 destinationColour;
               varying lowp vec2 textureCoordOut;)"
           #else
            R"(varying vec4 destinationColour;
               varying vec2 textureCoordOut;)"
           #endif
            R"(
               void main()
               {)"
           #if JUCE_OPENGL_ES
            R"(    lowp vec4 colour = vec4(0.95, 0.57, 0.03, 0.7);)"
           #else
            R"(    vec4 colour = vec4(0.95, 0.57, 0.03, 0.7);)"
           #endif
            R"(    gl_FragColor = colour;
               })";

        std::unique_ptr<juce::OpenGLShaderProgram> newShader (new juce::OpenGLShaderProgram (openGLContext));   // [1]
        juce::String statusText;

        if (newShader->addVertexShader (juce::OpenGLHelpers::translateVertexShaderToV3 (vertexShader))          // [2]
              && newShader->addFragmentShader (juce::OpenGLHelpers::translateFragmentShaderToV3 (fragmentShader))
              && newShader->link())
        {
            shape     .reset();
            attributes.reset();
            uniforms  .reset();

            shader.reset (newShader.release());                                                                 // [3]
            shader->use();

            shape     .reset (new Shape());
            attributes.reset (new Attributes (*shader));
            uniforms  .reset (new Uniforms (*shader));

            statusText = "GLSL: v" + juce::String (juce::OpenGLShaderProgram::getLanguageVersion(), 2);
        }
        else
        {
            statusText = newShader->getLastError();                                                             // [4]
        }
    }

private:
    //==============================================================================
    struct Vertex
    {
        float position[3];
        float normal[3];
        float colour[4];
        float texCoord[2];
    };

    //==============================================================================
    // This class just manages the attributes that the shaders use.
    struct Attributes
    {
        explicit Attributes (juce::OpenGLShaderProgram& shaderProgram)
        {
            position      .reset (createAttribute (shaderProgram, "position"));
            normal        .reset (createAttribute (shaderProgram, "normal"));
            sourceColour  .reset (createAttribute (shaderProgram, "sourceColour"));
            textureCoordIn.reset (createAttribute (shaderProgram, "textureCoordIn"));
        }

        void enable()
        {
            using namespace ::juce::gl;

            if (position.get() != nullptr)
            {
                glVertexAttribPointer (position->attributeID, 3, GL_FLOAT, GL_FALSE, sizeof (Vertex), nullptr);
                glEnableVertexAttribArray (position->attributeID);
            }

            if (normal.get() != nullptr)
            {
                glVertexAttribPointer (normal->attributeID, 3, GL_FLOAT, GL_FALSE, sizeof (Vertex), (GLvoid*) (sizeof (float) * 3));
                glEnableVertexAttribArray (normal->attributeID);
            }

            if (sourceColour.get() != nullptr)
            {
                glVertexAttribPointer (sourceColour->attributeID, 4, GL_FLOAT, GL_FALSE, sizeof (Vertex), (GLvoid*) (sizeof (float) * 6));
                glEnableVertexAttribArray (sourceColour->attributeID);
            }

            if (textureCoordIn.get() != nullptr)
            {
                glVertexAttribPointer (textureCoordIn->attributeID, 2, GL_FLOAT, GL_FALSE, sizeof (Vertex), (GLvoid*) (sizeof (float) * 10));
                glEnableVertexAttribArray (textureCoordIn->attributeID);
            }
        }

        void disable()
        {
            using namespace ::juce::gl;

            if (position.get() != nullptr)       glDisableVertexAttribArray (position->attributeID);
            if (normal.get() != nullptr)         glDisableVertexAttribArray (normal->attributeID);
            if (sourceColour.get() != nullptr)   glDisableVertexAttribArray (sourceColour->attributeID);
            if (textureCoordIn.get() != nullptr) glDisableVertexAttribArray (textureCoordIn->attributeID);
        }

        std::unique_ptr<juce::OpenGLShaderProgram::Attribute> position, normal, sourceColour, textureCoordIn;

    private:
        static juce::OpenGLShaderProgram::Attribute* createAttribute (juce::OpenGLShaderProgram& shader,
                                                                      const juce::String& attributeName)
        {
            using namespace ::juce::gl;

            if (glGetAttribLocation (shader.getProgramID(), attributeName.toRawUTF8()) < 0)
                return nullptr;

            return new juce::OpenGLShaderProgram::Attribute (shader, attributeName.toRawUTF8());
        }
    };

    //==============================================================================
    // This class just manages the uniform values that the demo shaders use.
    struct Uniforms
    {
        explicit Uniforms (juce::OpenGLShaderProgram& shaderProgram)
        {
            projectionMatrix.reset (createUniform (shaderProgram, "projectionMatrix"));
            viewMatrix      .reset (createUniform (shaderProgram, "viewMatrix"));
        }

        std::unique_ptr<juce::OpenGLShaderProgram::Uniform> projectionMatrix, viewMatrix;

    private:
        static juce::OpenGLShaderProgram::Uniform* createUniform (juce::OpenGLShaderProgram& shaderProgram,
                                                                  const juce::String& uniformName)
        {
            using namespace ::juce::gl;

            if (glGetUniformLocation (shaderProgram.getProgramID(), uniformName.toRawUTF8()) < 0)
                return nullptr;

            return new juce::OpenGLShaderProgram::Uniform (shaderProgram, uniformName.toRawUTF8());
        }
    };

    //==============================================================================
    /** This loads a 3D model from an OBJ file and converts it into some vertex buffers
        that we can draw.
    */
    struct Shape
    {
        Shape()
        {
            auto dir = juce::File::getCurrentWorkingDirectory();

            int numTries = 0;

            while (! dir.getChildFile ("Resources").exists() && numTries++ < 15)
                dir = dir.getParentDirectory();


            if (shapeFile.load (dir.getChildFile ("Resources").getChildFile ("teapot.obj")).wasOk()) {
                std::cout << "found file " << std::endl;
                for (auto* s : shapeFile.shapes)
                    vertexBuffers.add (new VertexBuffer (*s));
            }
        }

        void draw (Attributes& glAttributes)
        {
            using namespace ::juce::gl;

            for (auto* vertexBuffer : vertexBuffers)
            {
                vertexBuffer->bind();

                glAttributes.enable();
                glDrawElements (GL_TRIANGLES, vertexBuffer->numIndices, GL_UNSIGNED_INT, nullptr);
                glAttributes.disable();
            }
        }

    private:
        struct VertexBuffer
        {
            explicit VertexBuffer (WavefrontObjFile::Shape& aShape)
            {
                using namespace ::juce::gl;

                numIndices = aShape.mesh.indices.size();                                    // [1]

                glGenBuffers (1, &vertexBuffer);                                            // [2]
                glBindBuffer (GL_ARRAY_BUFFER, vertexBuffer);

                juce::Array<Vertex> vertices;
                createVertexListFromMesh (aShape.mesh, vertices, juce::Colours::green);     // [3]

                glBufferData (GL_ARRAY_BUFFER,                                              // [4]
                              static_cast<GLsizeiptr> (static_cast<size_t> (vertices.size()) * sizeof (Vertex)),
                              vertices.getRawDataPointer(), GL_STATIC_DRAW);

                glGenBuffers (1, &indexBuffer);                                             // [5]
                glBindBuffer (GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
                glBufferData (GL_ELEMENT_ARRAY_BUFFER,
                              static_cast<GLsizeiptr> (static_cast<size_t> (numIndices) * sizeof (juce::uint32)),
                              aShape.mesh.indices.getRawDataPointer(), GL_STATIC_DRAW);
            }

            ~VertexBuffer()
            {
                using namespace ::juce::gl;

                glDeleteBuffers (1, &vertexBuffer);
                glDeleteBuffers (1, &indexBuffer);
            }

            void bind()
            {
                using namespace ::juce::gl;

                glBindBuffer (GL_ARRAY_BUFFER, vertexBuffer);
                glBindBuffer (GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
            }

            GLuint vertexBuffer, indexBuffer;
            int numIndices;

            JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (VertexBuffer)
        };

        WavefrontObjFile shapeFile;
        juce::OwnedArray<VertexBuffer> vertexBuffers;

        static void createVertexListFromMesh (const WavefrontObjFile::Mesh& mesh, juce::Array<Vertex>& list, juce::Colour colour)
        {
            auto scale = 0.2f;                                                  // [6]
            WavefrontObjFile::TextureCoord defaultTexCoord { 0.5f, 0.5f };
            WavefrontObjFile::Vertex defaultNormal { 0.5f, 0.5f, 0.5f };

            for (auto i = 0; i < mesh.vertices.size(); ++i)                     // [7]
            {
                const auto& v = mesh.vertices.getReference (i);
                const auto& n = i < mesh.normals.size() ? mesh.normals.getReference (i) : defaultNormal;
                const auto& tc = i < mesh.textureCoords.size() ? mesh.textureCoords.getReference (i) : defaultTexCoord;

                list.add ({ { scale * v.x, scale * v.y, scale * v.z, },
                            { scale * n.x, scale * n.y, scale * n.z, },
                            { colour.getFloatRed(), colour.getFloatGreen(), colour.getFloatBlue(), colour.getFloatAlpha() },
                            { tc.x, tc.y } });                                  // [8]
            }
        }
    };

    juce::String vertexShader;
    juce::String fragmentShader;

    std::unique_ptr<juce::OpenGLShaderProgram> shader;
    std::unique_ptr<Shape> shape;
    std::unique_ptr<Attributes> attributes;
    std::unique_ptr<Uniforms> uniforms;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainContentComponent)
};
