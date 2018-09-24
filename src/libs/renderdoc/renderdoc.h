// -*- mode: c++; -*-

#ifndef FREESPACE2_LIBS_RENDERDOC_RENDERDOC_H
#define FREESPACE2_LIBS_RENDERDOC_RENDERDOC_H

namespace renderdoc {

bool loadApi ();

void triggerCapture ();

void startCapture ();

bool isCapturing ();

void endCapture ();

} // namespace renderdoc

#endif // FREESPACE2_LIBS_RENDERDOC_RENDERDOC_H
