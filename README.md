# korori

## LTexture

Workflow can be in 2 ways

* load surface -> convert surface to ABGR pixel format -> load texture from pixels
* _load pixels from file_ -> modify pixel data -> create texture from pixel data

wheres _load pixels from file_ is the process that does
- load surface
- convert surface to ABGR

# LFont

* loading ttf file's process is using FreeType to load all glyphs to get to know information about maximum/minimum width/height and 8-bit texture buffer from each glyph -> then we use all them to place into final bitmap font texture.
