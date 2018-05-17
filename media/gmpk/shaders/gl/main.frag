#version 410 core
#include "foundation/foundation.h"
#include "foundation/properties.h"
#include "foundation/frag_header.h"
#include "foundation/light.h"

// FRAGMENT
#include "model2d.frag"
#include "model3d.frag"
#include "glyph.frag"
#include "cubemap.frag"
#include "shadow.frag"

void main(void)
{
    GM_techniqueEntrance();
}