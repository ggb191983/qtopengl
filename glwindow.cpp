/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the documentation of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** BSD License Usage
** Alternatively, you may use this file under the terms of the BSD license
** as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "glwindow.h"
#include "vertex.h"
#include <QtWidgets/QOpenGLWidget>
#include <QSurfaceFormat>
#include <QOpenGLShaderProgram>
#include <QExposeEvent>
#include "input.h"

// OpenGL Includes
#include <OpenGLError>
#include <OpenGLShaderProgram>

#include <QOpenGLDebugMessage>
#include <QOpenGLDebugLogger>

// Create a colored triangle
// Front Verticies
#define VERTEX_FTR Vertex( QVector3D( 0.5f,  0.5f,  0.5f), QVector3D( 1.0f, 0.0f, 0.0f ) )
#define VERTEX_FTL Vertex( QVector3D(-0.5f,  0.5f,  0.5f), QVector3D( 0.0f, 1.0f, 0.0f ) )
#define VERTEX_FBL Vertex( QVector3D(-0.5f, -0.5f,  0.5f), QVector3D( 0.0f, 0.0f, 1.0f ) )
#define VERTEX_FBR Vertex( QVector3D( 0.5f, -0.5f,  0.5f), QVector3D( 0.0f, 0.0f, 0.0f ) )

// Back Verticies
#define VERTEX_BTR Vertex( QVector3D( 0.5f,  0.5f, -0.5f), QVector3D( 1.0f, 1.0f, 0.0f ) )
#define VERTEX_BTL Vertex( QVector3D(-0.5f,  0.5f, -0.5f), QVector3D( 0.0f, 1.0f, 1.0f ) )
#define VERTEX_BBL Vertex( QVector3D(-0.5f, -0.5f, -0.5f), QVector3D( 1.0f, 0.0f, 1.0f ) )
#define VERTEX_BBR Vertex( QVector3D( 0.5f, -0.5f, -0.5f), QVector3D( 1.0f, 1.0f, 1.0f ) )

// Create a colored cube
static const Vertex sg_vertexes[] = {
  // Face 1 (Front)
    VERTEX_FTR, VERTEX_FTL, VERTEX_FBL,
    VERTEX_FBL, VERTEX_FBR, VERTEX_FTR,
  // Face 2 (Back)
    VERTEX_BBR, VERTEX_BTL, VERTEX_BTR,
    VERTEX_BTL, VERTEX_BBR, VERTEX_BBL,
  // Face 3 (Top)
    VERTEX_FTR, VERTEX_BTR, VERTEX_BTL,
    VERTEX_BTL, VERTEX_FTL, VERTEX_FTR,
  // Face 4 (Bottom)
    VERTEX_FBR, VERTEX_FBL, VERTEX_BBL,
    VERTEX_BBL, VERTEX_BBR, VERTEX_FBR,
  // Face 5 (Left)
    VERTEX_FBL, VERTEX_FTL, VERTEX_BTL,
    VERTEX_FBL, VERTEX_BTL, VERTEX_BBL,
  // Face 6 (Right)
    VERTEX_FTR, VERTEX_FBR, VERTEX_BBR,
    VERTEX_BBR, VERTEX_BTR, VERTEX_FTR
};

#undef VERTEX_BBR
#undef VERTEX_BBL
#undef VERTEX_BTL
#undef VERTEX_BTR

#undef VERTEX_FBR
#undef VERTEX_FBL
#undef VERTEX_FTL
#undef VERTEX_FTR

float OpenGLWindow::m_TransSpeed = 0.5f;
float OpenGLWindow::m_RotSpeed = 0.5f;

OpenGLWindow::OpenGLWindow(QWidget *parent)
    : QOpenGLWidget(parent)
    , m_program(Q_NULLPTR)
    , m_debugLogger(Q_NULLPTR)
{
    QSurfaceFormat format;
    format.setDepthBufferSize(24);
    //format.setMajorVersion( 4 );
    //format.setMinorVersion( 3 );
    //format.setSamples( 4 );
    //format.setVersion(3,3);
    //format.setProfile( QSurfaceFormat::CoreProfile);


#ifdef    GL_DEBUG
  format.setOption(QSurfaceFormat::DebugContext);
#endif // GL_DEBUG


    setFormat(format);
    m_transform.translate(0.0f, 0.0f, -5.0f);
    OpenGLError::pushErrorHandler(this);
}

void OpenGLWindow::initializeGL()
{
    #ifdef    GL_DEBUG
      m_debugLogger = new QOpenGLDebugLogger(this);
      if (m_debugLogger->initialize())
      {
        qDebug() << "GL_DEBUG Debug Logger" << m_debugLogger << "\n";
        connect(m_debugLogger, SIGNAL(messageLogged(QOpenGLDebugMessage)), this, SLOT(messageLogged(QOpenGLDebugMessage)));
        m_debugLogger->startLogging();
      }
    #endif // GL_DEBUG

    // Initialize OpenGL Backend
    initializeOpenGLFunctions();
    connect(context(), SIGNAL(aboutToBeDestroyed()), this, SLOT(teardownGL()), Qt::DirectConnection);
    connect(this, SIGNAL(frameSwapped()), this, SLOT(update()));
    printVersionInformation();

    // Set global information
    glEnable(GL_CULL_FACE);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    // Application-specific initialization
    {
      // Create Shader (Do not release until VAO is created)
      m_program = new OpenGLShaderProgram(this);
      m_program->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/shaders/simple.vert");
      m_program->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/shaders/simple.frag");
      m_program->link();
      m_program->bind();

      // Cache Uniform Locations
      u_modelToWorld = m_program->uniformLocation("modelToWorld");
      u_worldToCamera = m_program->uniformLocation("worldToCamera");
      u_cameraToView = m_program->uniformLocation("cameraToView");

      // Create Buffer (Do not release until VAO is created)
      m_vertex.create();
      m_vertex.bind();
      m_vertex.setUsagePattern(QOpenGLBuffer::StaticDraw);
      m_vertex.allocate(sg_vertexes, sizeof(sg_vertexes));

      // Create Vertex Array Object
      m_object.create();
      m_object.bind();
      m_program->enableAttributeArray(0);
      m_program->enableAttributeArray(1);
      m_program->setAttributeBuffer(0, GL_FLOAT, Vertex::positionOffset(), Vertex::PositionTupleSize, Vertex::stride());
      m_program->setAttributeBuffer(1, GL_FLOAT, Vertex::colorOffset(), Vertex::ColorTupleSize, Vertex::stride());

      // Release (unbind) all
      m_object.release();
      m_vertex.release();
      m_program->release();
    }
}

void OpenGLWindow::paintGL()
{
    // Clear
      glClear(GL_COLOR_BUFFER_BIT);

      // Render using our shader
      m_program->bind();
      m_program->setUniformValue(u_worldToCamera, m_camera.toMatrix());
      m_program->setUniformValue(u_cameraToView, m_projection);
      {
        m_object.bind();
        m_program->setUniformValue(u_modelToWorld, m_transform.toMatrix());
        glDrawArrays(GL_TRIANGLES, 0, sizeof(sg_vertexes) / sizeof(sg_vertexes[0]));
        m_object.release();
      }
      m_program->release();
}

void OpenGLWindow::resizeGL(int width, int height)
{
     m_projection.setToIdentity();
     m_projection.perspective(60.0f, width / float(height), 0.0f, 1000.0f);
}

OpenGLWindow::~OpenGLWindow()
{
    makeCurrent();
    teardownGL();
    OpenGLError::popErrorHandler();
}

void OpenGLWindow::teardownGL()
{
  // Actually destroy our OpenGL information
  m_object.destroy();
  m_vertex.destroy();
  delete m_program;
}

void OpenGLWindow::update()
{
    // Update input
  Input::update();

  // Camera Transformation
  if (Input::buttonPressed(Qt::RightButton))
  {
    // Handle rotations
    m_camera.rotate(-m_RotSpeed * Input::mouseDelta().x(), Camera3D::LocalUp);
    m_camera.rotate(-m_RotSpeed * Input::mouseDelta().y(), m_camera.right());

    // Handle translations
    QVector3D translation;
    if (Input::keyPressed(Qt::Key_W))
    {
      translation += m_camera.forward();
    }
    if (Input::keyPressed(Qt::Key_S))
    {
      translation -= m_camera.forward();
    }
    if (Input::keyPressed(Qt::Key_A))
    {
      translation -= m_camera.right();
    }
    if (Input::keyPressed(Qt::Key_D))
    {
      translation += m_camera.right();
    }
    if (Input::keyPressed(Qt::Key_Q))
    {
      translation -= m_camera.up();
    }
    if (Input::keyPressed(Qt::Key_E))
    {
      translation += m_camera.up();
    }

    #ifdef    GL_DEBUG
      qDebug() << "TranslationSpeed:" << m_TransSpeed << "\n";
      qDebug() << "RotateSpeed:" << m_RotSpeed << "\n";
    #endif

    m_camera.translate(m_TransSpeed * translation);
  }

  // Update instance information
  m_transform.rotate(m_RotSpeed, QVector3D(0.4f, 0.3f, 0.3f));

  // Schedule a redraw
  QOpenGLWidget::update();
}

/*
void OpenGLWindow::exposeEvent(QExposeEvent *ev)
{
  if (ev->region() != m_cachedRegion)
  {
    m_cachedRegion = ev->region();
    QOpenGLWidget::exposeEvent(ev);
  }
  else
  {
    ev->ignore();
  }
}*/

void OpenGLWindow::keyPressEvent(QKeyEvent *event)
{
  if (event->isAutoRepeat())
  {
    event->ignore();
  }
  else
  {
    Input::registerKeyPress(event->key());
  }
}

void OpenGLWindow::keyReleaseEvent(QKeyEvent *event)
{
  if (event->isAutoRepeat())
  {
    event->ignore();
  }
  else
  {
    Input::registerKeyRelease(event->key());
  }
}

void OpenGLWindow::mousePressEvent(QMouseEvent *event)
{
  Input::registerMousePress(event->button());
}

void OpenGLWindow::mouseReleaseEvent(QMouseEvent *event)
{
  Input::registerMouseRelease(event->button());
}

bool OpenGLWindow::event(QEvent *e)
{
  if (e->type() == OpenGLError::type())
  {
    errorEventGL(static_cast<OpenGLError*>(e));
    return true;
  }
  return QOpenGLWidget::event(e);
}

void OpenGLWindow::errorEventGL(OpenGLError *event)
{
  qFatal("%s::%s => Returned an error!", event->callerName(), event->functionName());
}

void OpenGLWindow::changeTransSpeed(int speed)
{
    #ifdef    GL_DEBUG
      qDebug() << "TransSpeed:" << speed << "\n";
    #endif

    m_TransSpeed = speed*0.5/(float)100;

    #ifdef    GL_DEBUG
      qDebug() << "TranslationSpeed:" << m_TransSpeed << "\n";
    #endif
}

void OpenGLWindow::changeRotSpeed(int speed)
{
    #ifdef    GL_DEBUG
      qDebug() << "RotSpeed:" << speed << "\n";
    #endif

    m_RotSpeed = (float)speed/(float)100;

    #ifdef    GL_DEBUG
      qDebug() << "RotationSpeed:" << m_RotSpeed << "\n";
    #endif
}

void OpenGLWindow::messageLogged(const QOpenGLDebugMessage &msg)
{
  QString error;

  // Format based on severity
  switch (msg.severity())
  {
  case QOpenGLDebugMessage::NotificationSeverity:
    error += "--";
    break;
  case QOpenGLDebugMessage::HighSeverity:
    error += "!!";
    break;
  case QOpenGLDebugMessage::MediumSeverity:
    error += "!~";
    break;
  case QOpenGLDebugMessage::LowSeverity:
    error += "~~";
    break;
  }

  error += " (";

  // Format based on source
#define CASE(c) case QOpenGLDebugMessage::c: error += #c; break
  switch (msg.source())
  {
    CASE(APISource);
    CASE(WindowSystemSource);
    CASE(ShaderCompilerSource);
    CASE(ThirdPartySource);
    CASE(ApplicationSource);
    CASE(OtherSource);
    CASE(InvalidSource);
  }
#undef CASE

  error += " : ";

  // Format based on type
#define CASE(c) case QOpenGLDebugMessage::c: error += #c; break
  switch (msg.type())
  {
    CASE(ErrorType);
    CASE(DeprecatedBehaviorType);
    CASE(UndefinedBehaviorType);
    CASE(PortabilityType);
    CASE(PerformanceType);
    CASE(OtherType);
    CASE(MarkerType);
    CASE(GroupPushType);
    CASE(GroupPopType);
  }
#undef CASE

  error += ")";
  qDebug() << qPrintable(error) << "\n" << qPrintable(msg.message()) << "\n";
}

void OpenGLWindow::printVersionInformation()
{
  QString glType;
  QString glVersion;
  QString glProfile;

  // Get Version Information
  glType = (context()->isOpenGLES()) ? "OpenGL ES" : "OpenGL";
  glVersion = reinterpret_cast<const char*>(glGetString(GL_VERSION));

  // Get Profile Information
#define CASE(c) case QSurfaceFormat::c: glProfile = #c; break
  switch (format().profile())
  {
    CASE(NoProfile);
    CASE(CoreProfile);
    CASE(CompatibilityProfile);
  }
#undef CASE

  // qPrintable() will print our QString w/o quotes around it.
  qDebug() << qPrintable(glType) << qPrintable(glVersion) << "(" << qPrintable(glProfile) << ")";
}
