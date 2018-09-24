// -*- mode: c++; -*-

#ifndef FREESPACE2_FREESPACE2_SDLGRAPHICSOPERATIONS_H
#define FREESPACE2_FREESPACE2_SDLGRAPHICSOPERATIONS_H

#include "osapi/osapi.h"

class SDLGraphicsOperations : public os::GraphicsOperations {
public:
    SDLGraphicsOperations ();
    ~SDLGraphicsOperations () override;

    std::unique_ptr< os::OpenGLContext > createOpenGLContext (
        os::Viewport* viewport,
        const os::OpenGLContextAttributes& gl_attrs) override;

    void makeOpenGLContextCurrent (
        os::Viewport* view, os::OpenGLContext* ctx) override;

    std::unique_ptr< os::Viewport >
    createViewport (const os::ViewPortProperties& props) override;
};

#endif // FREESPACE2_FREESPACE2_SDLGRAPHICSOPERATIONS_H
