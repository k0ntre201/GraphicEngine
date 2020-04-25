#include "OpenGLRenderingEngine.hpp"

#include "../../Core/IO/FileReader.hpp"

GraphicEngine::OpenGL::OpenGLRenderingEngine::OpenGLRenderingEngine(
	std::shared_ptr<Common::Camera> camera,
	std::shared_ptr<Core::EventManager> eventManager) :
	RenderingEngine(camera, eventManager)
{
}

bool GraphicEngine::OpenGL::OpenGLRenderingEngine::drawFrame()
{
	glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	m_program->use();
	auto v = m_camera->getViewProjectionMatrix();
	m_uniformBufferMatrix->update(&v);
	m_vertexBuffer->bind();
	m_vertexBuffer->draw();
	return false;
}

void GraphicEngine::OpenGL::OpenGLRenderingEngine::init(size_t width, size_t height)
{
	using namespace GraphicEngine::Core::IO;
	if (glewInit() != GLEW_OK)
	{
		throw std::runtime_error("OpenGL initialization failed!");
	}
	glViewport(0, 0, width, height);

	OpenGLVertexShader vert(readFile<std::string>("C:/Projects/GraphicEngine/GraphicEngine/Assets/Shaders/Glsl/basicPCVP.vert"));
	OpenGLFragmentShader frag(readFile<std::string>("C:/Projects/GraphicEngine/GraphicEngine/Assets/Shaders/Glsl/basicPCVP.frag"));
	m_program = std::shared_ptr<OpenGLShaderProgram>(new OpenGLShaderProgram({ vert, frag }));
	auto uniformIndex = glGetUniformBlockIndex(m_program->getShaderProgramId(),"MVP");
	glUniformBlockBinding(m_program->getShaderProgramId(), uniformIndex, 0);
	m_uniformBufferMatrix = std::make_shared<UniformBuffer<glm::mat4>>();
	m_vertexBuffer = std::unique_ptr<VertexBuffer<GraphicEngine::Common::VertexPC>>(new VertexBuffer<GraphicEngine::Common::VertexPC>(vertices, indices));
}

void GraphicEngine::OpenGL::OpenGLRenderingEngine::resizeFrameBuffer(size_t width, size_t height)
{
	glViewport(0, 0, width, height);
}

void GraphicEngine::OpenGL::OpenGLRenderingEngine::cleanup()
{
}
