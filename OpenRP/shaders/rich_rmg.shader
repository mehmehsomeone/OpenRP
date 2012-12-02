// skyparms work like this:

// q3map_sun <red> <green> <blue> <intensity> <degrees> <elevation>

// color will be normalized, so it doesn't matter what range you use

// intensity falls off with angle but not distance 100 is a fairly bright sun

// degree of 0 = from the east, 90 = north, etc.  altitude of 0 = sunrise/set, 90 = noon

// automap shader

textures/rich/crazywater
{
	qer_editorimage	textures/common/direction
	surfaceparm	nonsolid
	surfaceparm	nonopaque
	surfaceparm	water
	surfaceparm	trans
	q3map_material	Water
	q3map_nolightmap
	q3map_onlyvertexlighting
	sort	seeThrough
	deformvertexes	wave	100 sin 1 1 0 1
    {
        map textures/yavin/lshadow
        blendFunc GL_DST_COLOR GL_ZERO
        glow
        rgbGen vertex
        tcMod turb 2 0.05 0 0.1
    }
    {
        map textures/common/water3
        blendFunc GL_ONE GL_SRC_ALPHA
        rgbGen const ( 0.423529 0.423529 0.423529 )
        alphaGen const 0.7
        tcMod turb 0.5 0.03 0 0.3
        tcMod scroll 0 -0.25
    }
    {
        map textures/common/stars
        blendFunc GL_ONE GL_ONE
        tcMod scroll 0 -1
    }
}

gfx/menus/rmg/arrow_w
{
	nopicmip
	nomipmaps
	notc
	q3map_nolightmap
	cull	twosided
    {
        clampmap gfx/menus/rmg/arrow_w
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        glow
        rgbGen identity
        alphaGen identity
    }
}

gfx/menus/rmg/arrow_b
{
	nopicmip
	nomipmaps
	notc
	q3map_nolightmap
	cull	twosided
    {
        clampmap gfx/menus/rmg/arrow_b
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        glow
        rgbGen identity
        alphaGen identity
    }
}

gfx/menus/rmg/arrow_r
{
	nopicmip
	nomipmaps
	notc
	q3map_nolightmap
	cull	twosided
    {
        clampmap gfx/menus/rmg/arrow_r
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        glow
        rgbGen identity
        alphaGen identity
    }
}

gfx/menus/rmg/automap
{
	nomipmaps
	q3map_nolightmap
	cull	twosided
    {
        map *automap
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        glow
        rgbGen vertex
    }
}

// JUNGLE

// ==============================================================

rmg_sky_jungle_morning
{
	lightcolor	( 0.101961 0.119412 0.136667 )
	qer_editorimage	textures/tools/editor_images/qer_sky
	q3map_surfacelight	35
	q3map_lightsubdivide	512
	sun 1 0.866667 0.592157 230 270 40
	surfaceparm	sky
	surfaceparm	noimpact
	surfaceparm	nomarks
	notc
	q3map_nolightmap
	skyParms	- 512 -
}

rmg_fog_jungle_morning
{
	qer_editorimage	textures/tools/fog
	surfaceparm	nonsolid
	surfaceparm	fog
	fogparms	( 0.7 0.7 0.7 ) 4000.0
}

rmg_sky_jungle_day
{
	lightcolor	( 0.401961 0.429412 0.456667 )
	qer_editorimage	textures/tools/editor_images/qer_sky
	q3map_surfacelight	35
	q3map_lightsubdivide	512
	sun 1 0.866667 0.592157 255 270 55
	surfaceparm	sky
	surfaceparm	noimpact
	surfaceparm	nomarks
	notc
	q3map_nolightmap
	skyParms	- 512 -
}

rmg_fog_jungle_day
{
	qer_editorimage	textures/tools/fog
	surfaceparm	nonsolid
	surfaceparm	fog
	fogparms	( 0.75 0.75 0.66 ) 4800.0
}

rmg_sky_jungle_night
{
	lightcolor	( 0.1 0.11 0.15 )
	qer_editorimage	textures/tools/editor_images/qer_sky
	q3map_surfacelight	35
	q3map_lightsubdivide	512
	sun 0.317647 0.352941 0.509804 40 270 55
	surfaceparm	sky
	surfaceparm	noimpact
	surfaceparm	nomarks
	notc
	q3map_nolightmap
	skyParms	- 512 -
}

rmg_fog_jungle_night
{
	qer_editorimage	textures/tools/fog
	surfaceparm	nonsolid
	surfaceparm	fog
	fogparms	( 0.07 0.07 0.1 ) 4300.0
}

// DESERT

// ==============================================================

rmg_sky_desert_morning
{
	lightcolor	( 0.301961 0.329412 0.466667 )
	qer_editorimage	textures/tools/editor_images/qer_sky
	q3map_surfacelight	35
	q3map_lightsubdivide	512
	sun 1 0.819608 0.54902 90 270 40
	surfaceparm	sky
	surfaceparm	noimpact
	surfaceparm	nomarks
	notc
	q3map_nolightmap
	skyParms	- 512 -
}

rmg_fog_desert_morning
{
	qer_editorimage	textures/tools/fog
	surfaceparm	nonsolid
	surfaceparm	fog
	fogparms	( 0.78 0.7 0.62 ) 4800.0
}

rmg_sky_desert_day
{
	lightcolor	( 0.401961 0.429412 0.566667 )
	qer_editorimage	textures/tools/editor_images/qer_sky
	q3map_surfacelight	35
	q3map_lightsubdivide	512
	sun 1 0.819608 0.54902 255 270 55
	surfaceparm	sky
	surfaceparm	noimpact
	surfaceparm	nomarks
	notc
	q3map_nolightmap
	skyParms	- 512 -
}

rmg_fog_desert_day
{
	qer_editorimage	textures/tools/fog
	surfaceparm	nonsolid
	surfaceparm	fog
	fogparms	( 0.75 0.65 0.52 ) 5000.0
}

rmg_sky_desert_night
{
	lightcolor	( 0.15 0.15 0.22 )
	qer_editorimage	textures/tools/editor_images/qer_sky
	q3map_surfacelight	35
	q3map_lightsubdivide	512
	sun 0.317647 0.352941 0.559804 76 270 35
	surfaceparm	sky
	surfaceparm	noimpact
	surfaceparm	nomarks
	notc
	q3map_nolightmap
	skyParms	- 512 -
}

rmg_fog_desert_night
{
	qer_editorimage	textures/tools/fog
	surfaceparm	nonsolid
	surfaceparm	fog
	fogparms	( 0.1 0.1 0.12 ) 4800.0
}

// GRASSY HILLS

// ==============================================================

rmg_sky_grassyhills_morning
{
	lightcolor	( 0.251961 0.289412 0.366667 )
	qer_editorimage	textures/tools/editor_images/qer_sky
	q3map_surfacelight	35
	q3map_lightsubdivide	512
	sun 0.945098 0.945098 0.972549 120 270 40
	surfaceparm	sky
	surfaceparm	noimpact
	surfaceparm	nomarks
	notc
	q3map_nolightmap
	skyParms	- 512 -
}

rmg_fog_grassyhills_morning
{
	qer_editorimage	textures/tools/fog
	surfaceparm	nonsolid
	surfaceparm	fog
	fogparms	( 0.56 0.6 0.51 ) 4750.0
}

rmg_sky_grassyhills_day
{
	lightcolor	( 0.301961 0.466667 0.329412 )
	qer_editorimage	textures/tools/editor_images/qer_sky
	q3map_surfacelight	35
	q3map_lightsubdivide	512
	sun 0.945098 0.972549 0.945098 255 270 55
	surfaceparm	sky
	surfaceparm	noimpact
	surfaceparm	nomarks
	notc
	q3map_nolightmap
	skyParms	- 512 -
}

rmg_fog_grassyhills_day
{
	qer_editorimage	textures/tools/fog
	surfaceparm	nonsolid
	surfaceparm	fog
	fogparms	( 0.66 0.7 0.61 ) 5000.0
}

rmg_sky_grassyhills_night
{
	lightcolor	( 0.15 0.17 0.15 )
	qer_editorimage	textures/tools/editor_images/qer_sky
	q3map_surfacelight	35
	q3map_lightsubdivide	512
	sun 0.317647 0.452941 0.409804 95 270 35
	surfaceparm	sky
	surfaceparm	noimpact
	surfaceparm	nomarks
	notc
	q3map_nolightmap
	skyParms	- 512 -
}

rmg_fog_grassyhills_night
{
	qer_editorimage	textures/tools/fog
	surfaceparm	nonsolid
	surfaceparm	fog
	fogparms	( 0.15 0.17 0.15 ) 4500.0
}

// SNOWY

// ==============================================================

rmg_sky_snowy_morning
{
	lightcolor	( 0.251961 0.269412 0.366667 )
	qer_editorimage	textures/tools/editor_images/qer_sky
	q3map_surfacelight	35
	q3map_lightsubdivide	512
	sun 0.843137 0.87451 0.92549 75 270 40
	surfaceparm	sky
	surfaceparm	noimpact
	surfaceparm	nomarks
	notc
	q3map_nolightmap
	skyParms	- 512 -
}

rmg_fog_snowy_morning
{
	qer_editorimage	textures/tools/fog
	surfaceparm	nonsolid
	surfaceparm	fog
	fogparms	( 0.4 0.4 0.47 ) 3500.0
}

rmg_sky_snowy_day
{
	lightcolor	( 0.301961 0.329412 0.466667 )
	qer_editorimage	textures/tools/editor_images/qer_sky
	q3map_surfacelight	35
	q3map_lightsubdivide	512
	sun 0.87451 0.87451 0.92549 255 270 55
	surfaceparm	sky
	surfaceparm	noimpact
	surfaceparm	nomarks
	notc
	q3map_nolightmap
	skyParms	- 512 -
}

rmg_fog_snowy_day
{
	qer_editorimage	textures/tools/fog
	surfaceparm	nonsolid
	surfaceparm	fog
	fogparms	( 0.65 0.65 0.72 ) 4000.0
}

rmg_sky_snowy_night
{
	lightcolor	( 0.15 0.15 0.25 )
	qer_editorimage	textures/tools/editor_images/qer_sky
	q3map_surfacelight	35
	q3map_lightsubdivide	512
	sun 0.317647 0.352941 0.509804 75 270 35
	surfaceparm	sky
	surfaceparm	noimpact
	surfaceparm	nomarks
	notc
	q3map_nolightmap
	skyParms	- 512 -
}

rmg_fog_snowy_night
{
	qer_editorimage	textures/tools/fog
	surfaceparm	nonsolid
	surfaceparm	fog
	fogparms	( 0.15 0.15 0.25 ) 3000.0
}

// DESERT

// ==============================================================

textures/rmg/desert1_2
{
	q3map_material	Sand
	q3map_nolightmap
	q3map_onlyvertexlighting
    {
        map textures/jordan/rock_sandstone
        glow
        rgbGen exactVertex
        tcMod scale 0.5 0.5
    }
}

textures/rmg/desert1_0
{
	q3map_material	Sand
	q3map_nolightmap
	q3map_onlyvertexlighting
    {
        map textures/jordan/sand
        glow
        rgbGen exactVertex
        tcMod scale 0.3 0.3
    }
    {
        map textures/jordan/shrubery
            surfaceSprites vertical 12 10 134 500
            ssFademax 4000
            ssVariance 3 2
            ssWind 0.5
        alphaFunc GE128
        blendFunc GL_ONE GL_ZERO
        depthWrite
        rgbGen exactVertex
    }
    {
        map textures/jordan/shrubbery2
            surfaceSprites vertical 12 10 112 500
            ssFademax 4000
            ssVariance 3 2
            ssWind 0.5
        alphaFunc GE128
        blendFunc GL_ONE GL_ZERO
        depthWrite
        rgbGen exactVertex
    }
    {
        map gfx/sprites/dust
            surfaceSprites effect 10 7 45 300
            ssFademax 1500
            ssFadescale 2
            ssVariance 2 1
            ssWind 6
            ssFXDuration 200
            ssFXGrow 3 2
            ssFXAlphaRange 1 0
            ssFXWeather
        blendFunc GL_ONE GL_ONE
        detail
    }
}

textures/rmg/desert_flat
{
	q3map_material	Sand
	q3map_nolightmap
	q3map_onlyvertexlighting
    {
        map textures/jordan/sand
        glow
        rgbGen exactVertex
        tcMod scale 0.5 0.5
    }
}

// GRASSY HILLS

// ==============================================================

textures/rmg/grassy1_1
{
	q3map_material	ShortGrass
	q3map_nolightmap
	q3map_onlyvertexlighting
    {
        map textures/common/grass_green
        glow
        rgbGen exactVertex
        tcMod scale 0.5 0.5
    }
    {
        map gfx/sprites/ss_grass2
            surfaceSprites vertical 32 24 32 400
            ssFademax 1500
            ssFadescale 2
            ssVariance 1.5 1.5
            ssWind 0.4
        alphaFunc GE192
        blendFunc GL_ONE GL_ZERO
        depthWrite
        rgbGen exactVertex
    }
}

textures/rmg/grassy1_0
{
	q3map_material	Dirt
	q3map_nolightmap
	q3map_onlyvertexlighting
    {
        map textures/colombia/dirt_terrain1
        glow
        rgbGen exactVertex
    }
    {
        map gfx/sprites/ss_grass2
            surfaceSprites vertical 20 20 32 400
            ssFademax 1200
            ssVariance 1.5 1.5
            ssWind 0.3
        alphaFunc GE192
        blendFunc GL_ONE GL_ZERO
        depthWrite
        rgbGen exactVertex
    }
}

textures/rmg/water1
{
	qer_editorimage	textures/colombia/water_terrain
	surfaceparm	water
	q3map_material	Water
	q3map_nolightmap
	q3map_onlyvertexlighting
	cull	twosided
    {
        map textures/colombia/water_terrain
        blendFunc GL_ONE GL_ONE_MINUS_SRC_ALPHA
        depthWrite
        glow
        rgbGen exactVertex
        alphaGen const 0.9
        tcMod turb 0 0.08 0.04 0.08
        tcMod scroll -0.05 -0.001
        tcMod scale 0.5 0.5
    }
    {
        map textures/colombia/water_terrain2
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        rgbGen exactVertex
        tcMod turb 0 0.08 0.04 0.08
        tcMod scale 0.5 0.5
    }
}

// JUNGLE

// ==============================================================

textures/rmg/jungle_4
{
	q3map_material	Rock
	q3map_nolightmap
	q3map_onlyvertexlighting
    {
        map textures/colombia/new_big_rock
        glow
        rgbGen exactVertex
        tcMod scale 0.4 0.4
    }
}

textures/rmg/jungle_3
{
	q3map_material	LongGrass
	q3map_nolightmap
	q3map_onlyvertexlighting
    {
        map textures/colombia/new_grass_big_rock
        glow
        rgbGen exactVertex
        tcMod scale 0.4 0.4
    }
    {
        map gfx/sprites/ss_grass_plants
            surfaceSprites vertical 24 16 35 400
            ssFademax 1500
            ssFadescale 2
            ssVariance 1 1.5
            ssWind 0.7
        alphaFunc GT0
        blendFunc GL_SRC_ALPHA GL_ONE_MINUS_SRC_ALPHA
        rgbGen exactVertex
    }
}

textures/rmg/jungle_2
{
	q3map_material	LongGrass
	q3map_nolightmap
	q3map_onlyvertexlighting
	sort	1
    {
        map textures/colombia/new_grass
        glow
        rgbGen exactVertex
        tcMod scale 0.4 0.4
    }
    {
        map gfx/sprites/ss_grass_plants
            surfaceSprites vertical 54 36 54 500
            ssFademax 4000
            ssFadescale 3
            ssVariance 1.3 2
            ssWind 0.7
        alphaFunc GE192
        blendFunc GL_ONE GL_ZERO
        depthWrite
        rgbGen exactVertex
    }
}

textures/rmg/jungle_1
{
	q3map_material	Mud
	q3map_nolightmap
	q3map_onlyvertexlighting
    {
        map textures/colombia/mudside_b
        glow
        rgbGen exactVertex
        tcMod scale 0.4 0.4
    }
}

textures/rmg/jungle_flat
{
	q3map_material	LongGrass
	q3map_nolightmap
	q3map_onlyvertexlighting
	sort	1
    {
        map textures/colombia/new_grass
        glow
        rgbGen exactVertex
        tcMod scale 0.25 0.25
    }
}

// SNOWY

// ==============================================================

textures/rmg/snow1_3
{
	qer_editorimage	textures/kamchatka/rock_huge
	q3map_material	Rock
	q3map_nolightmap
	q3map_onlyvertexlighting
    {
        map textures/kamchatka/rock_huge
        glow
        rgbGen exactVertex
        tcMod scale 0.5 0.5
    }
}

textures/rmg/snow1_1
{
	qer_editorimage	textures/kamchatka/rock_huge_snow
	q3map_material	Snow
	q3map_nolightmap
	q3map_onlyvertexlighting
    {
        map textures/kamchatka/rock_huge_snow
        glow
        rgbGen exactVertex
        tcMod scale 0.5 0.5
    }
}

textures/rmg/snow2_2
{
	qer_editorimage	textures/kamchatka/snow_2
	q3map_material	Snow
	q3map_nolightmap
	q3map_onlyvertexlighting
    {
        map textures/kamchatka/snow_2
        glow
        rgbGen exactVertex
        tcMod scale 0.5 0.5
    }
}

// GRASSY HILLS

// ==============================================================

textures/rmg/grassy_flat
{
	qer_editorimage	textures/common/grass_green
	q3map_material	ShortGrass
	q3map_nolightmap
	q3map_onlyvertexlighting
    {
        map textures/common/grass_green
        glow
        rgbGen exactVertex
        tcMod scale 0.5 0.5
    }
}

gfx/png_test/test
{
	q3map_material	Rock
	q3map_nolightmap
	q3map_onlyvertexlighting
    {
        map gfx/png_test/test
        glow
        rgbGen exactVertex
        tcMod scale 0.4 0.4
    }
}

